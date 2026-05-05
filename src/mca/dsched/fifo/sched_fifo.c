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
static pmix_list_t reqs;
dsched_event_base_t *evbase;

/*
 * Local functions
 */
static int init(void)
{
    PMIX_CONSTRUCT(&reqs, pmix_list_t);
    // setup our component progress thread
    evbase = dsched_progress_thread_init("dsched-fifo");

    return DSCHED_SUCCESS;
}

static int finalize(void)
{
    PMIX_LIST_DESTRUCT(&reqs);
    dsched_progress_thread_finalize("dsched-fifo");
    return DSCHED_SUCCESS;
}

static void sched(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    dsched_req_item_t *rqitm;
    int n, cnt=0;
    dsched_node_t *node;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    // if the incoming req is not NULL, then put it on our internal
    // list so the req is kept in order
    if (NULL != cd->req) {
        rqitm = PMIX_NEW(dsched_req_item_t);
        PMIX_RETAIN(cd->req);
        rqitm->req = cd->req;
        pmix_list_append(&reqs, &rqitm->super);
    }

    // get the first req on our list
    rqitm = (dsched_req_item_t*)pmix_list_get_first(&reqs);
    if (NULL == rqitm) {
        // no requests pending
        DSCHED_THREADSHIFT(cd, dsched_globals.evbase, dsched_sched_base_cbfunc);
        return;
    }

    // check to see if enough resources are available to allocate it
    if (rqitm->req->num_nodes < dsched_globals.avail.nnodes) {
        // we have enough!
        cd->alloc = PMIX_NEW(dsched_alloc_t);
        cd->alloc->scheduler = strdup("fifo");
        cd->alloc->pri = cd->order;
        cd->req = rqitm->req;
        // this needs to be done in the main thread!
        for (n=0; n < dsched_globals.nodes.size; n++) {
            node = (dsched_node_t*)pmix_pointer_array_get_item(&dsched_globals.nodes, n);
            if (NULL == node) {
                continue;
            }
            pmix_pointer_array_add(&cd->alloc->allocation, node);
            ++cnt;
            if (cnt == rqitm->req->num_nodes) {
                break;
            }
        }
    }

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
    cd->req = scd->mt->req;
    cd->trk = scd->trk;
    DSCHED_THREADSHIFT(cd, evbase, sched);

    return PMIX_OPERATION_IN_PROGRESS;
}
