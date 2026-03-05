/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2015      Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2016-2019 Intel, Inc.  All rights reserved.
 * Copyright (c) 2019      Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 *
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"
#include "src/util/pmix_output.h"

#include "src/mca/sched/base/base.h"
#include "sched_fifo.h"

/*
 * Public string for version number
 */
const char *dsched_mca_sched_fifo_component_version_string
    = "DSCHED FIFO sched MCA component version " DSCHED_VERSION;

/*
 * Local functionality
 */
static int sched_register(void);
static int sched_open(void);
static int sched_close(void);
static int sched_component_query(pmix_mca_base_module_t **module, int *priority);

/*
 * Instantiate the public struct with all of our public information
 * and pointer to our public functions in it
 */
dsched_sched_base_component_t dsched_mca_sched_fifo_component = {
    /* Handle the general mca_component_t struct containing
     *  meta information about the component sched
     */
    .base_version = {
        DSCHED_SCHED_BASE_VERSION_1_0_0,
        /* Component name and version */
        .pmix_mca_component_name = "fifo",
        PMIX_MCA_BASE_MAKE_VERSION(component,
                                   DSCHED_MAJOR_VERSION,
                                   DSCHED_MINOR_VERSION,
                                   DSCHED_RELEASE_VERSION),

        /* Component open and close functions */
        .pmix_mca_open_component = sched_open,
        .pmix_mca_close_component = sched_close,
        .pmix_mca_query_component = sched_component_query,
        .pmix_mca_register_component_params = sched_register,
    }
};
PMIX_MCA_BASE_COMPONENT_INIT(dsched, sched, fifo)

static int my_priority;

static int sched_register(void)
{
    pmix_mca_base_component_t *c = &dsched_mca_sched_fifo_component.base_version;

    my_priority = 1000;
    (void) pmix_mca_base_component_var_register(c, "priority",
                                                "Priority of the sched fifo component",
                                                PMIX_MCA_BASE_VAR_TYPE_INT,
                                                &my_priority);

    return DSCHED_SUCCESS;
}

static int sched_open(void)
{
    return DSCHED_SUCCESS;
}

static int sched_close(void)
{
    return DSCHED_SUCCESS;
}

static int sched_component_query(pmix_mca_base_module_t **module, int *priority)
{
    *priority = my_priority;
    *module = (pmix_mca_base_module_t *) &dsched_sched_fifo_module;
    return DSCHED_SUCCESS;
}
