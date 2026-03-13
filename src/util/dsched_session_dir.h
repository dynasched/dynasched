/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/** @file:
 *
 * Find and/or create DSCHED session directory.
 *
 */

#ifndef DSCHED_SESSION_DIR_H
#define DSCHED_SESSION_DIR_H

#include "dsched_config.h"

BEGIN_C_DECLS

DSCHED_EXPORT int dsched_session_dir_init(void);

DSCHED_EXPORT void dsched_session_dir_finalize(void);

END_C_DECLS

#endif /* DSCHED_SESSION_DIR_H */
