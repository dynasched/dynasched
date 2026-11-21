/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2015       Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2015       Los Alamos National Security, Inc.  All rights
 *                          reserved.
 * Copyright (c) 2017-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "src/include/dsched_config.h"

#include "src/include/dsched_constants.h"

#include "src/mca/base/pmix_mca_base_var.h"
#include "src/mca/ddl/ddl.h"

#include "ddl_libltdl.h"

/*
 * Public string showing the ddl dlibltdl component version number
 */
const char *dsched_ddl_dlibltdl_component_version_string
    = "DSCHED ddl dlibltdl MCA component version " DSCHED_VERSION;

/*
 * Local functions
 */
static int dlibltdl_component_register(void);
static int dlibltdl_component_open(void);
static int dlibltdl_component_close(void);
static int dlibltdl_component_query(pmix_mca_base_module_t **module, int *priority);

/*
 * Instantiate the public struct with all of our public information
 * and pointers to our public functions in it
 */

dsched_ddl_dlibltdl_component_t dsched_mca_ddl_dlibltdl_component = {

    /* Fill in the dsched_ddl_base_component_t */
    .base = {

        /* First, the pmix_mca_base_component_t struct containing meta
           information about the component itself */
        .base_version = {
            DSCHED_MCA_BASE_VERSION(ddl),

            /* Component name and version */
            .pmix_mca_component_name = "dlibltdl",
            PMIX_MCA_BASE_MAKE_VERSION(component,
                                       DSCHED_MAJOR_VERSION,
                                       DSCHED_MINOR_VERSION,
                                       DSCHED_RELEASE_VERSION),

            /* Component functions */
            .pmix_mca_register_component_params = dlibltdl_component_register,
            .pmix_mca_open_component = dlibltdl_component_open,
            .pmix_mca_close_component = dlibltdl_component_close,
            .pmix_mca_query_component = dlibltdl_component_query,
        },

        /* The ddl framework members */
        .priority = 50
    },

    /* Now fill in the dlibltdl component-specific members */
};
PMIX_MCA_BASE_COMPONENT_INIT(dsched, ddl, dlibltdl)

/* Storage for the info parameter below. Registration resolves the value
   through this pointer and keeps it, so it has to outlive the call --
   hence file scope rather than a local. */
static bool dlibltdl_have_lt_dladvise = DSCHED_INT_TO_BOOL(DSCHED_DDL_DLIBLTDL_HAVE_LT_DLADVISE);

static int dlibltdl_component_register(void)
{
    int ret;

    /* Register an info param indicating whether we have lt_dladvise
       support or not */
    ret = pmix_mca_base_component_var_register(&dsched_mca_ddl_dlibltdl_component.base.base_version,
                                               "have_lt_dladvise",
                                               "Whether the version of libltdl that this component "
                                               "is built against supports lt_dladvise "
                                               "functionality or not",
                                               PMIX_MCA_BASE_VAR_TYPE_BOOL,
                                               &dlibltdl_have_lt_dladvise);
    if (0 > ret) {
        return ret;
    }

    return DSCHED_SUCCESS;
}

static int dlibltdl_component_open(void)
{
    if (lt_dlinit()) {
        return DSCHED_ERROR;
    }

#if DSCHED_DDL_DLIBLTDL_HAVE_LT_DLADVISE
    dsched_ddl_dlibltdl_component_t *c = &dsched_mca_ddl_dlibltdl_component;

    if (lt_dladvise_init(&c->advise_private_noext)) {
        return DSCHED_ERR_OUT_OF_RESOURCE;
    }

    if (lt_dladvise_init(&c->advise_private_ext) ||
        lt_dladvise_ext(&c->advise_private_ext)) {
        return DSCHED_ERR_OUT_OF_RESOURCE;
    }

    if (lt_dladvise_init(&c->advise_public_noext) ||
        lt_dladvise_global(&c->advise_public_noext)) {
        return DSCHED_ERR_OUT_OF_RESOURCE;
    }

    if (lt_dladvise_init(&c->advise_public_ext) ||
        lt_dladvise_global(&c->advise_public_ext) ||
        lt_dladvise_ext(&c->advise_public_ext)) {
        return DSCHED_ERR_OUT_OF_RESOURCE;
    }
#endif

    return DSCHED_SUCCESS;
}

static int dlibltdl_component_close(void)
{
#if DSCHED_DDL_DLIBLTDL_HAVE_LT_DLADVISE
    dsched_ddl_dlibltdl_component_t *c = &dsched_mca_ddl_dlibltdl_component;

    lt_dladvise_destroy(&c->advise_private_noext);
    lt_dladvise_destroy(&c->advise_private_ext);
    lt_dladvise_destroy(&c->advise_public_noext);
    lt_dladvise_destroy(&c->advise_public_ext);
#endif

    lt_dlexit();

    return DSCHED_SUCCESS;
}

static int dlibltdl_component_query(pmix_mca_base_module_t **module, int *priority)
{
    /* The priority value is somewhat meaningless here; by
       src/mca/ddl/configure.m4, there's at most one component
       available. */
    *priority = dsched_mca_ddl_dlibltdl_component.base.priority;
    *module = &dsched_ddl_dlibltdl_module.super;

    return DSCHED_SUCCESS;
}
