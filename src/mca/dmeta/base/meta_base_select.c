/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
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

#include "src/mca/dmeta/base/base.h"

pmix_status_t dsched_dmeta_base_select(void)
{
    pmix_mca_base_component_list_item_t *cli = NULL;
    pmix_mca_base_component_t *component = NULL;
    pmix_mca_base_module_t *module = NULL;
    dsched_dmeta_module_t *nmodule;
    dsched_meta_base_active_module_t *newmodule, *mod;
    int rc, priority;
    bool inserted;

    if (dsched_meta_globals.selected) {
        /* ensure we don't do this twice */
        return PMIX_SUCCESS;
    }
    dsched_meta_globals.selected = true;

    /* Query all available components and ask if they have a module */
    PMIX_LIST_FOREACH (cli, &dsched_dmeta_base_framework.framework_components,
                       pmix_mca_base_component_list_item_t) {
        component = (pmix_mca_base_component_t *) cli->cli_component;

        pmix_output_verbose(5, dsched_dmeta_base_framework.framework_output,
                            "dsched:meta:select: checking available component %s",
                            component->pmix_mca_component_name);

        /* If there's no query function, skip it */
        if (NULL == component->pmix_mca_query_component) {
            pmix_output_verbose(
                5, dsched_dmeta_base_framework.framework_output,
                "dsched:meta:select: Skipping component [%s]. It does not implement a query function",
                component->pmix_mca_component_name);
            continue;
        }

        /* Query the component */
        pmix_output_verbose(5, dsched_dmeta_base_framework.framework_output,
                            "dsched:meta:select: Querying component [%s]",
                            component->pmix_mca_component_name);
        rc = component->pmix_mca_query_component(&module, &priority);

        /* If no module was returned, then skip component */
        if (PMIX_SUCCESS != rc || NULL == module) {
            pmix_output_verbose(
                5, dsched_dmeta_base_framework.framework_output,
                "dsched:meta:select: Skipping component [%s]. Query failed to return a module",
                component->pmix_mca_component_name);
            continue;
        }

        /* If we got a module, keep it */
        nmodule = (dsched_dmeta_module_t *) module;
        /* let it initialize */
        if (NULL != nmodule->init && PMIX_SUCCESS != nmodule->init()) {
            continue;
        }
        /* add to the list of selected modules */
        newmodule = PMIX_NEW(dsched_meta_base_active_module_t);
        newmodule->pri = priority;
        newmodule->module = nmodule;
        newmodule->component = (dsched_dmeta_base_component_t *) cli->cli_component;

        /* maintain priority order */
        inserted = false;
        PMIX_LIST_FOREACH (mod, &dsched_meta_globals.actives, dsched_meta_base_active_module_t) {
            if (priority > mod->pri) {
                pmix_list_insert_pos(&dsched_meta_globals.actives, (pmix_list_item_t *) mod, &newmodule->super);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            /* must be lowest priority - add to end */
            pmix_list_append(&dsched_meta_globals.actives, &newmodule->super);
        }
    }

    return PMIX_SUCCESS;
}
