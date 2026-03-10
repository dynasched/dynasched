/*
 * Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2004-2011 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2019      Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * Copyright (c) 2016-2019 Intel, Inc.  All rights reserved.
 *
 * Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/**
 * @file
 *
 */

#ifndef dsched_sched_fifo_EXPORT_H
#define dsched_sched_fifo_EXPORT_H

#include "dsched_config.h"

#include "src/mca/dsched/dsched.h"

BEGIN_C_DECLS

/*
 * Local Component structures
 */

DSCHED_MODULE_EXPORT extern dsched_dsched_base_component_t dsched_mca_dsched_fifo_component;

DSCHED_EXPORT extern dsched_dsched_base_module_t dsched_sched_fifo_module;

END_C_DECLS

#endif /* dsched_sched_fifo_EXPORT_H */
