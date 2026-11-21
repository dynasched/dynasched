/*
 * Copyright (c) 2015      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2016      IBM Corporation.  All rights reserved.
 * Copyright (c) 2017-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "src/include/dsched_config.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "src/include/dsched_constants.h"
#include "src/include/dsched_globals.h"
#include "src/mca/ddl/ddl.h"

#include "ddl_libltdl.h"

/*
 * Per the framework contract (see ddl.h), an error message handed back
 * through err_msg points to storage the caller does not own and must not
 * free. libltdl's lt_dlerror() returns exactly that, so pass it through
 * rather than duplicating it -- a copy would have no owner to free it.
 */
static void set_err_msg(char **err_msg)
{
    if (NULL != err_msg) {
        *err_msg = (char *) lt_dlerror();
    }
}

static int dlibltdl_open(const char *fname, bool use_ext, bool private_namespace,
                         dsched_ddl_handle_t **handle, char **err_msg)
{
    lt_dlhandle local_handle = NULL;

    assert(handle);

    *handle = NULL;
    if (NULL != err_msg) {
        *err_msg = NULL;
    }

#if DSCHED_DDL_DLIBLTDL_HAVE_LT_DLADVISE
    dsched_ddl_dlibltdl_component_t *c = &dsched_mca_ddl_dlibltdl_component;

    if (use_ext && private_namespace) {
        local_handle = lt_dlopenadvise(fname, c->advise_private_ext);
    } else if (use_ext && !private_namespace) {
        local_handle = lt_dlopenadvise(fname, c->advise_public_ext);
    } else if (!use_ext && private_namespace) {
        local_handle = lt_dlopenadvise(fname, c->advise_private_noext);
    } else {
        local_handle = lt_dlopenadvise(fname, c->advise_public_noext);
    }
#else
    /* Without lt_dladvise there is no way to say private vs. public, so
       that argument is simply not expressible here. */
    DSCHED_HIDE_UNUSED_PARAMS(private_namespace);

    if (use_ext) {
        local_handle = lt_dlopenext(fname);
    } else {
        local_handle = lt_dlopen(fname);
    }
#endif

    if (NULL == local_handle) {
        set_err_msg(err_msg);
        return DSCHED_ERROR;
    }

    *handle = calloc(1, sizeof(dsched_ddl_handle_t));
    if (NULL == *handle) {
        lt_dlclose(local_handle);
        return DSCHED_ERR_OUT_OF_RESOURCE;
    }
    (*handle)->ltdl_handle = local_handle;

#if DSCHED_ENABLE_DEBUG
    if (NULL != fname) {
        (*handle)->filename = strdup(fname);
    } else {
        (*handle)->filename = strdup("(null)");
    }
#endif

    return DSCHED_SUCCESS;
}

static int dlibltdl_lookup(dsched_ddl_handle_t *handle, const char *symbol, void **ptr,
                           char **err_msg)
{
    assert(handle);
    assert(handle->ltdl_handle);
    assert(symbol);
    assert(ptr);

    if (NULL != err_msg) {
        *err_msg = NULL;
    }

    *ptr = lt_dlsym(handle->ltdl_handle, symbol);
    if (NULL != *ptr) {
        return DSCHED_SUCCESS;
    }

    set_err_msg(err_msg);
    return DSCHED_ERROR;
}

static int dlibltdl_close(dsched_ddl_handle_t *handle)
{
    int ret;

    assert(handle);

    ret = lt_dlclose(handle->ltdl_handle);

#if DSCHED_ENABLE_DEBUG
    free(handle->filename);
#endif
    free(handle);

    return (0 == ret) ? DSCHED_SUCCESS : DSCHED_ERROR;
}

static int dlibltdl_foreachfile(const char *search_path,
                                int (*func)(const char *filename, void *data), void *data)
{
    int ret;

    assert(search_path);
    assert(func);

    ret = lt_dlforeachfile(search_path, func, data);
    return (0 == ret) ? DSCHED_SUCCESS : DSCHED_ERROR;
}

/*
 * Module definition
 */
dsched_ddl_base_module_t dsched_ddl_dlibltdl_module = {
    .open = dlibltdl_open,
    .lookup = dlibltdl_lookup,
    .close = dlibltdl_close,
    .foreachfile = dlibltdl_foreachfile
};
