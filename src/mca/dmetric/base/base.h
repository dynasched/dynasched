/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technometricy
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2011 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2012-2013 Los Alamos National Security, Inc.  All rights reserved.
 * Copyright (c) 2017-2019 Intel, Inc.  All rights reserved.
 * Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/** @file:
 */

#ifndef DSCHED_MCA_METRIC_BASE_H
#define DSCHED_MCA_METRIC_BASE_H

/*
 * includes
 */
#include "dsched_config.h"
#include "dsched_constants.h"

#include "src/mca/dmetric/dmetric.h"

BEGIN_C_DECLS

/*
 * MCA Framework
 */
DSCHED_EXPORT extern pmix_mca_base_framework_t dsched_dmetric_base_framework;
/* select a component */
DSCHED_EXPORT pmix_status_t dsched_dmetric_base_select(void);

/**
 * Track an active component / module
 */
typedef struct {
    pmix_list_item_t super;
    int pri;
    dsched_dmetric_module_t *module;
    dsched_dmetric_base_component_t *component;
} dsched_metric_base_active_module_t;
PMIX_CLASS_DECLARATION(dsched_metric_base_active_module_t);

/* framework globals */
typedef struct {
    pmix_list_t actives;
    bool initialized;
    bool selected;
} dsched_metric_globals_t;

DSCHED_EXPORT extern dsched_dmetric_module_t dsched_dmetric;

DSCHED_EXPORT extern dsched_metric_globals_t dsched_metric_globals;

DSCHED_EXPORT pmix_status_t dsched_metric_base_compute(pmix_list_t *data);

END_C_DECLS

#endif
