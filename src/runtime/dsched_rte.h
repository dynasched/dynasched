/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2007 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2008      Sun Microsystems, Inc.  All rights reserved.
 * Copyright (c) 2010-2022 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/** @file **/

#ifndef DSCHED_RTE_H
#define DSCHED_RTE_H

#include "src/include/dsched_config.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <event.h>

#include <pmix_common.h>

#include "src/include/dsched_globals.h"

BEGIN_C_DECLS

/**
 * Initialize the DSCHED layer, including the MCA system.
 *
 * @retval DSCHED_SUCCESS Upon success.
 * @retval DSCHED_ERROR Upon failure.
 *
 */
DSCHED_EXPORT int dsched_init_util(void);

DSCHED_EXPORT int dsched_init(pmix_info_t info[], size_t ninfo);

/**
 * Finalize the DSCHED layer, including the MCA system.
 *
 */
DSCHED_EXPORT void dsched_finalize(void);

DSCHED_EXPORT int dsched_register_params(void);

END_C_DECLS

#endif /* DSCHED_RTE_H */
