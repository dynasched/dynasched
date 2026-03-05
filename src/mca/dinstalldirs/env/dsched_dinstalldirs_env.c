/*
 * Copyright (c) 2006-2007 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2007      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "src/include/dsched_config.h"

#include <stdlib.h>
#include <string.h>

#include "src/mca/dinstalldirs/dinstalldirs.h"

static void dinstalldirs_env_init(pmix_info_t info[], size_t ninfo);

dsched_dinstalldirs_base_component_t dsched_mca_dinstalldirs_env_component = {
    /* First, the mca_component_t struct containing meta information
       about the component itself */
    .component = {
        DSCHED_DINSTALLDIRS_BASE_VERSION_1_0_0,

        /* Component name and version */
        "env",
        DSCHED_MAJOR_VERSION,
        DSCHED_MINOR_VERSION,
        DSCHED_RELEASE_VERSION,
    },

    /* Next the dsched_dinstall_dirs_t install_dirs_data information */
    .install_dirs_data = {
        .prefix = NULL,
        .exec_prefix = NULL,
        .bindir = NULL,
        .sbindir = NULL,
        .libexecdir = NULL,
        .datarootdir = NULL,
        .datadir = NULL,
        .sysconfdir = NULL,
        .sharedstatedir = NULL,
        .localstatedir = NULL,
        .libdir = NULL,
        .includedir = NULL,
        .infodir = NULL,
        .mandir = NULL,
        .pmixdatadir = NULL,
        .pmixlibdir = NULL,
        .pmixincludedir = NULL
    },
    .init = dinstalldirs_env_init
};
PMIX_MCA_BASE_COMPONENT_INIT(dsched, dinstalldirs, env)

#define SET_FIELD(field, envname)                                     \
    do {                                                              \
        char *tmp = getenv(envname);                                  \
        if (NULL != tmp && 0 == strlen(tmp)) {                        \
            tmp = NULL;                                               \
        }                                                             \
        dsched_mca_dinstalldirs_env_component.install_dirs_data.field = tmp; \
    } while (0)

static void dinstalldirs_env_init(pmix_info_t info[], size_t ninfo)
{
    size_t n;
    bool prefix_given = false;

    /* check for a prefix value */
    for (n = 0; n < ninfo; n++) {
        if (PMIX_CHECK_KEY(&info[n], PMIX_PREFIX)) {
            dsched_mca_dinstalldirs_env_component.install_dirs_data.prefix = info[n].value.data.string;
            prefix_given = true;
            break;
        }
    }

    if (!prefix_given) {
        SET_FIELD(prefix, "DSCHED_PREFIX");
    }
    SET_FIELD(exec_prefix, "DSCHED_EXEC_PREFIX");
    SET_FIELD(bindir, "DSCHED_BINDIR");
    SET_FIELD(sbindir, "DSCHED_SBINDIR");
    SET_FIELD(libexecdir, "DSCHED_LIBEXECDIR");
    SET_FIELD(datarootdir, "DSCHED_DATAROOTDIR");
    SET_FIELD(datadir, "DSCHED_DATADIR");
    SET_FIELD(sysconfdir, "DSCHED_SYSCONFDIR");
    SET_FIELD(sharedstatedir, "DSCHED_SHAREDSTATEDIR");
    SET_FIELD(localstatedir, "DSCHED_LOCALSTATEDIR");
    SET_FIELD(libdir, "DSCHED_LIBDIR");
    SET_FIELD(includedir, "DSCHED_INCLUDEDIR");
    SET_FIELD(infodir, "DSCHED_INFODIR");
    SET_FIELD(mandir, "DSCHED_MANDIR");
    SET_FIELD(pmixdatadir, "DSCHED_PKGDATADIR");
    SET_FIELD(pmixlibdir, "DSCHED_PKGLIBDIR");
    SET_FIELD(pmixincludedir, "DSCHED_PKGINCLUDEDIR");
}
