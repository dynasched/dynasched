/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
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
 * Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2012-2015 Los Alamos National Security, Inc.  All rights
 *                         reserved.
 * Copyright (c) 2019      Intel, Inc.  All rights reserved.
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

#include "src/mca/dsched/base/base.h"

int dsched_dsched_base_select(void)
{
    int exit_status = DSCHED_SUCCESS;
    dsched_dsched_base_component_t *best_component = NULL;
    dsched_dsched_base_module_t *best_module = NULL;

    /*
     * Select the best component
     */
    if (DSCHED_SUCCESS
        != pmix_mca_base_select("dsched", dsched_dsched_base_framework.framework_output,
                                &dsched_dsched_base_framework.framework_components,
                                (pmix_mca_base_module_t **) &best_module,
                                (pmix_mca_base_component_t **) &best_component, NULL)) {
        /* This will only happen if no component was selected */
        exit_status = DSCHED_ERROR;
        goto cleanup;
    }

    /* Save the winner */
    dsched_sched = *best_module;

    /* Initialize the winner */
    if (NULL != dsched_sched.init) {
        if (DSCHED_SUCCESS != dsched_sched.init()) {
            exit_status = DSCHED_ERROR;
            goto cleanup;
        }
    }

cleanup:
    return exit_status;
}
