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
DSCHED_EXPORT int dsched_dsched_base_select(void);

DSCHED_EXPORT extern dsched_dsched_base_module_t dsched_dsched;

END_C_DECLS

#endif
