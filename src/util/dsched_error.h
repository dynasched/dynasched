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
 * Copyright (c) 2017      FUJITSU LIMITED.  All rights reserved.
 * Copyright (c) 2019-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef DSCHED_UTIL_ERROR_H
#define DSCHED_UTIL_ERROR_H

#include "dsched_config.h"

#include "src/util/pmix_output.h"

BEGIN_C_DECLS

#define DSCHED_ERROR_LOG(r)                                             \
    do {                                                                \
        if (DSCHED_ERR_SILENT != (r)) {                                 \
            pmix_output(0, "DSCHED ERROR: %s in file %s at line %d",    \
                        dsched_strerror((r)), __FILE__, __LINE__);      \
        }                                                               \
    } while (0)


#define DSCHED_ERROR_NAME(n) dsched_strerror(n)


/**
 * Return string for given error message
 *
 * Accepts an error number argument \c errnum and returns a pointer to
 * the corresponding message string.  The result is returned in a
 * static buffer that should not be released with free().
 *
 * If errnum is \c DSCHED_ERR_IN_ERRNO, the system strerror is called
 * with an argument of the current value of \c errno and the resulting
 * string is returned.
 *
 * If the errnum is not a known value, the returned value may be
 * overwritten by subsequent calls to dsched_strerror.
 */
DSCHED_EXPORT const char *dsched_strerror(int errnum);

END_C_DECLS

#endif /* DSCHED_UTIL_ERROR_H */
