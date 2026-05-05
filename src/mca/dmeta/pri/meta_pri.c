/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technometay
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007      Sun Microsystems, Inc.  All rights reserved.
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"
#include <pmix_common.h>
#include <pmix_server.h>

#include <string.h>
#ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#include <stdarg.h>

#include "src/common/pmix_iof.h"
#include "src/util/pmix_argv.h"
#include "src/util/pmix_error.h"
#include "src/util/pmix_name_fns.h"
#include "src/util/pmix_show_help.h"

#include "src/mca/dsched/base/base.h"
#include "src/runtime/dsched_progress_threads.h"
#include "src/mca/dmeta/base/base.h"
#include "src/mca/dmeta/pri/meta_pri.h"

/* Static API's */
static int myinit(void);
static void finalize(void);
static pmix_status_t myschedule(dsched_meta_t *mt,
                                bool assign);

/* Module def */
dsched_dmeta_module_t dsched_dmeta_pri_module = {
    .init = myinit,
    .finalize = finalize,
    .schedule = myschedule
};

// local variables
dsched_event_base_t *evbase;

static int myinit(void)
{
    // setup our component progress thread
    evbase = dsched_progress_thread_init("dmeta-pri");
    return PMIX_SUCCESS;
}

static void finalize(void)
{
    dsched_progress_thread_finalize("dmeta-pri");
    return;
}

static void sched_cbfunc(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    int n;
    dsched_alloc_t *alloc, *best=NULL;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

pmix_output(0, "SCHEDULES RETURNED");
    if (cd->flag) {
        // we have the schedules from the dsched components - take
        // the one with the highest priority
        for (n=0; n < cd->allocations.size; n++) {
            alloc = (dsched_alloc_t*)pmix_pointer_array_get_item(&cd->allocations, n);
            if (NULL == alloc) {
                continue;
            }
            if (NULL == best || alloc->pri < best->pri) {
                best = alloc;
            }
        }
        if (NULL != best) {
            cd->mt->req->allocation = best;
            cd->mt->req->status = PMIX_SUCCESS;
        }
    }
    // pass back to the base
    DSCHED_THREADSHIFT(cd, dsched_globals.evbase, dsched_meta_base_cbfunc);
}

static void sched(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    // do any meta-level prep work - e.g., add attributes
    // to direct scheduler options
    cd->evcbfunc = sched_cbfunc;

    // pass this request down to the schedulers in the main
    // progress thread
    DSCHED_THREADSHIFT(cd, dsched_globals.evbase, dsched_sched_base_schedule);
}

static pmix_status_t myschedule(dsched_meta_t *mt,
                                bool assign)
{
    dsched_shift_caddy_t *cd;

    // do not block the meta-level processing as other
    // components may be attempting to operate in parallel
    cd = PMIX_NEW(dsched_shift_caddy_t);
    cd->mt = mt;
    cd->flag = assign;
    DSCHED_THREADSHIFT(cd, evbase, sched);

    return PMIX_OPERATION_IN_PROGRESS;
}
