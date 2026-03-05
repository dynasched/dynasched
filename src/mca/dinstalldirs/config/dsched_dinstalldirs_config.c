/*
 * Copyright (c) 2006-2007 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "src/include/dsched_config.h"

#include "src/mca/dinstalldirs/config/dinstall_dirs.h"
#include "src/mca/dinstalldirs/dinstalldirs.h"

const dsched_dinstalldirs_base_component_t dsched_mca_dinstalldirs_config_component = {
    /* First, the mca_component_t struct containing meta information
       about the component itself */
    .component = {
        DSCHED_DINSTALLDIRS_BASE_VERSION_1_0_0,

        /* Component name and version */
        "config", DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION, DSCHED_RELEASE_VERSION,

        /* Component open and close functions */
        NULL, NULL
    },

    .install_dirs_data = {
        .prefix = DSCHED_INSTALL_PREFIX,
        .exec_prefix = DSCHED_EXEC_PREFIX,
        .bindir = DSCHED_BINDIR,
        .sbindir = DSCHED_SBINDIR,
        .libexecdir = DSCHED_LIBEXECDIR,
        .datarootdir = DSCHED_DATAROOTDIR,
        .datadir = DSCHED_DATADIR,
        .sysconfdir = DSCHED_SYSCONFDIR,
        .sharedstatedir = DSCHED_SHAREDSTATEDIR,
        .localstatedir = DSCHED_LOCALSTATEDIR,
        .libdir = DSCHED_LIBDIR,
        .includedir = DSCHED_INCLUDEDIR,
        .infodir = DSCHED_INFODIR,
        .mandir = DSCHED_MANDIR,
        .pmixdatadir = DSCHED_PKGDATADIR,
        .pmixlibdir = DSCHED_PKGLIBDIR,
        .pmixincludedir = DSCHED_PKGINCLUDEDIR
    }
};
PMIX_MCA_BASE_COMPONENT_INIT(dsched, dinstalldirs, config)
