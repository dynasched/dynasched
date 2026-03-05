/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2015 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2015      Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2020      Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "src/include/dsched_config.h"

#include "src/include/dsched_constants.h"

#include "src/mca/ddl/ddl.h"
#include "src/util/pmix_argv.h"

#include "ddl_ddlopen.h"

/*
 * Public string showing the sysinfo ompi_linux component version number
 */
const char *dsched_ddl_ddlopen_component_version_string
    = "DSCHED ddl ddlopen MCA component version " DSCHED_VERSION;

/*
 * Local functions
 */
static int ddlopen_component_register(void);
static int ddlopen_component_open(void);
static int ddlopen_component_close(void);
static int ddlopen_component_query(pmix_mca_base_module_t **module, int *priority);

/*
 * Instantiate the public struct with all of our public information
 * and pointers to our public functions in it
 */

dsched_ddl_ddlopen_component_t dsched_mca_ddl_ddlopen_component = {

    /* Fill in the mca_ddl_base_component_t */
    .base = {

        /* First, the mca_component_t struct containing meta information
           about the component itself */
        .base_version = {
            DSCHED_DDL_BASE_VERSION_1_0_0,

            /* Component name and version */
            .pmix_mca_component_name = "ddlopen",
            PMIX_MCA_BASE_MAKE_VERSION(component,
                                       DSCHED_MAJOR_VERSION,
                                       DSCHED_MINOR_VERSION,
                                       DSCHED_RELEASE_VERSION),

            /* Component functions */
            .pmix_mca_register_component_params = ddlopen_component_register,
            .pmix_mca_open_component = ddlopen_component_open,
            .pmix_mca_close_component = ddlopen_component_close,
            .pmix_mca_query_component = ddlopen_component_query,
        },

        /* The ddl framework members */
        .priority = 80
    },
};
PMIX_MCA_BASE_COMPONENT_INIT(dsched, ddl, ddlopen)

static int ddlopen_component_register(void)
{
    int ret;

    dsched_mca_ddl_ddlopen_component.filename_suffixes_mca_storage = ".so,.dylib,.dll,.sl";
    ret = pmix_mca_base_component_var_register(
        &dsched_mca_ddl_ddlopen_component.base.base_version, "filename_suffixes",
        "Comma-delimited list of filename suffixes that the ddlopen component will try",
        PMIX_MCA_BASE_VAR_TYPE_STRING,
        &dsched_mca_ddl_ddlopen_component.filename_suffixes_mca_storage);
    if (ret < 0) {
        return ret;
    }
    dsched_mca_ddl_ddlopen_component.filename_suffixes
        = PMIx_Argv_split(dsched_mca_ddl_ddlopen_component.filename_suffixes_mca_storage, ',');

    return DSCHED_SUCCESS;
}

static int ddlopen_component_open(void)
{
    return DSCHED_SUCCESS;
}

static int ddlopen_component_close(void)
{
    if (NULL != dsched_mca_ddl_ddlopen_component.filename_suffixes) {
        PMIx_Argv_free(dsched_mca_ddl_ddlopen_component.filename_suffixes);
        dsched_mca_ddl_ddlopen_component.filename_suffixes = NULL;
    }

    return DSCHED_SUCCESS;
}

static int ddlopen_component_query(pmix_mca_base_module_t **module, int *priority)
{
    /* The priority value is somewhat meaningless here; by
       src/mca/ddl/configure.m4, there's at most one component
       available. */
    *priority = dsched_mca_ddl_ddlopen_component.base.priority;
    *module = &dsched_ddl_ddlopen_module.super;

    return DSCHED_SUCCESS;
}
