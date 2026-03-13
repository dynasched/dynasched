/*
 * Copyright (c) 2009-2011 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2010-2017 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2004-2011 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2011      Oracle and/or all its affiliates.  All rights reserved.
 * Copyright (c) 2011-2013 Los Alamos National Security, LLC.
 *                         All rights reserved.
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2017      IBM Corporation.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <string.h>
#ifdef HAVE_SYS_WAIT_H
#    include <sys/wait.h>
#endif

#include "src/mca/dsched/base/base.h"
#include "src/mca/dsched/dsched.h"

#include "sched_fifo.h"

static int init(void);
static int finalize(void);

/******************
 * dvm module
 ******************/
dsched_dsched_base_module_t dsched_sched_fifo_module = {
    .init = init,
    .finalize = finalize
};

/*
 * Local functions
 */
static int init(void)
{
    return DSCHED_SUCCESS;
}

static int finalize(void)
{
    return DSCHED_SUCCESS;
}
