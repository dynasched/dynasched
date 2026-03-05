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

#include "dsched_config.h"

#include "dsched_constants.h"
#include "src/mca/ddl/ddl.h"

#include "ddl_libltdl.h"

static int dlibltddl_open(const char *fname, bool use_ext, bool private_namespace,
                          dsched_ddl_handle_t **handle, char **err_msg)
{
    assert(handle);

    *handle = NULL;
    if (NULL != err_msg) {
        *err_msg = NULL;
    }

    lt_dlhandle local_handle;

#if DSCHED_DDL_DLIBLTDL_HAVE_LT_DLADVISE
    dsched_ddl_dlibltddl_component_t *c = &dshed_mca_ddl_dlibltddl_component;

    if (use_ext && private_namespace) {
        local_handle = lt_dlopenadvise(fname, c->advise_private_ext);
    } else if (use_ext && !private_namespace) {
        local_handle = lt_dlopenadvise(fname, c->advise_public_ext);
    } else if (!use_ext && private_namespace) {
        local_handle = lt_dlopenadvise(fname, c->advise_private_noext);
    } else if (!use_ext && !private_namespace) {
        local_handle = lt_dlopenadvise(fname, c->advise_public_noext);
    }
#else
    if (use_ext) {
        local_handle = lt_dlopenext(fname);
    } else {
        local_handle = lt_dlopen(fname);
    }
#endif

    if (NULL != local_handle) {
        *handle = calloc(1, sizeof(dsched_ddl_handle_t));
        (*handle)->ltddl_handle = local_handle;

#if DSCHED_ENABLE_DEBUG
        if (NULL != fname) {
            (*handle)->filename = strdup(fname);
        } else {
            (*handle)->filename = strdup("(null)");
        }
#endif

        return DSCHED_SUCCESS;
    }

    if (NULL != err_msg) {
        *err_msg = strdup((char *) lt_dlerror());
    }
    return DSCHED_ERROR;
}

static int dlibltddl_lookup(dsched_ddl_handle_t *handle, const char *symbol, void **ptr,
                            char **err_msg)
{
    assert(handle);
    assert(handle->ltddl_handle);
    assert(symbol);
    assert(ptr);

    if (NULL != err_msg) {
        *err_msg = NULL;
    }

    *ptr = lt_dlsym(handle->ltddl_handle, symbol);
    if (NULL != *ptr) {
        return DSCHED_SUCCESS;
    }

    if (NULL != err_msg) {
        *err_msg = strdup((char *) lt_dlerror());
    }
    return DSCHED_ERROR;
}

static int dlibltddl_close(dsched_ddl_handle_t *handle)
{
    assert(handle);

    int ret;
    ret = lt_dlclose(handle->ltddl_handle);

#if DSCHED_ENABLE_DEBUG
    free(handle->filename);
#endif
    free(handle);

    return ret;
}

static int dlibltddl_foreachfile(const char *search_path,
                                 int (*func)(const char *filename, void *data), void *data)
{
    assert(search_path);
    assert(func);

    int ret = lt_dlforeachfile(search_path, func, data);
    return (0 == ret) ? DSCHED_SUCCESS : DSCHED_ERROR;
}

/*
 * Module definition
 */
dsched_ddl_base_module_t dsched_ddl_dlibltddl_module = {
    .open = dlibltddl_open,
    .lookup = dlibltddl_lookup,
    .close = dlibltddl_close,
    .foreachfile = dlibltddl_foreachfile
};
