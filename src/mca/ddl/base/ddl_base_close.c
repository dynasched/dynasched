/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2015      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "src/include/dsched_config.h"

#include "src/mca/base/pmix_base.h"
#include "src/mca/mca.h"

#include "src/mca/ddl/base/base.h"
#include "src/mca/ddl/ddl.h"

int dsched_ddl_base_close(void)
{
    /* Close all available modules that are open */
    return pmix_mca_base_framework_components_close(&dsched_ddl_base_framework, NULL);
}
