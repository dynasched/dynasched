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
    .actives = PMIX_LIST_STATIC_INIT(dsched_meta_globals.actives),
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

DSCHED_MCA_BASE_FRAMEWORK_DECLARE(dmeta, "DynaSched meta plugins", NULL, dsched_dmeta_base_open,
                                  dsched_dmeta_base_close, dsched_mca_dmeta_base_static_components,
                                  PMIX_MCA_BASE_FRAMEWORK_FLAG_DEFAULT);

/* A meta component has answered for the round. Runs on
 * dsched_globals.evbase. */
void dsched_meta_base_cbfunc(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t*)cbdata;
    dsched_meta_t *mt = cd->mt;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    PMIX_RELEASE(cd);
    mt->nresponded++;
    if (mt->nresponded == mt->nactive && NULL != mt->cbfunc) {
        // all the meta components have completed
        mt->cbfunc(mt);
    }
}

void dsched_meta_base_schedule(dsched_meta_t *mt)
{
    pmix_status_t rc;
    dsched_meta_base_active_module_t *mod;
    bool assign = true;

    mt->nactive = 0;
    mt->nresponded = 0;
    PMIX_LIST_FOREACH (mod, &dsched_meta_globals.actives, dsched_meta_base_active_module_t) {
        if (NULL != mod->module->schedule) {
            mt->nactive++;
        }
    }

    /* Only one meta component chooses the proposal that is applied: the
     * first that takes the round on. The others see it too, so that
     * they can weigh it, but what they choose is not applied. Every
     * answer comes back through dsched_meta_base_cbfunc, posted to this
     * same event base, so none can arrive before this loop is done. */
    PMIX_LIST_FOREACH (mod, &dsched_meta_globals.actives, dsched_meta_base_active_module_t) {
        if (NULL == mod->module->schedule) {
            continue;
        }
        rc = mod->module->schedule(mt, assign);
        if (PMIX_SUCCESS != rc && PMIX_OPERATION_IN_PROGRESS != rc) {
            mt->nresponded++;
        } else {
            assign = false;
        }
    }

    if (mt->nactive == mt->nresponded && NULL != mt->cbfunc) {
        // nothing took the round on - it ends with no proposal
        mt->cbfunc(mt);
    }
}


PMIX_CLASS_INSTANCE(dsched_meta_base_active_module_t,
                    pmix_list_item_t,
                    NULL, NULL);
