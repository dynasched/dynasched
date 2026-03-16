/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2018-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/*
 * includes
 */
#include "dsched_config.h"
#include <pmix_common.h>

#include "src/mca/dmeta/pri/meta_pri.h"

static pmix_status_t component_query(pmix_mca_base_module_t **module, int *priority);

/*
 * Struct of function pointers that need to be initialized
 */
dsched_dmeta_base_component_t dsched_mca_dmeta_pri_component = {

    DSCHED_DMETA_BASE_VERSION_1_0_0,
    .pmix_mca_component_name = "pri",
    PMIX_MCA_BASE_MAKE_VERSION(component,
                               DSCHED_MAJOR_VERSION,
                               DSCHED_MINOR_VERSION,
                               DSCHED_RELEASE_VERSION),
    .pmix_mca_query_component = component_query,
};
PMIX_MCA_BASE_COMPONENT_INIT(dsched, dmeta, pri)

static pmix_status_t component_query(pmix_mca_base_module_t **module, int *priority)
{
    *priority = 5;
    *module = (pmix_mca_base_module_t *) &dsched_dmeta_pri_module;
    return PMIX_SUCCESS;
}
