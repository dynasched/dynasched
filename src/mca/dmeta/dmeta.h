/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technometay
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2011 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2010-2011 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2011-2015 Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2014      NVIDIA Corporation.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/** @file:
 *
 * The DSCHED Error and Recovery Manager (SchEd)
 *
 * This framework is the metaically central clearing house for process/daemon
 * state updates. In particular when a process fails and another process detects
 * it, then that information is reported through this framework. This framework
 * then (depending on the active component) decides how to handle the failure.
 *
 * For example, if a process fails this may activate an automatic recovery
 * of the process from a previous checkpoint, or initial state. Conversely,
 * the active component could decide not to continue the job, and request that
 * it be terminated. The error and recovery policy is determined by individual
 * components within this framework.
 *
 */

#ifndef DSCHED_MCA_META_H
#define DSCHED_MCA_META_H

/*
 * includes
 */

#include "dsched_config.h"
#include "dsched_constants.h"
#include "dsched_types.h"

#include "src/mca/base/pmix_base.h"
#include "src/mca/mca.h"

#include "src/class/pmix_object.h"
#include "src/class/pmix_pointer_array.h"
#include "src/util/pmix_output.h"

#include "src/include/dsched_globals.h"

BEGIN_C_DECLS

/*
 * Framework Interfaces
 */
/**
 * Module initialization function.
 *
 * @retval DSCHED_SUCCESS The operation completed successfully
 * @retval DSCHED_ERROR   An unspecifed error occurred
 */
typedef int (*dsched_dmeta_base_module_init_fn_t)(void);

/**
 * Module finalization function.
 *
 * @retval DSCHED_SUCCESS The operation completed successfully
 * @retval DSCHED_ERROR   An unspecifed error occurred
 */
typedef void (*dsched_dmeta_base_module_finalize_fn_t)(void);

// meta operation
typedef pmix_status_t (*dsched_dmeta_base_module_schedule_fn_t)(pmix_list_t *data);

/*
 * Module Structure
 */
typedef struct {
    /** Initialization Function */
    dsched_dmeta_base_module_init_fn_t init;
    /** Finalization Function */
    dsched_dmeta_base_module_finalize_fn_t finalize;
    // meta operation
    dsched_dmeta_base_module_schedule_fn_t schedule;
} dsched_dmeta_module_t;
DSCHED_EXPORT extern dsched_dmeta_module_t dsched_meta;

/*
 * Log Component
 */
typedef pmix_mca_base_component_t dsched_dmeta_base_component_t;

/*
 * Macro for use in components that are of type meta
 */
#define DSCHED_DMETA_BASE_VERSION_1_0_0 DSCHED_MCA_BASE_VERSION_1_0_0("dmeta", 1, 0, 0)

END_C_DECLS

#endif
