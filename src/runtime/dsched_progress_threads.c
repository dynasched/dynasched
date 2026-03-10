/*
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2015-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"
#include "src/include/dsched_constants.h"

#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#include <string.h>
#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#    include <pthread_np.h>
#endif
#include "src/class/pmix_list.h"
#include "src/event/event-internal.h"
#include "src/include/dsched_globals.h"
#include "src/threads/pmix_threads.h"
#include "src/util/pmix_argv.h"
#include "src/util/dsched_error.h"
#include "src/util/pmix_fd.h"

#include "src/runtime/dsched_progress_threads.h"

/* create a tracking object for progress threads */
typedef struct {
    pmix_list_item_t super;

    int refcount;
    char *name;

    dsched_event_base_t *ev_base;

    /* This will be set to false when it is time for the progress
       thread to exit */
    volatile bool ev_active;

    /* This event will always be set on the ev_base (so that the
       ev_base is not empty!) */
    dsched_event_t block;

    bool engine_constructed;
    pmix_thread_t engine;
} dsched_progress_tracker_t;

static void tracker_constructor(dsched_progress_tracker_t *p)
{
    p->refcount = 1; // start at one since someone created it
    p->name = NULL;
    p->ev_base = NULL;
    p->ev_active = false;
    p->engine_constructed = false;
}

static void tracker_destructor(dsched_progress_tracker_t *p)
{
    dsched_event_del(&p->block);

    if (NULL != p->name) {
        free(p->name);
    }
    if (NULL != p->ev_base) {
        dsched_event_base_free(p->ev_base);
    }
    if (p->engine_constructed) {
        PMIX_DESTRUCT(&p->engine);
    }
}

static PMIX_CLASS_INSTANCE(dsched_progress_tracker_t, pmix_list_item_t, tracker_constructor,
                           tracker_destructor);

static bool inited = false;
static pmix_list_t tracking;
static struct timeval long_timeout = {.tv_sec = 3600, .tv_usec = 0};
static const char *shared_thread_name = "DSCHED-wide async progress thread";

/*
 * If this event is fired, just restart it so that this event base
 * continues to have something to block on.
 */
static void dummy_timeout_cb(int fd, short args, void *cbdata)
{
    dsched_progress_tracker_t *trk = (dsched_progress_tracker_t *) cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(fd, args);

    dsched_event_add(&trk->block, &long_timeout);
}

/*
 * Main for the progress thread
 */
static void *progress_engine(pmix_object_t *obj)
{
    pmix_thread_t *t = (pmix_thread_t *) obj;
    dsched_progress_tracker_t *trk = (dsched_progress_tracker_t *) t->t_arg;

    while (trk->ev_active) {
        dsched_event_loop(trk->ev_base, DSCHED_EVLOOP_ONCE);
    }

    return PMIX_THREAD_CANCELLED;
}

static void stop_progress_engine(dsched_progress_tracker_t *trk)
{
    assert(trk->ev_active);
    trk->ev_active = false;

    /* break the event loop - this will cause the loop to exit upon
       completion of any current event */
    dsched_event_base_loopexit(trk->ev_base);

    pmix_thread_join(&trk->engine, NULL);
}

static int start_progress_engine(dsched_progress_tracker_t *trk)
{
#ifdef HAVE_PTHREAD_SETAFFINITY_NP
    cpu_set_t cpuset;
    char **ranges, *dash;
    int k, n, start, end;
#endif

    assert(!trk->ev_active);
    trk->ev_active = true;

    /* fork off a thread to progress it */
    trk->engine.t_run = progress_engine;
    trk->engine.t_arg = trk;

    int rc = pmix_thread_start(&trk->engine);
    if (DSCHED_SUCCESS != rc) {
        DSCHED_ERROR_LOG(rc);
    }

#ifdef HAVE_PTHREAD_SETAFFINITY_NP
    if (NULL != dsched_globals.progress_thread_cpus) {
        CPU_ZERO(&cpuset);
        // comma-delimited list of cpu ranges
        ranges = PMIx_Argv_split(dsched_globals.progress_thread_cpus, ',');
        for (n=0; NULL != ranges[n]; n++) {
            // look for '-'
            start = strtoul(ranges[n], &dash, 10);
            if (NULL == dash) {
                CPU_SET(start, &cpuset);
            } else {
                ++dash;  // skip over the '-'
                end = strtoul(dash, NULL, 10);
                for (k=start; k < end; k++) {
                    CPU_SET(k, &cpuset);
                }
            }
        }
        rc = pthread_setaffinity_np(trk->engine.t_handle, sizeof(cpu_set_t), &cpuset);
        if (0 != rc && dsched_globals.bind_progress_thread_reqd) {
            pmix_output(0, "Failed to bind progress thread %s",
                        (NULL == trk->name) ? "NULL" : trk->name);
            rc = DSCHED_ERR_NOT_SUPPORTED;
        } else {
            rc = DSCHED_SUCCESS;
        }
    }
#endif
    return rc;
}

dsched_event_base_t *dsched_progress_thread_init(const char *name, int npri)
{
    dsched_progress_tracker_t *trk;
    int rc;

    if (!inited) {
        PMIX_CONSTRUCT(&tracking, pmix_list_t);
        inited = true;
    }

    if (NULL == name) {
        name = shared_thread_name;
    }

    /* check if we already have this thread */
    PMIX_LIST_FOREACH(trk, &tracking, dsched_progress_tracker_t)
    {
        if (0 == strcmp(name, trk->name)) {
            /* we do, so up the refcount on it */
            ++trk->refcount;
            /* return the existing base */
            return trk->ev_base;
        }
    }

    trk = PMIX_NEW(dsched_progress_tracker_t);
    if (NULL == trk) {
        DSCHED_ERROR_LOG(DSCHED_ERR_OUT_OF_RESOURCE);
        return NULL;
    }

    trk->name = strdup(name);
    if (NULL == trk->name) {
        DSCHED_ERROR_LOG(DSCHED_ERR_OUT_OF_RESOURCE);
        PMIX_RELEASE(trk);
        return NULL;
    }

    if (NULL == (trk->ev_base = dsched_event_base_create())) {
        DSCHED_ERROR_LOG(DSCHED_ERR_OUT_OF_RESOURCE);
        PMIX_RELEASE(trk);
        return NULL;
    }

    if (0 < npri) {
        event_base_priority_init(trk->ev_base, npri);
    }

    /* add an event to the new event base (if there are no events,
       dsched_event_loop() will return immediately) */
    dsched_event_set(trk->ev_base, &trk->block, -1, DSCHED_EV_PERSIST, dummy_timeout_cb, trk);
    dsched_event_add(&trk->block, &long_timeout);

    /* construct the thread object */
    PMIX_CONSTRUCT(&trk->engine, pmix_thread_t);
    trk->engine_constructed = true;
    if (DSCHED_SUCCESS != (rc = start_progress_engine(trk))) {
        DSCHED_ERROR_LOG(rc);
        PMIX_RELEASE(trk);
        return NULL;
    }
    pmix_list_append(&tracking, &trk->super);

    return trk->ev_base;
}

int dsched_progress_thread_finalize(const char *name)
{
    dsched_progress_tracker_t *trk;

    if (!inited) {
        /* nothing we can do */
        return DSCHED_ERR_NOT_FOUND;
    }

    if (NULL == name) {
        name = shared_thread_name;
    }

    /* find the specified engine */
    PMIX_LIST_FOREACH(trk, &tracking, dsched_progress_tracker_t)
    {
        if (0 == strcmp(name, trk->name)) {
           /* If the progress thread is active, stop it */
            if (trk->ev_active) {
                stop_progress_engine(trk);
            }

            pmix_list_remove_item(&tracking, &trk->super);
            PMIX_RELEASE(trk);
            return DSCHED_SUCCESS;
        }
    }

    return DSCHED_ERR_NOT_FOUND;
}

/*
 * Stop the progress thread, but don't delete the tracker (or event base)
 */
int dsched_progress_thread_pause(const char *name)
{
    dsched_progress_tracker_t *trk;

    if (!inited) {
        /* nothing we can do */
        return DSCHED_ERR_NOT_FOUND;
    }

    /* find the specified engine */
    PMIX_LIST_FOREACH(trk, &tracking, dsched_progress_tracker_t)
    {
        if (NULL == name || 0 == strcmp(name, trk->name)) {
            if (trk->ev_active) {
                stop_progress_engine(trk);
            }
            if (NULL != name) {
                break;
            }
        }
    }

    return DSCHED_SUCCESS;
}

int dsched_progress_thread_resume(const char *name)
{
    dsched_progress_tracker_t *trk;

    if (!inited) {
        /* nothing we can do */
        return DSCHED_ERR_NOT_FOUND;
    }

    if (NULL == name) {
        name = shared_thread_name;
    }

    /* find the specified engine */
    PMIX_LIST_FOREACH(trk, &tracking, dsched_progress_tracker_t)
    {
        if (0 == strcmp(name, trk->name)) {
            if (trk->ev_active) {
                return DSCHED_ERR_RESOURCE_BUSY;
            }

            return start_progress_engine(trk);
        }
    }

    return DSCHED_ERR_NOT_FOUND;
}
