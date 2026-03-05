/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2015      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef DSCHED_PDL_BASE_H
#define DSCHED_PDL_BASE_H

#include "src/include/dsched_config.h"
#include "src/mca/ddl/ddl.h"
#include "src/util/pmix_environ.h"

#include "src/mca/base/pmix_base.h"

BEGIN_C_DECLS

/**
 * Globals
 */
DSCHED_EXPORT extern pmix_mca_base_framework_t dsched_ddl_base_framework;
extern dsched_ddl_base_component_t *dsched_ddl_base_selected_component;
extern dsched_ddl_base_module_t *dsched_ddl;

/**
 * Initialize the PDL MCA framework
 *
 * @retval DSCHED_SUCCESS Upon success
 * @retval DSCHED_ERROR   Upon failures
 *
 * This function is invoked during dsched_init();
 */
int dsched_ddl_base_open(pmix_mca_base_open_flag_t flags);

/**
 * Select an available component.
 *
 * @retval DSCHED_SUCCESS Upon Success
 * @retval DSCHED_NOT_FOUND If no component can be selected
 * @retval DSCHED_ERROR Upon other failure
 *
 */
int dsched_ddl_base_select(void);

/**
 * Finalize the PDL MCA framework
 *
 * @retval DSCHED_SUCCESS Upon success
 * @retval DSCHED_ERROR   Upon failures
 *
 * This function is invoked during dsched_finalize();
 */
int dsched_ddl_base_close(void);

/**
 * Open a DSO
 *
 * (see dsched_ddl_base_module_open_ft_t in src/mca/ddl/ddl.h for
 * documentation of this function)
 */
int dsched_ddl_open(const char *fname, bool use_ext, bool private_namespace,
                  dsched_ddl_handle_t **handle, char **err_msg);

/**
 * Lookup a symbol in a DSO
 *
 * (see dsched_ddl_base_module_lookup_ft_t in src/mca/ddl/ddl.h for
 * documentation of this function)
 */
int dsched_ddl_lookup(dsched_ddl_handle_t *handle, const char *symbol, void **ptr, char **err_msg);

/**
 * Close a DSO
 *
 * (see dsched_ddl_base_module_close_ft_t in src/mca/ddl/ddl.h for
 * documentation of this function)
 */
int dsched_ddl_close(dsched_ddl_handle_t *handle);

/**
 * Iterate over files in a path
 *
 * (see dsched_ddl_base_module_foreachfile_ft_t in src/mca/ddl/ddl.h for
 * documentation of this function)
 */
int dsched_ddl_foreachfile(const char *search_path,
                         int (*cb_func)(const char *filename, void *context), void *context);

END_C_DECLS

#endif /* DSCHED_PDL_BASE_H */
