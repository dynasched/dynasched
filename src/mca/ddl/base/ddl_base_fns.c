/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2015      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/**
 * This file is a simple set of wrappers around the selected DSCHED PDL
 * component (it's a compile-time framework with, at most, a single
 * component; see ddl.h for details).
 */

#include "src/include/dsched_config.h"
#include "src/include/dsched_constants.h"

#include "src/mca/ddl/base/base.h"
#include "src/util/pmix_output.h"

int dsched_ddl_open(const char *fname, bool use_ext, bool private_namespace,
                  dsched_ddl_handle_t **handle, char **err_msg)
{
    *handle = NULL;

    if (NULL != dsched_ddl && NULL != dsched_ddl->open) {
        return dsched_ddl->open(fname, use_ext, private_namespace, handle, err_msg);
    }

    return DSCHED_ERR_NOT_SUPPORTED;
}

int dsched_ddl_lookup(dsched_ddl_handle_t *handle, const char *symbol, void **ptr, char **err_msg)
{
    if (NULL != dsched_ddl && NULL != dsched_ddl->lookup) {
        return dsched_ddl->lookup(handle, symbol, ptr, err_msg);
    }

    return DSCHED_ERR_NOT_SUPPORTED;
}

int dsched_ddl_close(dsched_ddl_handle_t *handle)
{
    if (NULL != dsched_ddl && NULL != dsched_ddl->close) {
        return dsched_ddl->close(handle);
    }

    return DSCHED_ERR_NOT_SUPPORTED;
}

int dsched_ddl_foreachfile(const char *search_path,
                         int (*cb_func)(const char *filename, void *context), void *context)
{
    if (NULL != dsched_ddl && NULL != dsched_ddl->foreachfile) {
        return dsched_ddl->foreachfile(search_path, cb_func, context);
    }

    return DSCHED_ERR_NOT_SUPPORTED;
}
