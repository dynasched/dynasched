/*
 * Copyright (c) 2026      Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/** @file
 *
 * Macros that declare a DynaSched MCA framework and stamp its
 * components.  Kept apart from dsched_globals.h so that a low-level
 * framework (dinstalldirs) can use them without pulling in the daemon's
 * global state.
 */

#ifndef DSCHED_MCA_H
#define DSCHED_MCA_H

#include "dsched_config.h"

#include "src/mca/mca.h"
#include "src/mca/base/pmix_mca_base_framework.h"

BEGIN_C_DECLS

/* A framework's interface version, as stated in the framework's own
 * header:
 *
 *   #define DSCHED_MCA_dsched_MAJOR_VERSION   1
 *   #define DSCHED_MCA_dsched_MINOR_VERSION   0
 *   #define DSCHED_MCA_dsched_RELEASE_VERSION 0
 *
 * The name carries the framework's directory name verbatim, lower case
 * and all: the preprocessor pastes tokens, it does not upper-case them. */
#define DSCHED_MCA_FW_VER_(name, level) DSCHED_MCA_##name##_##level##_VERSION
#define DSCHED_MCA_FW_VER(name, level)  DSCHED_MCA_FW_VER_(name, level)

/* Open a component struct.
 *
 * Every DynaSched component begins its base struct with this, naming its
 * framework as a bare token - the framework's directory name, so that the
 * same token both stringifies into the struct and pastes into the version
 * macros above:
 *
 *   dsched_dsched_base_component_t dsched_mca_dsched_fifo_component = {
 *       DSCHED_MCA_BASE_VERSION(dsched),
 *       .pmix_mca_component_name = "fifo",
 *       ...
 *
 * PMIx's PMIX_MCA_BASE_VERSION() cannot serve here: it stamps the project
 * as "pmix" with PMIx's own version numbers. This one says "dsched" and
 * DynaSched's. */
#define DSCHED_MCA_BASE_VERSION(type)                                                  \
    PMIX_MCA_BASE_VERSION_2_1_0("dsched", DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION,  \
                                DSCHED_RELEASE_VERSION, #type,                         \
                                DSCHED_MCA_FW_VER(type, MAJOR),                        \
                                DSCHED_MCA_FW_VER(type, MINOR),                        \
                                DSCHED_MCA_FW_VER(type, RELEASE))

/* Declare a DynaSched framework, stating its interface version.
 *
 * Built on the same underlying declaration as PMIx's versioned form, so
 * that the framework reports the version its header states and
 * pmix_mca_base_components_open() can refuse a component built against a
 * different one. It takes no version arguments and no project argument:
 * every framework here is "dsched", and the numbers come from the header.
 * A framework that uses this without defining the three version macros
 * does not compile, which is the point. */
#define DSCHED_MCA_BASE_FRAMEWORK_DECLARE(name, description, registerfn, openfn,     \
                                          closefn, static_components, flags)         \
    PMIX_MCA_BASE_FRAMEWORK_DECLARE_FULL(dsched, name,                               \
                                         DSCHED_MCA_FW_VER(name, MAJOR),             \
                                         DSCHED_MCA_FW_VER(name, MINOR),             \
                                         description, registerfn, openfn, closefn,   \
                                         static_components, flags)

END_C_DECLS

#endif /* DSCHED_MCA_H */
