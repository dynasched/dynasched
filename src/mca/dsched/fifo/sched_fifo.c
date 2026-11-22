/*
 * Copyright (c) 2009-2011 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2010-2017 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2004-2011 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2011      Oracle and/or all its affiliates.  All rights reserved.
 * Copyright (c) 2011-2013 Los Alamos National Security, LLC.
 *                         All rights reserved.
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2017      IBM Corporation.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <string.h>
#ifdef HAVE_SYS_WAIT_H
#    include <sys/wait.h>
#endif

#include "src/runtime/dsched_progress_threads.h"
#include "src/mca/dsched/base/base.h"
#include "src/mca/dsched/dsched.h"

#include "sched_fifo.h"

static int init(void);
static int finalize(void);
static pmix_status_t fifo_schedule(dsched_shift_caddy_t *scd,
                                   int order);

/******************
 * dvm module
 ******************/
dsched_dsched_module_t dsched_sched_fifo_module = {
    .init = init,
    .finalize = finalize,
    .schedule = fifo_schedule
};

// local variables
static dsched_event_base_t *evbase;

static int init(void)
{
    // setup our component progress thread
    evbase = dsched_progress_thread_init("dsched-fifo");
    return DSCHED_SUCCESS;
}

static int finalize(void)
{
    dsched_progress_thread_finalize("dsched-fifo");
    return DSCHED_SUCCESS;
}

/*
 * First in, first out.
 *
 * Requests are taken strictly in the order they arrived: only the
 * oldest pending request is considered, and until it can be met, none
 * behind it is. It is met with whole nodes, chosen from the pool in pool
 * order:
 *
 *  - a request naming hosts (PMIX_ALLOC_NODE_LIST) chooses among those,
 *    and one naming hosts to exclude (PMIX_ALLOC_EXCLUDE) never gets
 *    them;
 *  - PMIX_ALLOC_NUM_NODES nodes, if it asks for a number;
 *  - nodes enough for PMIX_ALLOC_NUM_CPUS slots, if it asks for those -
 *    and with both, the requested number of the nodes with the most
 *    slots, so that it is met whenever it can be;
 *  - with neither, every host it names, or else one node.
 *
 * A request that could not be met even with every node free never will
 * be, and is refused rather than left to block the queue.
 *
 * This runs on the component's own thread, and reads only the round's
 * snapshot: the pool as it was when the round began, and the queue.
 */

/* the name a host is known by in the pool: our own host by our own name */
static const char *canonical(const char *name)
{
    return dsched_check_host_is_local(name) ? dsched_globals.hostname : name;
}

static bool name_matches(const dsched_node_t *node, const char *name)
{
    int m;

    if (0 == strcmp(node->name, name)) {
        return true;
    }
    for (m = 0; NULL != node->aliases && NULL != node->aliases[m]; m++) {
        if (0 == strcmp(node->aliases[m], name)) {
            return true;
        }
    }
    return false;
}

static int find_node(dsched_meta_t *mt, const char *name)
{
    const char *nm = canonical(name);
    int n;

    for (n = 0; n < mt->npool; n++) {
        if (name_matches(mt->pool[n].node, nm)) {
            return n;
        }
    }
    return -1;
}

static bool excluded(dsched_req_t *req, dsched_meta_t *mt, int idx)
{
    int m;

    for (m = 0; NULL != req->nodes_excluded && NULL != req->nodes_excluded[m]; m++) {
        if (name_matches(mt->pool[idx].node, canonical(req->nodes_excluded[m]))) {
            return true;
        }
    }
    return false;
}

/* Choose nodes for req from the pool - from the free ones if onlyfree,
 * else as if every node were free. picks (sized for the whole pool)
 * receives their pool indices. Returns PMIX_ERR_NOT_FOUND for a request
 * naming a host the pool does not have, and PMIX_ERR_OUT_OF_RESOURCE if
 * the nodes considered cannot meet it. */
static pmix_status_t choose(dsched_req_t *req, dsched_meta_t *mt, bool onlyfree,
                            int *picks, int *npicks)
{
    int *cand, ncand = 0, nwanted = 0, n, m, k, idx, tmp;
    int need_nodes = req->num_nodes, need_cpus = req->num_cpus;
    int64_t slots = 0;
    bool dup;

    *npicks = 0;
    cand = (int *) malloc((0 < mt->npool ? mt->npool : 1) * sizeof(int));
    if (NULL == cand) {
        return PMIX_ERR_NOMEM;
    }

    /* the candidates, in pool order */
    if (NULL != req->nodes_wanted) {
        for (m = 0; NULL != req->nodes_wanted[m]; m++) {
            idx = find_node(mt, req->nodes_wanted[m]);
            if (0 > idx) {
                free(cand);
                return PMIX_ERR_NOT_FOUND;
            }
            if (excluded(req, mt, idx)) {
                continue;
            }
            for (dup = false, k = 0; k < ncand; k++) {
                dup = dup || (cand[k] == idx);
            }
            if (!dup) {
                cand[ncand++] = idx;
            }
        }
        // put them in pool order
        for (n = 1; n < ncand; n++) {
            for (k = n; 0 < k && cand[k - 1] > cand[k]; k--) {
                tmp = cand[k];
                cand[k] = cand[k - 1];
                cand[k - 1] = tmp;
            }
        }
        nwanted = ncand;
    } else {
        for (n = 0; n < mt->npool; n++) {
            if (!excluded(req, mt, n)) {
                cand[ncand++] = n;
            }
        }
    }

    if (NULL != req->nodes_wanted && 0 == need_nodes && 0 == need_cpus) {
        /* every host it names, and all of them at once */
        for (k = 0; k < ncand; k++) {
            if (onlyfree && !mt->pool[cand[k]].free) {
                free(cand);
                return PMIX_ERR_OUT_OF_RESOURCE;
            }
            picks[(*npicks)++] = cand[k];
        }
        free(cand);
        return (0 < nwanted) ? PMIX_SUCCESS : PMIX_ERR_OUT_OF_RESOURCE;
    }
    if (0 == need_nodes && 0 == need_cpus) {
        need_nodes = 1;
    }

    /* only the ones that may be used now */
    if (onlyfree) {
        for (n = 0, k = 0; k < ncand; k++) {
            if (mt->pool[cand[k]].free) {
                cand[n++] = cand[k];
            }
        }
        ncand = n;
    }

    if (0 < need_nodes && 0 < need_cpus) {
        /* the requested number of the nodes with the most slots - stable,
         * so that among equals pool order decides */
        for (n = 1; n < ncand; n++) {
            for (k = n; 0 < k && mt->pool[cand[k - 1]].slots < mt->pool[cand[k]].slots; k--) {
                tmp = cand[k];
                cand[k] = cand[k - 1];
                cand[k - 1] = tmp;
            }
        }
    }
    for (k = 0; k < ncand; k++) {
        if (0 < need_nodes && *npicks == need_nodes) {
            break;
        }
        if (0 == need_nodes && slots >= need_cpus) {
            break;
        }
        picks[(*npicks)++] = cand[k];
        slots += mt->pool[cand[k]].slots;
    }
    free(cand);
    if ((0 < need_nodes && *npicks < need_nodes) || (0 < need_cpus && slots < need_cpus)) {
        *npicks = 0;
        return PMIX_ERR_OUT_OF_RESOURCE;
    }
    return PMIX_SUCCESS;
}

/* the proposal for the round: grant the oldest request, refuse it, or
 * nothing at all - it must wait */
static dsched_alloc_t *propose(dsched_meta_t *mt, int order)
{
    dsched_req_t *req;
    dsched_alloc_t *alloc;
    int *picks, npicks = 0, k;
    pmix_status_t rc;

    if (0 == mt->npending) {
        return NULL;
    }
    req = mt->pending[0];
    picks = (int *) malloc((0 < mt->npool ? mt->npool : 1) * sizeof(int));
    if (NULL == picks) {
        return NULL;
    }

    rc = choose(req, mt, true, picks, &npicks);
    if (PMIX_ERR_OUT_OF_RESOURCE == rc) {
        // could it be met at all, once nodes are freed?
        rc = choose(req, mt, false, picks, &npicks);
        if (PMIX_SUCCESS == rc) {
            // yes - it waits, and so does everything behind it
            free(picks);
            return NULL;
        }
    } else if (PMIX_SUCCESS != rc && PMIX_ERR_NOT_FOUND != rc) {
        free(picks);
        return NULL;
    }

    alloc = PMIX_NEW(dsched_alloc_t);
    alloc->scheduler = strdup("fifo");
    alloc->pri = order;
    PMIX_RETAIN(req);
    alloc->req = req;
    alloc->status = rc;
    for (k = 0; PMIX_SUCCESS == rc && k < npicks; k++) {
        PMIX_RETAIN(mt->pool[picks[k]].node);
        pmix_pointer_array_add(&alloc->allocation, mt->pool[picks[k]].node);
    }
    free(picks);
    return alloc;
}

static void sched(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    PMIX_ACQUIRE_OBJECT(cd);

    cd->alloc = propose(cd->mt, cd->order);

    // return result to the base frame
    DSCHED_THREADSHIFT(cd, dsched_globals.evbase, dsched_sched_base_cbfunc);
}

static pmix_status_t fifo_schedule(dsched_shift_caddy_t *scd, int order)
{
    dsched_shift_caddy_t *cd;

    // do not block the meta-level processing as other
    // components may be attempting to operate in parallel
    cd = PMIX_NEW(dsched_shift_caddy_t);
    cd->order = order;
    cd->mt = scd->mt;
    // our caddy holds a reference on the tracker, released with it
    PMIX_RETAIN(scd->trk);
    cd->trk = scd->trk;
    DSCHED_THREADSHIFT(cd, evbase, sched);
    return PMIX_OPERATION_IN_PROGRESS;
}
