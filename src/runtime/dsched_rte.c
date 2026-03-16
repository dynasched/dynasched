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
#include "src/util/pmix_if.h"
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
    int ret;
    char *path = NULL, *ptr;
    const char *rvers;
    char token[100];
    char *error;
    unsigned int major, minor, release;
    unsigned int reqmajor, reqminor, reqrelease;
    unsigned int maxmajor, maxminor, maxrelease;
    char hostname[DSCHED_MAXHOSTNAMELEN];
    char **prefixes;
    bool match;
    int i, idx;

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

    /* initialize the output system */
    pmix_output_init();

    /* set the nodename so anyone who needs it has it */
    gethostname(hostname, DSCHED_MAXHOSTNAMELEN);
    /* add network aliases to our list of alias hostnames */
    pmix_ifgetaliases(&dsched_globals.aliases);
    /* we have to strip node names here, if user directs, to ensure that
     * the names returned by the DVM match ours
     */
    if (NULL != dsched_globals.strip_prefixes && !pmix_net_isaddr(hostname)) {
        prefixes = PMIx_Argv_split(dsched_globals.strip_prefixes, ',');
        match = false;
        for (i = 0; NULL != prefixes[i]; i++) {
            if (0 == strncmp(hostname, prefixes[i], strlen(prefixes[i]))) {
                /* remove the prefix and leading zeroes */
                idx = strlen(prefixes[i]);
                while (idx < (int) strlen(hostname)
                       && (hostname[idx] <= '0' || '9' < hostname[idx])) {
                    idx++;
                }
                if ((int) strlen(hostname) <= idx) {
                    /* there were no non-zero numbers in the name */
                    dsched_globals.hostname = strdup(&hostname[strlen(prefixes[i])]);
                } else {
                    dsched_globals.hostname = strdup(&hostname[idx]);
                }
                /* add this to our list of aliases */
                PMIx_Argv_append_unique_nosize(&dsched_globals.aliases, dsched_globals.hostname);
                match = true;
                break;
            }
        }
        /* if we didn't find a match, then just use the hostname as-is */
        if (!match) {
            dsched_globals.hostname = strdup(hostname);
        }
        PMIx_Argv_free(prefixes);
    } else {
        dsched_globals.hostname = strdup(hostname);
    }

    // if we are not keeping FQDN, then strip it off if not an IP address
    if (!pmix_net_isaddr(dsched_globals.hostname) &&
        NULL != (ptr = strchr(dsched_globals.hostname, '.'))) {
        if (dsched_globals.keep_fqdn_hostnames) {
            /* retain the non-fqdn name as an alias */
            *ptr = '\0';
            PMIx_Argv_append_unique_nosize(&dsched_globals.aliases, dsched_globals.hostname);
            *ptr = '.';
        } else {
            /* add the fqdn name as an alias */
            PMIx_Argv_append_unique_nosize(&dsched_globals.aliases, dsched_globals.hostname);
            /* retain the non-fqdn name as the node's name */
            *ptr = '\0';
        }
    }

    /*
     * Initialize the event library
     */
    if (DSCHED_SUCCESS != (ret = dsched_event_base_open())) {
        error = "dsched_event_base_open";
        goto error;
    }
    dsched_globals.evactive = true;

    return DSCHED_SUCCESS;

error:
    if (DSCHED_ERR_SILENT != ret) {
        pmix_show_help("help-dsched-runtime", "dsched_init:startup:internal-failure", true, error,
                       DSCHED_ERROR_NAME(ret), ret);
    }

    return ret;
}
