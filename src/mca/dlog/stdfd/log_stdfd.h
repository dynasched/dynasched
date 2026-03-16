/* -*- C -*-
 *
 * Copyright (c) 2004-2008 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2009      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2014-2018 Intel, Inc. All rights reserved.
 * Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */
#ifndef DLOG_STDFD_H
#define DLOG_STDFD_H

#include "dsched_config.h"

#include "src/mca/dlog/dlog.h"

BEGIN_C_DECLS

/*
 * Plog interfaces
 */

DSCHED_EXPORT extern dsched_dlog_base_component_t dsched_mca_dlog_stdfd_component;
extern dsched_dlog_module_t dsched_dlog_stdfd_module;

END_C_DECLS

#endif
