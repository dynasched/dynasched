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
 * Copyright (c) 2012-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2015-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2016-2017 Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2017-2022 IBM Corporation.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef DSCHED_CMD_LINE_H
#define DSCHED_CMD_LINE_H

#include "dsched_config.h"

#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "src/class/pmix_list.h"
#include "src/class/pmix_object.h"
#include "src/util/pmix_argv.h"

BEGIN_C_DECLS

/* define the command line options that DynaSched internally understands */

//      NAME                            STRING                      ARGUMENT

// Basic options
#define DSCHED_CLI_HELP                   "help"                      // optional
#define DSCHED_CLI_VERSION                "version"                   // none
#define DSCHED_CLI_VERBOSE                "verbose"                   // number of instances => verbosity level

// MCA parameter options
#define DSCHED_CLI_DMCA                   "dmca"                      // requires TWO
#define DSCHED_CLI_PMIXMCA                "pmixmca"                   // requires TWO
#define DSCHED_CLI_MCA                    "mca"                       // requires TWO

// DVM options
#define DSCHED_CLI_NO_READY_MSG           "no-ready-msg"              // none
#define DSCHED_CLI_DAEMONIZE              "daemonize"                 // none
#define DSCHED_CLI_SET_SID                "set-sid"                   // none
#define DSCHED_CLI_REPORT_PID             "report-pid"                // required
#define DSCHED_CLI_REPORT_URI             "report-uri"                // required
#define DSCHED_CLI_KEEPALIVE              "keepalive"                 // required
#define DSCHED_CLI_DEBUG                  "debug"                     // none
#define DSCHED_CLI_TMPDIR                 "tmpdir"                    // required
#define DSCHED_CLI_RUN_AS_ROOT            "allow-run-as-root"         // none

// Daemon-specific CLI options
#define DSCHED_CLI_CONTROLLER_URI         "controller"                // required

END_C_DECLS

#endif /* DSCHED_CMD_LINE_H */
