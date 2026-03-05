/*
 * Copyright (c) 2006-2013 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2007-2010 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2010      Sandia National Laboratories. All rights reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */

#ifndef DSCHED_DINSTALLDIRS_BASE_H
#define DSCHED_DINSTALLDIRS_BASE_H

#include "src/include/dsched_config.h"
#include "src/mca/base/pmix_mca_base_framework.h"
#include "src/mca/dinstalldirs/dinstalldirs.h"

/*
 * Global functions for MCA overall dinstalldirs open and close
 */
BEGIN_C_DECLS

/**
 * Framework structure declaration
 */
DSCHED_EXPORT extern pmix_mca_base_framework_t dsched_dinstalldirs_base_framework;

/* Just like dsched_dinstall_dirs_expand() (see dinstalldirs.h), but will
   also insert the value of the environment variable $DSCHED_DESTDIR, if
   it exists/is set.  This function should *only* be used during the
   setup routines of dinstalldirs. */
DSCHED_EXPORT char *dsched_dinstall_dirs_expand_setup(const char *input);

DSCHED_EXPORT int dsched_dinstall_dirs_base_init(pmix_info_t info[], size_t ninfo);

END_C_DECLS

#endif /* DSCHED_BASE_DINSTALLDIRS_H */
