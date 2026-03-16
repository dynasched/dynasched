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
#include "src/mca/base/pmix_mca_base_var.h"
#include "src/mca/base/pmix_mca_base_vari.h"
#include "src/mca/pmdl/base/base.h"
#include "src/util/pmix_argv.h"
#include "src/util/pmix_cmd_line.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_os_path.h"
#include "src/util/pmix_path.h"
#include "src/util/pmix_printf.h"
#include "src/util/pmix_environ.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_globals.h"
#include "src/runtime/dsched_rte.h"
#include "src/util/dsched_cmd_line.h"

static bool passed_thru = false;
static int gen_verbose = -1;

int dsched_register_params(void)
{
    pmix_output_stream_t lds;
    char *home;
    bool opened = false;

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

    (void) pmix_mca_base_var_register("dsched", "dsched", NULL, "verbose",
                                      "Debug verbosity for DynaSched",
                                      PMIX_MCA_BASE_VAR_TYPE_INT,
                                      &dsched_globals.verbosity);
    if (0 < dsched_globals.verbosity) {
        dsched_globals.output = pmix_output_open(NULL);
        pmix_output_set_verbosity(dsched_globals.output,
                                  dsched_globals.verbosity);
        dsched_globals.pmix_output = pmix_output_open(NULL);
        pmix_output_set_verbosity(dsched_globals.pmix_output,
                                  dsched_globals.verbosity);
        opened = true;
    }

    /* register the backend verbosity */
    (void) pmix_mca_base_var_register("dsched", "pmix", NULL, "server_verbose",
                                      "Debug verbosity for backend PMIx server support",
                                      PMIX_MCA_BASE_VAR_TYPE_INT,
                                      &gen_verbose);
    if (0 < gen_verbose && dsched_globals.verbosity < gen_verbose) {
        if (!opened) {
            dsched_globals.pmix_output = pmix_output_open(NULL);
        }
        pmix_output_set_verbosity(dsched_globals.pmix_output, gen_verbose);
    }

    (void) pmix_mca_base_var_register("dsched", "dsched", NULL, "progress_thread_cpus",
                                      "Comma-delimited list of ranges of CPUs to which"
                                      "the internal DynaSched progress thread(s) are to be bound",
                                      PMIX_MCA_BASE_VAR_TYPE_STRING,
                                      &dsched_globals.progress_thread_cpus);

    (void) pmix_mca_base_var_register("dsched", "dsched", NULL, "bind_progress_thread_reqd",
                                      "Whether binding of internal DynaSched progress threads is required",
                                      PMIX_MCA_BASE_VAR_TYPE_BOOL,
                                      &dsched_globals.bind_progress_thread_reqd);

    (void) pmix_mca_base_var_register("dsched", "dsched", NULL, "keep_fqdn_hostnames",
                                      "Whether or not to keep FQDN hostnames [default: no]",
                                      PMIX_MCA_BASE_VAR_TYPE_BOOL,
                                      &dsched_globals.keep_fqdn_hostnames);

    (void) pmix_mca_base_var_register(
        "dsched", "dsched", NULL, "strip_prefix",
        "Prefix(es) to match when deciding whether to strip leading characters and zeroes from "
        "node names returned by daemons",
        PMIX_MCA_BASE_VAR_TYPE_STRING, &dsched_globals.strip_prefixes);


    return DSCHED_SUCCESS;
}

int dsched_preload_default_mca_params(void)
{
    char *file, *home, *tmp, **paths = NULL;
    pmix_list_t params, params2, pfinal;
    pmix_mca_base_var_file_value_t *fv, *fv2, *fvnext, *fvnext2;
    bool match;
    int i, rc;

    home = (char*)pmix_home_directory(-1);
    PMIX_CONSTRUCT(&params, pmix_list_t);
    PMIX_CONSTRUCT(&params2, pmix_list_t);
    PMIX_CONSTRUCT(&pfinal, pmix_list_t);

    if (NULL == dsched_globals.param_files) {
        /* start with the system-level defaults */
        file = pmix_os_path(false, dsched_dinstall_dirs.sysconfdir, "dsched-mca-params.conf", NULL);
        rc = pmix_mca_base_parse_paramfile(file, &params);
        free(file);
        if (PMIX_SUCCESS != rc && PMIX_ERR_NOT_FOUND != rc) {
            // it is okay if the file isn't found
            return rc;
        }
        /* now get the user-level defaults */
        file = pmix_os_path(false, home, ".dsched", "mca-params.conf", NULL);
        rc = pmix_mca_base_parse_paramfile(file, &params2);
        free(file);
        if (PMIX_SUCCESS != rc && PMIX_ERR_NOT_FOUND != rc) {
            // it is okay if the file isn't found
            return rc;
        }
    } else {
        // split the string on commas
        paths = PMIx_Argv_split(dsched_globals.param_files, ',');
        // process each path
        for (i=0; NULL != paths[i]; i++) {
            rc = pmix_mca_base_parse_paramfile(paths[i], &params);
            if (PMIX_SUCCESS != rc && PMIX_ERR_NOT_FOUND != rc) {
                // it is okay if the file isn't found
                PMIx_Argv_free(paths);
                return rc;
            }
        }
        PMIx_Argv_free(paths);
    }

    /* cross-check the lists, keeping the params2 entries over any
     * matching params entries as they overwrite the system ones */
    PMIX_LIST_FOREACH_SAFE(fv, fvnext, &params, pmix_mca_base_var_file_value_t) {
        match = false;
        PMIX_LIST_FOREACH_SAFE(fv2, fvnext2, &params2, pmix_mca_base_var_file_value_t) {
            /* do we have a match? */
            if (0 == strcmp(fv->mbvfv_var, fv2->mbvfv_var)) {
                /* transfer the user-level default to the final list */
                pmix_list_remove_item(&params2, &fv2->super);
                pmix_list_append(&pfinal, &fv2->super);
                /* remove and release the system-level duplicate */
                pmix_list_remove_item(&params, &fv->super);
                PMIX_RELEASE(fv);
                match = true;
                break;
            }
        }
        if (!match) {
            /* transfer the system-level default to the final list */
            pmix_list_remove_item(&params, &fv->super);
            pmix_list_append(&pfinal, &fv->super);
        }
    }
    /* transfer any remaining use-level defaults to the final list
     * as they had no matches */
    while (NULL != (fv2 = (pmix_mca_base_var_file_value_t*)pmix_list_remove_first(&params2))) {
        pmix_list_append(&pfinal, &fv2->super);
    }
    PMIX_LIST_DESTRUCT(&params);
    PMIX_LIST_DESTRUCT(&params2);

    // process any override params
    PMIX_CONSTRUCT(&params, pmix_list_t);
    if (NULL != dsched_globals.override_param_file) {
        // process the file
        rc = pmix_mca_base_parse_paramfile(dsched_globals.override_param_file, &params);
        if (PMIX_SUCCESS != rc && PMIX_ERR_NOT_FOUND != rc) {
            // it is okay if the file isn't found
            PMIX_LIST_DESTRUCT(&params);
            PMIX_LIST_DESTRUCT(&pfinal);
            return rc;
        }
        if (0 < pmix_list_get_size(&params)) {
            // check the params against any given
            PMIX_LIST_FOREACH(fv, &pfinal, pmix_mca_base_var_file_value_t) {
                PMIX_LIST_FOREACH(fv2, &params, pmix_mca_base_var_file_value_t) {
                    if (0 == strcmp(fv->mbvfv_var, fv2->mbvfv_var)) {
                        if (!dsched_globals.suppress_override_warning) {
                            pmix_show_help("help-pmix-mca-var.txt", "overridden-param-set", true, fv->mbvfv_var);
                        }
                        free(fv->mbvfv_var);
                        fv->mbvfv_var = strdup(fv2->mbvfv_var);
                        free(fv->mbvfv_value);
                        fv->mbvfv_value = strdup(fv2->mbvfv_value);
                        break;
                    }
                }
            }
            // now overwrite any envars
            PMIX_LIST_FOREACH(fv, &params, pmix_mca_base_var_file_value_t) {
                if (pmix_pmdl_base_check_pmix_param(fv->mbvfv_var)) {
                    pmix_asprintf(&tmp, "PMIX_MCA_%s", fv->mbvfv_var);
                    if (!dsched_globals.suppress_override_warning && NULL != getenv(tmp)) {
                        pmix_show_help("help-pmix-mca-var.txt", "overridden-param-set", true, tmp);
                    }
                    // set it, and overwrite if they already
                    // have a value in our environment
                    setenv(tmp, fv->mbvfv_value, true);
                    free(tmp);
                } else {
                    pmix_asprintf(&tmp, "DSCHED_MCA_%s", fv->mbvfv_var);
                    if (!dsched_globals.suppress_override_warning && NULL != getenv(tmp)) {
                        pmix_show_help("help-pmix-mca-var.txt", "overridden-param-set", true, tmp);
                    }
                    // set it, and overwrite if they already
                    // have a value in our environment
                    setenv(tmp, fv->mbvfv_value, true);
                    free(tmp);
                }
            }
        }
        PMIX_LIST_DESTRUCT(&params);
    }

    /* now process the final list - but do not overwrite if the
     * user already has the param in our environment as their
     * environment settings override all defaults */
    PMIX_LIST_FOREACH(fv, &pfinal, pmix_mca_base_var_file_value_t) {
        if (pmix_pmdl_base_check_pmix_param(fv->mbvfv_var)) {
            pmix_asprintf(&tmp, "PMIX_MCA_%s", fv->mbvfv_var);
            // set it, but don't overwrite if they already
            // have a value in our environment
            setenv(tmp, fv->mbvfv_value, false);
            free(tmp);
        } else {
            pmix_asprintf(&tmp, "DSCHED_MCA_%s", fv->mbvfv_var);
            // set it, but don't overwrite if they already
            // have a value in our environment
            setenv(tmp, fv->mbvfv_value, false);
            free(tmp);
        }
    }

    PMIX_LIST_DESTRUCT(&pfinal);
    return DSCHED_SUCCESS;
}

int dsched_process_cli_mca_params(pmix_cli_result_t *cli)
{
    pmix_cli_item_t *opt;
    int n;
    char *tmp, *ptr;

    opt = pmix_cmd_line_get_param(cli, DSCHED_CLI_PMIXMCA);
    if (NULL != opt) {
        for (n=0; NULL != opt->values[n]; n++) {
            // entry is param=value
            ptr = strchr(opt->values[n], '=');
            *ptr = '\0';
            ++ptr;
            pmix_asprintf(&tmp, "PMIX_MCA_%s", opt->values[n]);
            // overwrite anything in the environ as cmd line is final
            setenv(tmp, ptr, true);
            free(tmp);
        }
    }

    opt = pmix_cmd_line_get_param(cli, DSCHED_CLI_MCA);
    if (NULL != opt) {
        for (n=0; NULL != opt->values[n]; n++) {
            // entry is param=value
            ptr = strchr(opt->values[n], '=');
            *ptr = '\0';
            ++ptr;
            pmix_asprintf(&tmp, "DSCHED_MCA_%s", opt->values[n]);
            // overwrite anything in the environ as cmd line is final
            setenv(tmp, ptr, true);
            free(tmp);
        }
    }

    opt = pmix_cmd_line_get_param(cli, DSCHED_CLI_DMCA);
    if (NULL != opt) {
        for (n=0; NULL != opt->values[n]; n++) {
            // entry is param=value
            ptr = strchr(opt->values[n], '=');
            *ptr = '\0';
            ++ptr;
            pmix_asprintf(&tmp, "DSCHED_MCA_%s", opt->values[n]);
            // overwrite anything in the environ as cmd line is final
            setenv(tmp, ptr, true);
            free(tmp);
        }
    }
    return DSCHED_SUCCESS;
}
