/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2008 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2009-2010 Oracle and/or its affiliates.  All rights reserved.
 * Copyright (c) 2012-2013 Los Alamos National Security, LLC.
 *                         All rights reserved
 * Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2014-2018 Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * Copyright (c) 2017      IBM Corporation.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"
#include "include/dsched_constants.h"
#include "include/dsched_types.h"

#ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#endif
#include <signal.h>
#include <stdio.h>

#include "src/mca/base/pmix_mca_base_var.h"
#include "src/mca/dinstalldirs/dinstalldirs.h"
#include "src/util/pmix_argv.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_path.h"
#include "src/util/pmix_printf.h"
#include "src/util/pmix_environ.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_globals.h"
#include "src/runtime/dsched_rte.h"

static bool passed_thru = false;

int dsched_register_params(void)
{
    pmix_output_stream_t lds;
    char *home;

    /* only go thru this once - mpirun calls it twice, which causes
     * any error messages to show up twice
     */
    if (passed_thru) {
        return DSCHED_SUCCESS;
    }
    passed_thru = true;

    /* get a clean output channel too - need to do this here because
     * we use it below, and prun and some other tools call this
     * function prior to calling dsched_init
     */
    PMIX_CONSTRUCT(&lds, pmix_output_stream_t);
    lds.lds_want_stdout = true;
    dsched_globals.clean_output = pmix_output_open(&lds);
    PMIX_DESTRUCT(&lds);


    /* LOOK FOR A TMP DIRECTORY BASE */
    dsched_globals.tmpdir = NULL;
    (void)
        pmix_mca_base_var_register("dsched", "dsched", NULL, "tmpdir",
                                   "Directory to be used for storing scheduler rendezvous file",
                                   PMIX_MCA_BASE_VAR_TYPE_STRING,
                                   &dsched_globals.tmpdir);

    home = (char *) pmix_home_directory(geteuid());
    pmix_asprintf(&dsched_globals.param_files,
                   "%s" PMIX_PATH_SEP ".dsched" PMIX_PATH_SEP "mca-params.conf%c%s" PMIX_PATH_SEP
                   "dsched-mca-params.conf",
                   home, ',', dsched_dinstall_dirs.sysconfdir);

   (void) pmix_mca_base_var_register("dsched", "dsched", NULL, "param_files",
                                      "Path for MCA configuration files containing "
                                      "variable values",
                                      PMIX_MCA_BASE_VAR_TYPE_STRING,
                                      &dsched_globals.param_files);

    (void) pmix_mca_base_var_register("dsched", "dsched", NULL, "override_param_file",
                                      "Variables set in this file will override any value "
                                      "set in the environment or another configuration file",
                                      PMIX_MCA_BASE_VAR_TYPE_STRING,
                                      &dsched_globals.override_param_file);

    (void) pmix_mca_base_var_register("dsched", "dsched", NULL, "suppress_override_warning",
                                      "Suppress warnings when attempting to set an "
                                      "overridden value (default: false)",
                                      PMIX_MCA_BASE_VAR_TYPE_BOOL,
                                      &dsched_globals.suppress_override_warning);


    return DSCHED_SUCCESS;
}
