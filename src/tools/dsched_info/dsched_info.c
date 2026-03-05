/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2007 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2010-2016 Los Alamos National Security, LLC.
 *                         All rights reserved.
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2015      Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#    include <netdb.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#    include <sys/param.h>
#endif
#include <errno.h>
#include <signal.h>

#include "src/class/pmix_object.h"
#include "src/class/pmix_pointer_array.h"
#include "src/mca/base/pmix_base.h"
#include "src/runtime/pmix_info_support.h"
#include "src/runtime/pmix_rte.h"
#include "src/util/pmix_argv.h"
#include "src/util/pmix_basename.h"
#include "src/util/pmix_cmd_line.h"
#include "src/util/pmix_path.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_constants.h"
#include "src/include/dsched_frameworks.h"
#include "src/include/dsched_internal_version.h"
#include "src/include/dsched_portable_platform_real.h"
#include "src/include/dsched_globals.h"
#include "src/mca/dinstalldirs/dinstalldirs.h"
#include "src/runtime/dsched_rte.h"
#include "src/util/dsched_cmd_line.h"
#include "src/util/dsched_error.h"

static int register_framework_params(pmix_pointer_array_t *component_map)
{
    int rc;

    /* Register mca/base parameters */
    if (PMIX_SUCCESS != pmix_mca_base_open(NULL)) {
        pmix_show_help("help-dsched-info.txt", "lib-call-fail", true, "pmix_mca_base_open",
                       __FILE__, __LINE__);
        return PMIX_ERROR;
    }

    /* Register the PMIX layer's MCA parameters */
    if (PMIX_SUCCESS != (rc = dsched_register_params())) {
        fprintf(stderr, "dsched_register_params failed\n");
        return rc;
    }

    return pmix_info_register_project_frameworks("dsched", dsched_frameworks, component_map);
}

/*
 * Public variables
 */

const char *dsched_info_type_all = "all";
const char *dsched_info_type_dsched = "dsched";
const char *dsched_info_type_base = "base";

// cmd line definition
static struct option dinfooptions[] = {
    PMIX_OPTION_SHORT_DEFINE(PMIX_CLI_HELP, PMIX_ARG_OPTIONAL, 'h'),
    PMIX_OPTION_SHORT_DEFINE(PMIX_CLI_VERSION, PMIX_ARG_NONE, 'V'),
    PMIX_OPTION_SHORT_DEFINE(PMIX_CLI_VERBOSE, PMIX_ARG_NONE, 'v'),

    // MCA parameters
    PMIX_OPTION_DEFINE(DSCHED_CLI_PMIXMCA, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_DMCA, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_MCA, PMIX_ARG_REQD),

    PMIX_OPTION_SHORT_DEFINE(PMIX_CLI_INFO_ALL, PMIX_ARG_NONE, 'a'),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_ARCH, PMIX_ARG_NONE),
    PMIX_OPTION_SHORT_DEFINE(PMIX_CLI_INFO_CONFIG, PMIX_ARG_NONE, 'c'),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_HOSTNAME, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_INTERNAL, PMIX_ARG_NONE),

    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_PARAM, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_PARAMS, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_PATH, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_VERSION, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(PMIX_CLI_PRETTY_PRINT, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(PMIX_CLI_PARSABLE, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(PMIX_CLI_PARSEABLE, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_SHOW_FAILED, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_SELECTED_ONLY, PMIX_ARG_NONE),
    PMIX_OPTION_SHORT_DEFINE(PMIX_CLI_INFO_TYPES, PMIX_ARG_REQD, 't'),
    PMIX_OPTION_DEFINE(PMIX_CLI_INFO_COLOR, PMIX_ARG_REQD),

    PMIX_OPTION_DEFINE("include-pmix", PMIX_ARG_NONE),

    /* End of list */
    PMIX_OPTION_END
};
static char *dinfoshorts = "h::vVact:";


int main(int argc, char *argv[])
{
    int ret = 0;
    pmix_status_t rc;
    bool acted = false;
    bool want_all = false;
    bool include_pmix = false;
    int i;
    char *color;
    char *str;
    char *ptr;
    pmix_pointer_array_t dsched_component_map;
    pmix_pointer_array_t mca_types;
    pmix_cli_result_t results;
    pmix_cli_item_t *opt;
    DSCHED_HIDE_UNUSED_PARAMS(argc);

    /* protect against problems if someone passes us thru a pipe
     * and then abnormally terminates the pipe early */
    signal(SIGPIPE, SIG_IGN);

    /* Initialize the argv parsing stuff */
    dsched_globals.basename = pmix_basename(argv[0]);
    if (DSCHED_SUCCESS != (ret = dsched_init_util())) {
        pmix_show_help("help-dsched-info.txt", "lib-call-fail", true, "dsched_init_util", __FILE__,
                       __LINE__, NULL);
        exit(ret);
    }

    /* parse the input argv to get values, including everyone's MCA params */
    PMIX_CONSTRUCT(&results, pmix_cli_result_t);
    rc = pmix_cmd_line_parse(argv, dinfoshorts, dinfooptions, NULL,
                             &results, "help-dsched-info.txt");
    if (PMIX_SUCCESS != rc) {
        PMIX_DESTRUCT(&results);
        if (PMIX_OPERATION_SUCCEEDED == rc) {
            return PMIX_SUCCESS;
        }
        if (PMIX_ERR_SILENT != rc) {
            fprintf(stderr, "%s: command line error (%s)\n", dsched_globals.basename, PMIx_Error_string(rc));
        }
        return rc;
    }
    // we do NOT accept arguments other than our own
    if (NULL != results.tail) {
        str = PMIx_Argv_join(results.tail, ' ');
        if (0 != strcmp(str, argv[0])) {
            ptr = pmix_show_help_string("help-dsched-info.txt", "no-args", false,
                                        dsched_globals.basename, str, dsched_globals.basename);
            free(str);
            if (NULL != ptr) {
                printf("%s", ptr);
                free(ptr);
            }
            return -1;
        }
        free(str);
    }

    // see if they want PMIx info included
    include_pmix = pmix_cmd_line_is_taken(&results, "include-pmix");

    /* Determine color support */
    opt = pmix_cmd_line_get_param(&results, PMIX_CLI_INFO_COLOR);
    if (NULL != opt) {
        color = NULL;
        if (NULL != opt->values) {
            color = opt->values[0];
        }
        if (NULL == color) {
            color = "auto";
        }
    } else {
        color = "auto";
    }
    if (0 == strcasecmp(color, "auto")) {
        #if HAVE_ISATTY
            pmix_info_color = isatty(STDOUT_FILENO);
        #else
            pmix_info_color = false;
        #endif
    } else if (0 == strcasecmp(color, "always")) {
        pmix_info_color = true;
    } else if (0 == strcasecmp(color, "never")) {
        pmix_info_color = false;
    } else {
        fprintf(stderr, "%s: Unrecognized value '%s' to color parameter\n", argv[0], color);
        exit(2);
    }


    /* set the flags */
    if (pmix_cmd_line_is_taken(&results, PMIX_CLI_PRETTY_PRINT)) {
        pmix_info_pretty = true;
    } else if (pmix_cmd_line_is_taken(&results, PMIX_CLI_PARSABLE) ||
               pmix_cmd_line_is_taken(&results, PMIX_CLI_PARSEABLE)) {
        pmix_info_pretty = false;
    }

    if (pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_SELECTED_ONLY)) {
        /* register only selected components */
        pmix_info_register_flags = PMIX_MCA_BASE_REGISTER_DEFAULT;
    }

    if (pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_SHOW_FAILED)) {
        pmix_mca_base_component_track_load_errors = true;
    }

    /* setup the mca_types array */
    PMIX_CONSTRUCT(&mca_types, pmix_pointer_array_t);
    pmix_pointer_array_init(&mca_types, 256, INT_MAX, 128);

    /* add a type for dsched itself */
    pmix_pointer_array_add(&mca_types, "mca");
    pmix_pointer_array_add(&mca_types, "dsched");

    /* push all the types found by autogen */
    for (i = 0; NULL != dsched_frameworks[i]; i++) {
        pmix_pointer_array_add(&mca_types, dsched_frameworks[i]->framework_name);
    }

    if (include_pmix) {
        // push all the PMIx framework types
        pmix_info_register_types(&mca_types, true);
    }

    /* setup the component_map array */
    PMIX_CONSTRUCT(&dsched_component_map, pmix_pointer_array_t);
    pmix_pointer_array_init(&dsched_component_map, 256, INT_MAX, 128);

    /* Register all global MCA Params */
    if (DSCHED_SUCCESS != (ret = dsched_register_params())) {
        if (DSCHED_ERR_SILENT != ret) {
            pmix_show_help("help-dsched-runtime", "dsched_init:startup:internal-failure", true,
                           "dsched register params",
                           DSCHED_ERROR_NAME(ret), ret);
        }
        return 1;
    }

     /* Register framework/component params */
    if (PMIX_SUCCESS != (ret = register_framework_params(&dsched_component_map))) {
        if (PMIX_ERR_BAD_PARAM == ret) {
            /* output what we got */
            pmix_info_do_params("DynaSched", true, pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_INTERNAL),
                                &mca_types, &dsched_component_map, NULL);
        }
        exit(1);
    }

    if (include_pmix) {
        /* register params for pmix */
        if (PMIX_SUCCESS != (ret = pmix_register_params())) {
            fprintf(stderr, "pmix_register_params failed with %d\n", ret);
            return PMIX_ERROR;
        }


        /* Register PMIx's params */
        if (PMIX_SUCCESS != (ret = pmix_info_register_framework_params(&dsched_component_map))) {
            if (PMIX_ERR_BAD_PARAM == ret) {
                /* output what we got */
                pmix_info_do_params("PMIx", true, pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_INTERNAL),
                                    &mca_types, &dsched_component_map, NULL);
            }
            exit(1);
        }
    }

    /* Execute the desired action(s) */
    want_all = pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_ALL);

    opt = pmix_cmd_line_get_param(&results, PMIX_CLI_INFO_VERSION);
    if (want_all || NULL != opt) {
        if (NULL == opt) {
            pmix_info_show_package(DSCHED_PACKAGE_STRING);
            pmix_info_show_version("DynaSched", pmix_info_ver_full, DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION,
                                   DSCHED_RELEASE_VERSION, DSCHED_GREEK_VERSION, DSCHED_REPO_REV,
                                   DSCHED_RELEASE_DATE);
            pmix_info_show_component_version("DynaSched", &mca_types, &dsched_component_map, pmix_info_type_all,
                                             pmix_info_component_all, pmix_info_ver_full,
                                             pmix_info_ver_all);

        } else if (NULL == opt->values) {
            pmix_info_show_package(DSCHED_PACKAGE_STRING);
            pmix_info_show_version("DynaSched", pmix_info_ver_full, DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION,
                                    DSCHED_RELEASE_VERSION, DSCHED_GREEK_VERSION, DSCHED_REPO_REV,
                                    DSCHED_RELEASE_DATE);
            pmix_info_show_component_version("DynaSched", &mca_types, &dsched_component_map, pmix_info_type_all,
                                             pmix_info_component_all, pmix_info_ver_full,
                                             pmix_info_ver_all);

        } else {
            if (0 == strcasecmp(opt->values[0], "dsched") ||
                0 == strcasecmp(opt->values[0], "all")) {
                pmix_info_show_package(DSCHED_PACKAGE_STRING);
                pmix_info_show_version("DynaSched", pmix_info_ver_full, DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION,
                                       DSCHED_RELEASE_VERSION, DSCHED_GREEK_VERSION, DSCHED_REPO_REV,
                                       DSCHED_RELEASE_DATE);
                pmix_info_show_component_version("DynaSched", &mca_types, &dsched_component_map, pmix_info_type_all,
                                                 pmix_info_component_all, pmix_info_ver_full,
                                                 pmix_info_ver_all);

            } else {
                // the first arg is either the name of a framework, or a framework:component pair
                char **tmp = PMIx_Argv_split(opt->values[0], ':');
                const char *component = pmix_info_component_all;
                const char *modifier = pmix_info_ver_full;
                if (NULL != tmp[1]) {
                    component = tmp[1];
                }
                if (NULL != opt->values[1]) {
                    modifier = opt->values[1];
                }
                pmix_info_show_component_version("DynaSched", &mca_types, &dsched_component_map, tmp[0],
                                                 component, modifier,
                                                 pmix_info_ver_all);
                PMIx_Argv_free(tmp);
            }
            acted = true;
        }
    }

    if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_PATH)) {
        pmix_info_do_path(want_all, &results, &dsched_dinstall_dirs);
        acted = true;
    }

    if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_ARCH)) {
        pmix_info_do_arch();
        acted = true;
    }

    if (!want_all && pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_HOSTNAME)) {
        // hostname is contain in do_config, so don't duplicate it here
        pmix_info_do_hostname();
        acted = true;
    }

    if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_CONFIG)) {
        pmix_info_do_config(true, DSCHED_CONFIGURE_USER, DSCHED_CONFIGURE_DATE, DSCHED_CONFIGURE_HOST,
                            DSCHED_CONFIGURE_CLI, DSCHED_BUILD_USER, DSCHED_BUILD_DATE, DSCHED_BUILD_HOST,
                            DSCHED_CC, DSCHED_CC_ABSOLUTE, PLATFORM_STRINGIFY(PLATFORM_COMPILER_FAMILYNAME),
                            PLATFORM_STRINGIFY(PLATFORM_COMPILER_VERSION_STR), DSCHED_BUILD_CFLAGS, DSCHED_BUILD_LDFLAGS,
                            DSCHED_BUILD_LIBS, DSCHED_ENABLE_DEBUG, DSCHED_HAVE_DDL_SUPPORT,
                            DSCHED_C_HAVE_VISIBILITY);
        acted = true;
    }

    if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_PARAM) ||
        pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_PARAMS)) {
        pmix_info_do_params("DynaSched", true, pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_INTERNAL),
                            &mca_types, &dsched_component_map, &results);
        acted = true;
    }

    if (pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_TYPES)) {
        pmix_info_do_type(&results);
        acted = true;
    }

    /* If no command line args are specified, show default set */

    if (!acted) {
        pmix_info_show_package(DSCHED_PACKAGE_STRING);
        pmix_info_show_version("DynaSched", pmix_info_ver_full, DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION,
                               DSCHED_RELEASE_VERSION, DSCHED_GREEK_VERSION, DSCHED_REPO_REV,
                               DSCHED_RELEASE_DATE);
        pmix_info_show_path(pmix_info_path_prefix, dsched_dinstall_dirs.prefix);
        pmix_info_do_arch();
        pmix_info_do_config(false, DSCHED_CONFIGURE_USER, DSCHED_CONFIGURE_DATE, DSCHED_CONFIGURE_HOST,
                            DSCHED_CONFIGURE_CLI, DSCHED_BUILD_USER, DSCHED_BUILD_DATE, DSCHED_BUILD_HOST,
                            DSCHED_CC, DSCHED_CC_ABSOLUTE, PLATFORM_STRINGIFY(PLATFORM_COMPILER_FAMILYNAME),
                            PLATFORM_STRINGIFY(PLATFORM_COMPILER_VERSION_STR), DSCHED_BUILD_CFLAGS, DSCHED_BUILD_LDFLAGS,
                            DSCHED_BUILD_LIBS, DSCHED_ENABLE_DEBUG, DSCHED_HAVE_DDL_SUPPORT,
                            DSCHED_C_HAVE_VISIBILITY);
        pmix_info_show_component_version("DynaSched", &mca_types, &dsched_component_map, pmix_info_type_all,
                                         pmix_info_component_all, pmix_info_ver_full,
                                         pmix_info_ver_all);
    }

    // if requested, do the PMIx info
    if (include_pmix) {
        acted = false;
        pmix_info_out(NULL, NULL, "\n\n\n==================");
        pmix_info_out(NULL, NULL, "\nPMIx CONFIGURATION\n");
        if (want_all || NULL != opt) {
            if (NULL == opt) {
                pmix_info_show_pmix_package();
                pmix_info_show_pmix_version();
                pmix_info_show_component_version("PMIx", &mca_types, &dsched_component_map, pmix_info_type_all,
                                                 pmix_info_component_all, pmix_info_ver_full,
                                                 pmix_info_ver_all);

            } else if (NULL == opt->values) {
                pmix_info_show_pmix_package();
                pmix_info_show_pmix_version();
                pmix_info_show_component_version("PMIx", &mca_types, &dsched_component_map, pmix_info_type_all,
                                                 pmix_info_component_all, pmix_info_ver_full,
                                                 pmix_info_ver_all);
                if (include_pmix) {
                    pmix_info_show_component_version("PMIx", &mca_types, &dsched_component_map, pmix_info_type_all,
                                                     pmix_info_component_all, pmix_info_ver_full,
                                                     pmix_info_ver_all);
                }

            } else {
                if (0 == strcasecmp(opt->values[0], "pmix") ||
                    0 == strcasecmp(opt->values[0], "all")) {
                    pmix_info_show_pmix_package();
                    pmix_info_show_pmix_version();
                    pmix_info_show_component_version("PMIx", &mca_types, &dsched_component_map, pmix_info_type_all,
                                                     pmix_info_component_all, pmix_info_ver_full,
                                                     pmix_info_ver_all);

                } else {
                    // the first arg is either the name of a framework, or a framework:component pair
                    char **tmp = PMIx_Argv_split(opt->values[0], ':');
                    const char *component = pmix_info_component_all;
                    const char *modifier = pmix_info_ver_full;
                    if (NULL != tmp[1]) {
                        component = tmp[1];
                    }
                    if (NULL != opt->values[1]) {
                        modifier = opt->values[1];
                    }
                    pmix_info_show_component_version("PMIx", &mca_types, &dsched_component_map, tmp[0],
                                                     component, modifier,
                                                     pmix_info_ver_all);

                    PMIx_Argv_free(tmp);
                }
                acted = true;
            }
        }

        if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_PATH)) {
            pmix_info_do_pmix_path(want_all, &results);
            acted = true;
        }

        if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_ARCH)) {
            pmix_info_do_arch();
            acted = true;
        }

        if (!want_all && pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_HOSTNAME)) {
            // hostname is contain in do_config, so don't duplicate it here
            pmix_info_do_hostname();
            acted = true;
        }

        if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_CONFIG)) {
            pmix_info_do_pmix_config(want_all);
            acted = true;
        }

        if (want_all || pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_PARAM) ||
            pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_PARAMS)) {
             pmix_info_do_params("PMIx", true, pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_INTERNAL),
                                &mca_types, &dsched_component_map, &results);
            acted = true;
        }

        if (pmix_cmd_line_is_taken(&results, PMIX_CLI_INFO_TYPES)) {
            pmix_info_do_type(&results);
            acted = true;
        }

        /* If no command line args are specified, show default set */

    if (!acted) {
            pmix_info_show_pmix_package();
            pmix_info_show_pmix_version();
            pmix_info_show_path(pmix_info_path_prefix, pmix_pinstall_dirs.prefix);
            pmix_info_do_arch();
            pmix_info_do_pmix_config(want_all);
            pmix_info_show_component_version("PMIx", &mca_types, &dsched_component_map, pmix_info_type_all,
                                             pmix_info_component_all, pmix_info_ver_full,
                                             pmix_info_ver_all);
        }
    }
    /* All done */
    PMIX_DESTRUCT(&mca_types);
    pmix_mca_base_close();

    return 0;
}
