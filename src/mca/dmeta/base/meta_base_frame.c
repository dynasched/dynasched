/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technometay
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
 *                         and Technometay (RIST).  All rights reserved.
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

#include "src/include/dsched_globals.h"
#include "src/mca/dmeta/base/base.h"
#include "src/util/pmix_show_help.h"

#include "src/mca/dmeta/base/static-components.h"

/* Instantiate the global vars */
dsched_meta_globals_t dsched_meta_globals = {
    .actives = PMIX_LIST_STATIC_INIT,
    .initialized = false,
    .selected = false
};

static pmix_status_t dsched_dmeta_base_close(void)
{
    dsched_meta_base_active_module_t *active;
    pmix_status_t rc;

    if (!dsched_meta_globals.initialized) {
        return PMIX_SUCCESS;
    }

    PMIX_LIST_FOREACH(active, &dsched_meta_globals.actives, dsched_meta_base_active_module_t) {
        if (NULL != active->module->finalize) {
            active->module->finalize();
        }
    }
    PMIX_LIST_DESTRUCT(&dsched_meta_globals.actives);

    /* Close all active components */
    rc = pmix_mca_base_framework_components_close(&dsched_dmeta_base_framework, NULL);
    // mark as uninitialized
    dsched_meta_globals.initialized = false;
    return rc;
}

/**
 *  * Function for finding and opening either all MCA components, or the one
 *   * that was specifically requested via a MCA parameter.
 *    */
static pmix_status_t dsched_dmeta_base_open(pmix_mca_base_open_flag_t flags)
{
    // initialize globals
    PMIX_CONSTRUCT(&dsched_meta_globals.actives, pmix_list_t);
    dsched_meta_globals.initialized = true;

    /* Open up all available components */
    return pmix_mca_base_framework_components_open(&dsched_dmeta_base_framework, flags);
}

PMIX_MCA_BASE_FRAMEWORK_DECLARE(dsched, dmeta, "DynaSched meta plugins", NULL, dsched_dmeta_base_open,
                                dsched_dmeta_base_close, dsched_mca_dmeta_base_static_components,
                                PMIX_MCA_BASE_FRAMEWORK_FLAG_DEFAULT);

void dsched_meta_base_cbfunc(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    dsched_meta_t *mt = cd->mt;
    dsched_req_t *req = mt->req;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    mt->nresponded++;

    if (mt->nresponded == mt->nactive) {
        // all the metaschedulers have completed
        if (NULL != req) {
            // reply to requestor
            if (NULL != req->cbfunc) {
                req->cbfunc(req->status, NULL, 0, req->cbdata, NULL, NULL);
            }
            pmix_pointer_array_set_item(&dsched_globals.requests, req->index, NULL);
            PMIX_RELEASE(req);
        }
        PMIX_RELEASE(mt);
    }
    PMIX_RELEASE(cd);
}

void dsched_meta_base_schedule(int sd, short args, void *cbdata)
{
    dsched_req_t *req = (dsched_req_t*)cbdata;
    pmix_status_t rc;
    dsched_meta_t *mt;
    dsched_meta_base_active_module_t *mod;
    bool assign = true;
    dsched_shift_caddy_t *cd;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    // NOTE: a NULL req indicates that this was triggered
    // by completion of an executing job (either the job
    // completed, or the session timed out). Schedulers
    // shall indicate which pending req is being allocated.
    // A non-NULL req indicates that a new request has
    // arrived and needs to be added to the scheduler.
    mt = PMIX_NEW(dsched_meta_t);
    if (NULL != req) {
        PMIX_RETAIN(req);
    }
    mt->req = req;
    mt->nactive = pmix_list_get_size(&dsched_meta_globals.actives);

    PMIX_LIST_FOREACH (mod, &dsched_meta_globals.actives, dsched_meta_base_active_module_t) {
        if (NULL != mod->module->schedule) {
            rc = mod->module->schedule(mt, assign);
            if (PMIX_SUCCESS != rc &&
                PMIX_OPERATION_IN_PROGRESS != rc) {
                mt->nresponded++;
            } else {
                assign = false;
            }
        }
    }

    if (mt->nactive == mt->nresponded) {
        req->status = PMIX_ERR_NOT_AVAILABLE;
        cd = PMIX_NEW(dsched_shift_caddy_t);
        cd->mt = mt;
        DSCHED_THREADSHIFT(cd, dsched_globals.evbase, dsched_meta_base_cbfunc);
    }
}


PMIX_CLASS_INSTANCE(dsched_meta_base_active_module_t,
                    pmix_list_item_t,
                    NULL, NULL);
