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
    .actives = PMIX_LIST_STATIC_INIT,
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

PMIX_MCA_BASE_FRAMEWORK_DECLARE(dsched, dsched, "DynaSched scheduler plugins", NULL, dsched_dsched_base_open,
                                dsched_dsched_base_close, dsched_mca_dsched_base_static_components,
                                PMIX_MCA_BASE_FRAMEWORK_FLAG_DEFAULT);

void dsched_sched_base_cbfunc(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *scd = (dsched_shift_caddy_t*)cbdata;
    dsched_op_tracker_t *trk = scd->trk;
    dsched_alloc_t *alloc = scd->alloc;
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)trk->cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    trk->nresponded++;
    if (NULL != alloc) {
        pmix_pointer_array_add(&cd->allocations, alloc);
    }

    if (trk->nresponded == trk->nactive) {
        // all the schedulers have completed
        DSCHED_THREADSHIFT(cd, dsched_globals.evbase, cd->evcbfunc);
    }
}

void dsched_sched_base_schedule(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    pmix_status_t rc;
    dsched_op_tracker_t *trk;
    dsched_sched_base_active_module_t *mod;
    int order;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    // pass down to schedulers so they can each compute
    // an allocation based on the request and available
    // resources


    // if an allocation cannot be made at this time, then
    // the scheduler will return an estimated time for it
    // to become available, if possible

    // NOTE: a NULL req indicates that this was triggered
    // by completion of an executing job (either the job
    // completed, or the session timed out). Schedulers
    // shall indicate which pending req is being allocated.
    // A non-NULL req indicates that a new request has
    // arrived and needs to be added to the schedule.

    trk = PMIX_NEW(dsched_op_tracker_t);
    trk->cbdata = cd;
    cd->trk = trk;
    trk->nactive = pmix_list_get_size(&dsched_sched_globals.actives);
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
        cd->mt->req->status = PMIX_ERR_NOT_AVAILABLE;
        DSCHED_THREADSHIFT(cd, dsched_globals.evbase, cd->evcbfunc);
        PMIX_RELEASE(trk);
    }
}

PMIX_CLASS_INSTANCE(dsched_sched_base_active_module_t,
                    pmix_list_item_t,
                    NULL, NULL);
