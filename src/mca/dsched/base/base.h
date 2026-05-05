/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
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

#ifndef DSCHED_MCA_SCHED_BASE_H
#define DSCHED_MCA_SCHED_BASE_H

/*
 * includes
 */
#include "dsched_config.h"
#include "dsched_constants.h"

#include "src/mca/dsched/dsched.h"

BEGIN_C_DECLS

/*
 * MCA Framework
 */
DSCHED_EXPORT extern pmix_mca_base_framework_t dsched_dsched_base_framework;
/* select a component */
DSCHED_EXPORT pmix_status_t dsched_dsched_base_select(void);

/**
 * Track an active component / module
 */
typedef struct {
    pmix_list_item_t super;
    int pri;
    dsched_dsched_module_t *module;
    dsched_dsched_base_component_t *component;
} dsched_sched_base_active_module_t;
PMIX_CLASS_DECLARATION(dsched_sched_base_active_module_t);

/* framework globals */
typedef struct {
    pmix_list_t actives;
    bool initialized;
    bool selected;
} dsched_sched_globals_t;

DSCHED_EXPORT extern dsched_sched_globals_t dsched_sched_globals;

DSCHED_EXPORT void dsched_sched_base_schedule(int sd, short args, void *cbdata);

DSCHED_EXPORT void dsched_sched_base_cbfunc(int sd, short args, void *cbdata);

END_C_DECLS

#endif
