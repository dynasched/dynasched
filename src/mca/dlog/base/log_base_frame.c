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

#include "src/mca/dlog/base/base.h"
#include "src/util/pmix_show_help.h"

#include "src/mca/dlog/base/static-components.h"

/* Instantiate the global vars */
dsched_log_globals_t dsched_log_globals = {
    .actives = PMIX_LIST_STATIC_INIT,
    .initialized = false,
    .selected = false
};

dsched_dlog_module_t dlog_log = {
    .init = NULL,
    .finalize = NULL,
    .log = dsched_log_base_log
};

static pmix_status_t dsched_dlog_base_close(void)
{
    dsched_log_base_active_module_t *active;
    pmix_status_t rc;

    if (!dsched_log_globals.initialized) {
        return PMIX_SUCCESS;
    }

    PMIX_LIST_FOREACH(active, &dsched_log_globals.actives, dsched_log_base_active_module_t) {
        if (NULL != active->module->finalize) {
            active->module->finalize();
        }
    }
    PMIX_LIST_DESTRUCT(&dsched_log_globals.actives);

    /* Close all active components */
    rc = pmix_mca_base_framework_components_close(&dsched_dlog_base_framework, NULL);
    // mark as uninitialized
    dsched_log_globals.initialized = false;
    return rc;
}

/**
 *  * Function for finding and opening either all MCA components, or the one
 *   * that was specifically requested via a MCA parameter.
 *    */
static pmix_status_t dsched_dlog_base_open(pmix_mca_base_open_flag_t flags)
{
    // initialize globals
    PMIX_CONSTRUCT(&dsched_log_globals.actives, pmix_list_t);
    dsched_log_globals.initialized = true;

    /* Open up all available components */
    return pmix_mca_base_framework_components_open(&dsched_dlog_base_framework, flags);
}

PMIX_MCA_BASE_FRAMEWORK_DECLARE(dsched, dlog, "DynaSched log plugins", NULL, dsched_dlog_base_open,
                                dsched_dlog_base_close, dsched_mca_dlog_base_static_components,
                                PMIX_MCA_BASE_FRAMEWORK_FLAG_DEFAULT);

pmix_status_t dsched_log_base_log(pmix_list_t *data)
{
    pmix_status_t rc;
    dsched_log_base_active_module_t *mod;

    PMIX_LIST_FOREACH (mod, &dsched_log_globals.actives, dsched_log_base_active_module_t) {
        if (NULL != mod->module->log) {
            rc = mod->module->log(data);
            if (PMIX_SUCCESS != rc) {
                return rc;
            }
        }
    }
    return PMIX_SUCCESS;
}


PMIX_CLASS_INSTANCE(dsched_log_base_active_module_t,
                    pmix_list_item_t,
                    NULL, NULL);
