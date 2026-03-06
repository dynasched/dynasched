/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2006-2018 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2007-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2007-2008 Sun Microsystems, Inc.  All rights reserved.
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2014-2016 Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 *
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/** @file **/

#include "dsched_config.h"
#include "src/include/dsched_constants.h"

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#    include <sys/stat.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "src/util/pmix_if.h"
#include "src/util/pmix_net.h"
#include "src/util/pmix_environ.h"
#include "src/util/pmix_os_path.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_printf.h"
#include "src/util/pmix_show_help.h"
#include "src/runtime/pmix_init_util.h"
#include "src/threads/pmix_threads.h"

#include "src/mca/base/pmix_base.h"
#include "src/mca/base/pmix_mca_base_var.h"
#include "src/mca/base/pmix_mca_base_vari.h"
#include "src/mca/pmdl/base/base.h"

#include "src/include/dsched_frameworks.h"
#include "src/include/dsched_globals.h"
#include "src/mca/dinstalldirs/base/base.h"
#include "src/util/dsched_error.h"

#include "src/runtime/dsched_rte.h"


const char dsched_version_string[] = DSCHED_IDENT_STRING;
static int preload_default_mca_params(void);

static bool check_exist(char *path)
{
    struct stat buf;
    /* coverity[TOCTOU] */
    if (0 == stat(path, &buf)) { /* exists */
        return true;
    }
    return false;
}

static void print_error(unsigned major, unsigned minor, unsigned release,
                        unsigned reqmajor, unsigned reqminor, unsigned reqrelease,
                        char *minmax, char *limit)
{
    fprintf(stderr, "************************************************\n");
    fprintf(stderr, "We have detected that the runtime version\n");
    fprintf(stderr, "of the PMIx library we were given violates\n");
    fprintf(stderr, "the %s version requirement:\n\n", minmax);
    fprintf(stderr, "    Runtime:  0x%x%02x%02x\n", major, minor, release);
    fprintf(stderr, "    %s:  0x%x%02x%02x\n\n", limit, reqmajor, reqminor, reqrelease);
    fprintf(stderr, "Please update your LD_LIBRARY_PATH to point\n");
    fprintf(stderr, "us to the same PMIx version used to build DynaSched.\n");
    fprintf(stderr, "************************************************\n");
}

static bool check_range(unsigned major, unsigned minor, unsigned release,
                        unsigned limmajor, unsigned limminor, unsigned limrelease)
{
    if (major > limmajor) {
        // we are good - nothing more to check
        return true;
    } else if (major < limmajor) {
        return false;
    }
    // checking within the major release series
    if (minor > limminor) {
        // we are good - nothing more to check
        return true;
    } else if (minor < limminor) {
        return false;
    }
    // checking within the minor release series
    if (release < limrelease) {
        return false;
    }
    return true;
}

static char *make_version_string(int major, int minor, int release,
                                 const char *greek, const char *repo)
{
    char *str = NULL, *tmp;
    char temp[BUFSIZ];

    temp[BUFSIZ - 1] = '\0';
    snprintf(temp, BUFSIZ - 1, "%d.%d", major, minor);
    str = strdup(temp);
    if (release >= 0) {
        snprintf(temp, BUFSIZ - 1, ".%d", release);
        pmix_asprintf(&tmp, "%s%s", str, temp);
        free(str);
        str = tmp;
    }
    if (NULL != greek) {
        pmix_asprintf(&tmp, "%s%s", str, greek);
        free(str);
        str = tmp;
    }
    if (NULL != repo) {
        pmix_asprintf(&tmp, "%s%s", str, repo);
        free(str);
        str = tmp;
    }

    if (NULL == str) {
        str = strdup(temp);
    }

    return str;
}

static bool util_initialized = false;

int dsched_init_util(void)
{
    int ret, n;
    char *path = NULL;
    char *evar, **prefixes;
    const char *rvers;
    char token[100];
    char *error;
    unsigned int major, minor, release;
    unsigned int reqmajor, reqminor, reqrelease;
    unsigned int maxmajor, maxminor, maxrelease;

    if (util_initialized) {
        return DSCHED_SUCCESS;
    }
    util_initialized = true;

    /* check to see if the version of PMIx we were given in the
     * library path matches the version we were built against.
     * Because we are using PMIx internals, we cannot support
     * cross version operations from inside of PRRTE.
     */
    rvers = PMIx_Get_version();
    ret = sscanf(rvers, "%s %u.%u.%u",
                 token, &major, &minor, &release);
    ret = sscanf(DSCHED_PMIX_MIN_VERSION_STRING, "%u.%u.%u",
                 &reqmajor, &reqminor, &reqrelease);
    ret = sscanf(DSCHED_PMIX_MAX_VERSION_STRING, "%u.%u.%u",
                 &maxmajor, &maxminor, &maxrelease);

    /* check the version triplet agains the min values
     * specified in VERSION
     */
    if (!check_range(major, minor, release,
                     reqmajor, reqminor, reqrelease)) {
        print_error(major, minor, release,
                    reqmajor, reqminor, reqrelease,
                    "minimum", "Minimum");
        return DSCHED_ERR_SILENT;
    }

    /* check the version triplet agains the max values
     * specified in VERSION
     */
    if (check_range(major, minor, release,
                    maxmajor, maxminor, maxrelease)) {
        print_error(major, minor, release,
                    maxmajor, maxminor, maxrelease,
                    "maximum", "Maximum");
        return DSCHED_ERR_SILENT;
    }

    // setup names
    pmix_tool_msg = "Report bugs to: https://github.com/dynasched/dynasched";
    pmix_tool_org = "DynaSched";
    pmix_tool_version = make_version_string(DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION,
                                            DSCHED_RELEASE_VERSION, DSCHED_GREEK_VERSION, NULL);
    /* carry across the toolname */
    pmix_tool_basename = dsched_globals.basename;

    // publish MCA prefixes
    prefixes = NULL;
    for (n=0; NULL != dsched_framework_names[n]; n++) {
        if (0 == strcmp("common", dsched_framework_names[n])) {
            continue;
        }
        PMIx_Argv_append_nosize(&prefixes, dsched_framework_names[n]);
    }
    evar = PMIx_Argv_join(prefixes, ',');
    pmix_setenv("DSCHED_MCA_PREFIXES", evar, true, &environ);
    free(evar);
    PMIx_Argv_free(prefixes);

    /* initialize install dirs code */
    ret = pmix_mca_base_framework_open(&dsched_dinstalldirs_base_framework,
                                       PMIX_MCA_BASE_OPEN_DEFAULT);
    if (DSCHED_SUCCESS != ret) {
        fprintf(stderr,
                "dsched_dinstalldirs_base_open() failed -- process will likely abort (%s:%d, "
                "returned %d instead of DSCHED_SUCCESS)\n",
                __FILE__, __LINE__, ret);
        return ret;
    }

    /* initialize the MCA infrastructure */
    if (check_exist(dsched_dinstall_dirs.pmixlibdir)) {
        pmix_asprintf(&path, "dsched@%s", dsched_dinstall_dirs.pmixlibdir);
    }
    ret = pmix_init_util(NULL, 0, path);
    if (NULL != path) {
        free(path);
    }
    if (PMIX_SUCCESS != ret) {
        return ret;
    }
    pmix_show_help_add_data("dsched", dsched_show_help_data);

    /* Setup the parameter system */
    if (DSCHED_SUCCESS != (ret = pmix_mca_base_var_init())) {
        return ret;
    }

    /* Register all global MCA Params */
    if (DSCHED_SUCCESS != (ret = dsched_register_params())) {
        if (DSCHED_ERR_SILENT != ret) {
            pmix_show_help("help-dsched-runtime", "dsched_init:startup:internal-failure", true,
                           "dsched register params",
                           DSCHED_ERROR_NAME(ret), ret);
        }
        return 1;
    }

    /* pre-load any default mca param files */
    preload_default_mca_params();

    /* initialize the output system */
    pmix_output_init();

    /* set the nodename so anyone who needs it has it */
    gethostname(dsched_globals.hostname, DSCHED_MAXHOSTNAMELEN);

    /*
     * Initialize the event library
     */
    if (DSCHED_SUCCESS != (ret = dsched_event_base_open())) {
        error = "dsched_event_base_open";
        goto error;
    }

    return DSCHED_SUCCESS;

error:
    if (DSCHED_ERR_SILENT != ret) {
        pmix_show_help("help-dsched-runtime", "dsched_init:startup:internal-failure", true, error,
                       DSCHED_ERROR_NAME(ret), ret);
    }

    return ret;
}

int dsched_init(pmix_info_t *info, size_t ninfo)
{
    int ret;
    char *error;
    DSCHED_HIDE_UNUSED_PARAMS(info, ninfo);

    if (!util_initialized) {
        dsched_init_util();
    }

    /* setup the global session and node arrays */
    PMIX_CONSTRUCT(&dsched_globals.nodes, pmix_pointer_array_t);
    ret = pmix_pointer_array_init(&dsched_globals.nodes, DSCHED_GLOBAL_ARRAY_BLOCK_SIZE,
                                  DSCHED_GLOBAL_ARRAY_MAX_SIZE,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE);
    if (PMIX_SUCCESS != ret) {
        PMIX_ERROR_LOG(ret);
        error = "setup node array";
        goto error;
    }
    PMIX_CONSTRUCT(&dsched_globals.sessions, pmix_pointer_array_t);
    ret = pmix_pointer_array_init(&dsched_globals.sessions,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE,
                                  DSCHED_GLOBAL_ARRAY_MAX_SIZE,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE);
    if (PMIX_SUCCESS != ret) {
        PMIX_ERROR_LOG(ret);
        error = "setup session array";
        goto error;
    }

    // setup an array for node topologies
    PMIX_CONSTRUCT(&dsched_globals.topologies, pmix_pointer_array_t);
    ret = pmix_pointer_array_init(&dsched_globals.topologies, DSCHED_GLOBAL_ARRAY_BLOCK_SIZE,
                                  DSCHED_GLOBAL_ARRAY_MAX_SIZE,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE);
    if (PMIX_SUCCESS != ret) {
        PMIX_ERROR_LOG(ret);
        error = "setup node topologies array";
        goto error;
    }

    /* initialize the cache */
    PMIX_CONSTRUCT(&dsched_globals.cache, pmix_pointer_array_t);
    pmix_pointer_array_init(&dsched_globals.cache, 1, INT_MAX, 1);

    return DSCHED_SUCCESS;

error:
    if (DSCHED_ERR_SILENT != ret) {
        pmix_show_help("help-dsched-runtime", "dsched_init:startup:internal-failure", true, error,
                       DSCHED_ERROR_NAME(ret), ret);
    }

    return ret;
}

static bool check_pmix_overlap(char *var, char *value, bool overwrite)
{
    char *tmp;

    if (0 == strncmp(var, "dl_", 3)) {
        pmix_asprintf(&tmp, "PMIX_MCA_pdl_%s", &var[3]);
        setenv(tmp, value, overwrite);
        free(tmp);
        return true;
    } else if (0 == strncmp(var, "oob_", 4) &&
               NULL == strstr(var, "verbose")) {
        pmix_asprintf(&tmp, "PMIX_MCA_ptl_%s", &var[4]);
        setenv(tmp, value, overwrite);
        free(tmp);
        return true;
    } else if (0 == strncmp(var, "hwloc_", 6)) {
        pmix_asprintf(&tmp, "PMIX_MCA_%s", var);
        setenv(tmp, value, overwrite);
        free(tmp);
        return true;
    } else if (0 == strncmp(var, "if_", 3)) {
        // need to convert if to pif
        pmix_asprintf(&tmp, "PMIX_MCA_pif_%s", &var[3]);
        setenv(tmp, value, overwrite);
        free(tmp);
        return true;
    } else if (0 == strncmp(var, "mca_", 4)) {
        pmix_asprintf(&tmp, "PMIX_MCA_%s", var);
        setenv(tmp, value, overwrite);
        free(tmp);
        return true;
    }
    return false;
}

static int preload_default_mca_params(void)
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
                    // if this relates to the DL, OOB, HWLOC, or IF,
                    // or mca frameworks, then we also need to set
                    // the equivalent PMIx value
                    if (check_pmix_overlap(fv->mbvfv_var, fv->mbvfv_value, true) &&
                        !dsched_globals.suppress_override_warning) {
                        pmix_show_help("help-pmix-mca-var.txt", "overridden-param-set", true, fv->mbvfv_var);
                    }
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
            // if this relates to the DL, OOB, HWLOC, or IF,
            // or mca frameworks, then we also need to set
            // the equivalent PMIx value
            check_pmix_overlap(fv->mbvfv_var, fv->mbvfv_value, false);
        }
    }

    PMIX_LIST_DESTRUCT(&pfinal);
    return DSCHED_SUCCESS;
}
