/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2010-2011 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2011-2013 Los Alamos National Security, LLC.
 *                         All rights reserved.
 * Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2014-2019 Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"
#include "dsched_constants.h"

#include <string.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#endif

#include "src/class/pmix_list.h"
#include "src/mca/base/pmix_base.h"
#include "src/mca/mca.h"

#include "src/util/pmix_output.h"
#include "src/util/pmix_environ.h"

#include "src/mca/dsched/base/base.h"
#include "src/util/pmix_show_help.h"

#include "src/mca/dsched/base/static-components.h"

/* Instantiate the global vars */
dsched_sched_globals_t dsched_sched_globals = {
    .actives = PMIX_LIST_STATIC_INIT(dsched_sched_globals.actives),
    .initialized = false,
    .selected = false
};

static pmix_status_t dsched_dsched_base_close(void)
{
    dsched_sched_base_active_module_t *active;
    pmix_status_t rc;

    if (!dsched_sched_globals.initialized) {
        return PMIX_SUCCESS;
    }

    PMIX_LIST_FOREACH(active, &dsched_sched_globals.actives, dsched_sched_base_active_module_t) {
        if (NULL != active->module->finalize) {
            active->module->finalize();
        }
    }
    PMIX_LIST_DESTRUCT(&dsched_sched_globals.actives);

    /* Close all active components */
    rc = pmix_mca_base_framework_components_close(&dsched_dsched_base_framework, NULL);
    // mark as uninitialized
    dsched_sched_globals.initialized = false;
    return rc;
}

/**
 *  * Function for finding and opening either all MCA components, or the one
 *   * that was specifically requested via a MCA parameter.
 *    */
static pmix_status_t dsched_dsched_base_open(pmix_mca_base_open_flag_t flags)
{
    // initialize globals
    PMIX_CONSTRUCT(&dsched_sched_globals.actives, pmix_list_t);
    dsched_sched_globals.initialized = true;

    /* Open up all available components */
    return pmix_mca_base_framework_components_open(&dsched_dsched_base_framework, flags);
}

DSCHED_MCA_BASE_FRAMEWORK_DECLARE(dsched, "DynaSched scheduler plugins", NULL, dsched_dsched_base_open,
                                  dsched_dsched_base_close, dsched_mca_dsched_base_static_components,
                                  PMIX_MCA_BASE_FRAMEWORK_FLAG_DEFAULT);

/* A scheduler has answered for the round, with a proposal in scd->alloc
 * or none. Runs on dsched_globals.evbase. scd is the scheduler's own
 * caddy, holding a reference on the tracker; cd, which the tracker
 * points to, collects the proposals. */
void dsched_sched_base_cbfunc(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *scd = (dsched_shift_caddy_t*)cbdata;
    dsched_op_tracker_t *trk = scd->trk;
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)trk->cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    PMIX_ACQUIRE_OBJECT(scd);

    trk->nresponded++;
    if (NULL != scd->alloc) {
        // the proposal passes to cd, which releases what it collects
        pmix_pointer_array_add(&cd->allocations, scd->alloc);
        scd->alloc = NULL;
    }

    if (trk->nresponded == trk->nactive) {
        // all the schedulers have completed
        DSCHED_THREADSHIFT(cd, dsched_globals.evbase, cd->evcbfunc);
    }
    PMIX_RELEASE(scd);
}

/* Ask every active scheduler for a proposal for the round cd->mt.
 * Runs on dsched_globals.evbase. Each scheduler proposes from the
 * round's snapshot of the pool and the pending requests - on its own
 * thread, if it likes, since it touches nothing else - and answers
 * through dsched_sched_base_cbfunc. */
void dsched_sched_base_schedule(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    pmix_status_t rc;
    dsched_op_tracker_t *trk;
    dsched_sched_base_active_module_t *mod;
    int order;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    PMIX_ACQUIRE_OBJECT(cd);

    // cd holds the tracker's only reference, and releases it with itself
    trk = PMIX_NEW(dsched_op_tracker_t);
    trk->cbdata = cd;
    cd->trk = trk;
    PMIX_LIST_FOREACH (mod, &dsched_sched_globals.actives, dsched_sched_base_active_module_t) {
        if (NULL != mod->module->schedule) {
            trk->nactive++;
        }
    }
    order = 1;

    PMIX_LIST_FOREACH (mod, &dsched_sched_globals.actives, dsched_sched_base_active_module_t) {
        if (NULL != mod->module->schedule) {
            rc = mod->module->schedule(cd, order);
            if (PMIX_SUCCESS != rc &&
                PMIX_OPERATION_IN_PROGRESS != rc) {
                trk->nresponded++;
            }
            ++order;
        }
    }

    if (trk->nresponded == trk->nactive) {
        // no scheduler is answering - the round has no proposals
        DSCHED_THREADSHIFT(cd, dsched_globals.evbase, cd->evcbfunc);
    }
}

PMIX_CLASS_INSTANCE(dsched_sched_base_active_module_t,
                    pmix_list_item_t,
                    NULL, NULL);
