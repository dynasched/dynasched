/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University.
 *                         All rights reserved.
 *
 * Copyright (c) 2015 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2020      Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 * Copyright (c) 2015      Los Alamos National Security, LLC. All rights
 *                         reserved.
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "src/include/dsched_config.h"

#ifdef HAVE_UNISTD_H
#    include "unistd.h"
#endif

#include "src/mca/base/pmix_base.h"
#include "src/mca/mca.h"
#include "src/mca/ddl/base/base.h"
#include "src/mca/ddl/ddl.h"
#include "src/util/pmix_output.h"

int dsched_ddl_base_select(void)
{
    int exit_status = PMIX_SUCCESS;
    dsched_ddl_base_component_t *best_component = NULL;
    dsched_ddl_base_module_t *best_module = NULL;

    /*
     * Select the best component
     */
    if (PMIX_SUCCESS
        != pmix_mca_base_select("ddl", dsched_ddl_base_framework.framework_output,
                                &dsched_ddl_base_framework.framework_components,
                                (pmix_mca_base_module_t **) &best_module,
                                (pmix_mca_base_component_t **) &best_component, NULL)) {
        /* This will only happen if no component was selected */
        exit_status = PMIX_ERROR;
        goto cleanup;
    }

    /* Save the winner */
    dsched_ddl_base_selected_component = best_component;
    dsched_ddl = best_module;

cleanup:
    return exit_status;
}
