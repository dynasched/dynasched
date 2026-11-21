/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2006-2015 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef DSCHED_MCA_PINSTALLDIRS_H
#define DSCHED_MCA_PINSTALLDIRS_H

#include "src/include/dsched_config.h"

#include "src/mca/base/pmix_base.h"
#include "src/mca/mca.h"
#include "src/include/dsched_mca.h"
#include "src/mca/pinstalldirs/pinstalldirs.h"

BEGIN_C_DECLS


/* Install directories.  Only available after prte_init() */
DSCHED_EXPORT extern pmix_pinstall_dirs_t dsched_dinstall_dirs;

/**
 * Expand out path variables (such as ${prefix}) in the input string
 * using the current dsched_dinstall_dirs structure */
DSCHED_EXPORT char *dsched_dinstall_dirs_expand(const char *input);

/**
 * Structure for dinstalldirs components.
 */
struct dsched_dinstalldirs_base_component_2_0_0_t {
    /** MCA base component */
    pmix_mca_base_component_t component;
    /** install directories provided by the given component */
    pmix_pinstall_dirs_t install_dirs_data;
};
/**
 * Convenience typedef
 */
typedef struct dsched_dinstalldirs_base_component_2_0_0_t dsched_dinstalldirs_base_component_t;

/*
 * Macro for use in components that are of type dinstalldirs
 */
/* The dinstalldirs framework interface version. It is stated here and nowhere
 * else: components stamp it into their struct with
 * DSCHED_MCA_BASE_VERSION(dinstalldirs), and the framework's declaration reaches
 * the same three by pasting its name, so the two cannot drift apart.
 * Bump it on any change to the module interface that a component built
 * against the previous one would not survive. */
#define DSCHED_MCA_dinstalldirs_MAJOR_VERSION   1
#define DSCHED_MCA_dinstalldirs_MINOR_VERSION   0
#define DSCHED_MCA_dinstalldirs_RELEASE_VERSION 0

END_C_DECLS

#endif /* DSCHED_MCA_PINSTALLDIRS_H */
