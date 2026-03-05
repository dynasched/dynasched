/*
 * Copyright (c) 2006-2012 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2007      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2010      Sandia National Laboratories. All rights reserved.
 * Copyright (c) 2015      Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */

#include "src/include/pmix_config.h"

#include "src/mca/mca.h"
#include "src/mca/dinstalldirs/base/base.h"
#include "src/mca/dinstalldirs/base/static-components.h"
#include "src/mca/dinstalldirs/dinstalldirs.h"

pmix_pinstall_dirs_t dsched_dinstall_dirs = {
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
};

#define CONDITIONAL_COPY(target, origin, field)             \
    do {                                                    \
        if (origin.field != NULL && target.field == NULL) { \
            target.field = origin.field;                    \
        }                                                   \
    } while (0)

static int dsched_dinstalldirs_base_open(pmix_mca_base_open_flag_t flags)
{
    return pmix_mca_base_framework_components_open(&dsched_dinstalldirs_base_framework, flags);
}

int dsched_dinstall_dirs_base_init(pmix_info_t info[], size_t ninfo)
{
    pmix_mca_base_component_list_item_t *component_item;

    PMIX_LIST_FOREACH (component_item, &dsched_dinstalldirs_base_framework.framework_components,
                       pmix_mca_base_component_list_item_t) {
        const dsched_dinstalldirs_base_component_t *component
            = (const dsched_dinstalldirs_base_component_t *) component_item->cli_component;

        if (NULL != component->init) {
            component->init(info, ninfo);
        }

        /* copy over the data, if something isn't already there */
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, prefix);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, exec_prefix);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, bindir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, sbindir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, libexecdir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, datarootdir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, datadir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, sysconfdir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, sharedstatedir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, localstatedir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, libdir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, includedir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, infodir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, mandir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, pmixdatadir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, pmixlibdir);
        CONDITIONAL_COPY(dsched_dinstall_dirs, component->install_dirs_data, pmixincludedir);
    }

    /* expand out all the fields */
    dsched_dinstall_dirs.prefix = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.prefix);
    dsched_dinstall_dirs.exec_prefix = dsched_dinstall_dirs_expand_setup(
        dsched_dinstall_dirs.exec_prefix);
    dsched_dinstall_dirs.bindir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.bindir);
    dsched_dinstall_dirs.sbindir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.sbindir);
    dsched_dinstall_dirs.libexecdir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.libexecdir);
    dsched_dinstall_dirs.datarootdir = dsched_dinstall_dirs_expand_setup(
        dsched_dinstall_dirs.datarootdir);
    dsched_dinstall_dirs.datadir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.datadir);
    dsched_dinstall_dirs.sysconfdir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.sysconfdir);
    dsched_dinstall_dirs.sharedstatedir = dsched_dinstall_dirs_expand_setup(
        dsched_dinstall_dirs.sharedstatedir);
    dsched_dinstall_dirs.localstatedir = dsched_dinstall_dirs_expand_setup(
        dsched_dinstall_dirs.localstatedir);
    dsched_dinstall_dirs.libdir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.libdir);
    dsched_dinstall_dirs.includedir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.includedir);
    dsched_dinstall_dirs.infodir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.infodir);
    dsched_dinstall_dirs.mandir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.mandir);
    dsched_dinstall_dirs.pmixdatadir = dsched_dinstall_dirs_expand_setup(
        dsched_dinstall_dirs.pmixdatadir);
    dsched_dinstall_dirs.pmixlibdir = dsched_dinstall_dirs_expand_setup(dsched_dinstall_dirs.pmixlibdir);
    dsched_dinstall_dirs.pmixincludedir = dsched_dinstall_dirs_expand_setup(
        dsched_dinstall_dirs.pmixincludedir);

#if 0
    fprintf(stderr, "prefix:         %s\n", dsched_dinstall_dirs.prefix);
    fprintf(stderr, "exec_prefix:    %s\n", dsched_dinstall_dirs.exec_prefix);
    fprintf(stderr, "bindir:         %s\n", dsched_dinstall_dirs.bindir);
    fprintf(stderr, "sbindir:        %s\n", dsched_dinstall_dirs.sbindir);
    fprintf(stderr, "libexecdir:     %s\n", dsched_dinstall_dirs.libexecdir);
    fprintf(stderr, "datarootdir:    %s\n", dsched_dinstall_dirs.datarootdir);
    fprintf(stderr, "datadir:        %s\n", dsched_dinstall_dirs.datadir);
    fprintf(stderr, "sysconfdir:     %s\n", dsched_dinstall_dirs.sysconfdir);
    fprintf(stderr, "sharedstatedir: %s\n", dsched_dinstall_dirs.sharedstatedir);
    fprintf(stderr, "localstatedir:  %s\n", dsched_dinstall_dirs.localstatedir);
    fprintf(stderr, "libdir:         %s\n", dsched_dinstall_dirs.libdir);
    fprintf(stderr, "includedir:     %s\n", dsched_dinstall_dirs.includedir);
    fprintf(stderr, "infodir:        %s\n", dsched_dinstall_dirs.infodir);
    fprintf(stderr, "mandir:         %s\n", dsched_dinstall_dirs.mandir);
    fprintf(stderr, "pkgdatadir:     %s\n", dsched_dinstall_dirs.pkgdatadir);
    fprintf(stderr, "pkglibdir:      %s\n", dsched_dinstall_dirs.pkglibdir);
    fprintf(stderr, "pkgincludedir:  %s\n", dsched_dinstall_dirs.pkgincludedir);
#endif

    /* NTH: Is it ok not to close the components? If not we can add a flag
       to mca_base_framework_components_close to indicate not to deregister
       variable groups */
    return PMIX_SUCCESS;
}

static int dsched_dinstalldirs_base_close(void)
{
    free(dsched_dinstall_dirs.prefix);
    free(dsched_dinstall_dirs.exec_prefix);
    free(dsched_dinstall_dirs.bindir);
    free(dsched_dinstall_dirs.sbindir);
    free(dsched_dinstall_dirs.libexecdir);
    free(dsched_dinstall_dirs.datarootdir);
    free(dsched_dinstall_dirs.datadir);
    free(dsched_dinstall_dirs.sysconfdir);
    free(dsched_dinstall_dirs.sharedstatedir);
    free(dsched_dinstall_dirs.localstatedir);
    free(dsched_dinstall_dirs.libdir);
    free(dsched_dinstall_dirs.includedir);
    free(dsched_dinstall_dirs.infodir);
    free(dsched_dinstall_dirs.mandir);
    free(dsched_dinstall_dirs.pmixdatadir);
    free(dsched_dinstall_dirs.pmixlibdir);
    free(dsched_dinstall_dirs.pmixincludedir);
    memset(&dsched_dinstall_dirs, 0, sizeof(dsched_dinstall_dirs));

    return pmix_mca_base_framework_components_close(&dsched_dinstalldirs_base_framework, NULL);
}

/* Declare the dinstalldirs framework */
PMIX_MCA_BASE_FRAMEWORK_DECLARE(dsched, dinstalldirs, NULL, NULL, dsched_dinstalldirs_base_open,
                                dsched_dinstalldirs_base_close,
                                dsched_mca_dinstalldirs_base_static_components,
                                PMIX_MCA_BASE_FRAMEWORK_FLAG_NOREGISTER
                                    | PMIX_MCA_BASE_FRAMEWORK_FLAG_NO_DSO);
