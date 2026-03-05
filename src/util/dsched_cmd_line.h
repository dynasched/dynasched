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

/* define the command line options that PRRTE internally understands.
 * It is the responsibility of each schizo component to translate its
 * command line inputs to these definitions. The definitions are provided
 * to help avoid errors due to typos - i.e., where the schizo component
 * interprets its CLI but assigns it to an erroneous string */

//      NAME                            STRING                      ARGUMENT

// Basic options
#define DSCHED_CLI_HELP                   "help"                      // optional
#define DSCHED_CLI_VERSION                "version"                   // none
#define DSCHED_CLI_VERBOSE                "verbose"                   // number of instances => verbosity level
#define DSCHED_CLI_PARSEABLE              "parseable"                 // none
#define DSCHED_CLI_PARSABLE               "parsable"                  // none
#define DSCHED_CLI_PERSONALITY            "personality"               // required

// MCA parameter options
#define DSCHED_CLI_DMCA                   "dmca"                      // requires TWO
#define DSCHED_CLI_PMIXMCA                "pmixmca"                   // requires TWO
#define DSCHED_CLI_MCA                    "mca"                       // required

// DVM options
#define DSCHED_CLI_NO_READY_MSG           "no-ready-msg"              // none
#define DSCHED_CLI_DAEMONIZE              "daemonize"                 // none
#define DSCHED_CLI_SYSTEM_SERVER          "system-server"             // none
#define DSCHED_CLI_SET_SID                "set-sid"                   // none
#define DSCHED_CLI_REPORT_PID             "report-pid"                // required
#define DSCHED_CLI_REPORT_URI             "report-uri"                // required
#define DSCHED_CLI_TEST_SUICIDE           "test-suicide"              // none
#define DSCHED_CLI_DEFAULT_HOSTFILE       "default-hostfile"          // required
#define DSCHED_CLI_SINGLETON              "singleton"                 // required
#define DSCHED_CLI_KEEPALIVE              "keepalive"                 // required
#define DSCHED_CLI_LAUNCH_AGENT           "launch-agent"              // required
#define DSCHED_CLI_MAX_VM_SIZE            "max-vm-size"               // required
#define DSCHED_CLI_DEBUG                  "debug"                     // none
#define DSCHED_CLI_DEBUG_DAEMONS          "debug-daemons"             // none
#define DSCHED_CLI_DEBUG_DAEMONS_FILE     "debug-daemons-file"        // none
#define DSCHED_CLI_LEAVE_SESSION_ATTACHED "leave-session-attached"    // none
#define DSCHED_CLI_TMPDIR                 "tmpdir"                    // required
#define DSCHED_CLI_PREFIX                 "prefix"                    // required
#define DSCHED_CLI_NOPREFIX               "noprefix"                  // none
#define DSCHED_CLI_FWD_SIGNALS            "forward-signals"           // required
#define DSCHED_CLI_RUN_AS_ROOT            "allow-run-as-root"         // none
#define DSCHED_CLI_STREAM_BUF             "stream-buffering"          // required
#define DSCHED_CLI_BOOTSTRAP              "bootstrap"                 // none

// Application options
#define DSCHED_CLI_NP                     "np"                        // required
#define DSCHED_CLI_NPERNODE               "N"                         // required
#define DSCHED_CLI_APPFILE                "app"                       // required
#define DSCHED_CLI_FWD_ENVAR              "x"                         // required
#define DSCHED_CLI_FWD_ENVIRON            "fwd-environment"           // optional
#define DSCHED_CLI_HOSTFILE               "hostfile"                  // required
#define DSCHED_CLI_ADDHOSTFILE            "add-hostfile"              // required
#define DSCHED_CLI_HOST                   "host"                      // required
#define DSCHED_CLI_ADDHOST                "add-host"                  // required
#define DSCHED_CLI_PATH                   "path"                      // required
#define DSCHED_CLI_PSET                   "pset"                      // required
#define DSCHED_CLI_PRELOAD_FILES          "preload-files"             // required
#define DSCHED_CLI_PRELOAD_BIN            "preload-binary"            // none
#define DSCHED_CLI_STDIN                  "stdin"                     // required
#define DSCHED_CLI_OUTPUT                 "output"                    // required
#define DSCHED_CLI_WDIR                   "wdir"                      // required
#define DSCHED_CLI_SET_CWD_SESSION        "set-cwd-to-session-dir"    // none
#define DSCHED_CLI_ENABLE_RECOVERY        "enable-recovery"           // none
#define DSCHED_CLI_DISABLE_RECOVERY       "disable-recovery"          // none
#define DSCHED_CLI_MEM_ALLOC_KIND			"memory-alloc-kinds"        // required

// Placement options
#define DSCHED_CLI_MAPBY                  "map-by"                    // required
#define DSCHED_CLI_RANKBY                 "rank-by"                   // required
#define DSCHED_CLI_BINDTO                 "bind-to"                   // required

// Runtime options
#define DSCHED_CLI_RTOS                   "runtime-options"           // required

// Debug options
#define DSCHED_CLI_DO_NOT_LAUNCH          "do-not-launch"             // none
#define DSCHED_CLI_DISPLAY                "display"                   // required
#define DSCHED_CLI_XTERM                  "xterm"                     // none
#define DSCHED_CLI_DO_NOT_AGG_HELP        "no-aggregate-help"         // none

// Tool connection options
#define DSCHED_CLI_SYS_SERVER_FIRST       "system-server-first"       // none
#define DSCHED_CLI_SYS_SERVER_ONLY        "system-server-only"        // none
#define DSCHED_CLI_DO_NOT_CONNECT         "do-not-connect"            // none
#define DSCHED_CLI_WAIT_TO_CONNECT        "wait-to-connect"           // required
#define DSCHED_CLI_NUM_CONNECT_RETRIES    "num-connect-retries"       // required
#define DSCHED_CLI_PID                    "pid"                       // required
#define DSCHED_CLI_NAMESPACE              "namespace"                 // required
#define DSCHED_CLI_DVM_URI                "dvm-uri"                   // required
#define DSCHED_CLI_DVM                    "dvm"                       // optional

// Daemon-specific CLI options
#define DSCHED_CLI_PUBSUB_SERVER          "prte-server"               // required
#define DSCHED_CLI_CONTROLLER_URI         "dvm-master-uri"            // required
#define DSCHED_CLI_PARENT_URI             "parent-uri"                // required
#define DSCHED_CLI_TREE_SPAWN             "tree-spawn"                // required
#define DSCHED_CLI_PLM                    "plm"                       // required


/* define accepted synonyms - these must be defined on the schizo component's
 * command line in order to be accepted, but PRRTE will automatically translate
 * them to their accepted synonym */
#define DSCHED_CLI_MACHINEFILE    "machinefile"       // synonym for "hostfile"
#define DSCHED_CLI_WD             "wd"                // synonym for "wdir


/* define the command line directives PRRTE recognizes */

// Placement directives - used by mapping and binding
#define DSCHED_CLI_SLOT       "slot"
#define DSCHED_CLI_HWT        "hwthread"
#define DSCHED_CLI_CORE       "core"
#define DSCHED_CLI_L1CACHE    "l1cache"
#define DSCHED_CLI_L2CACHE    "l2cache"
#define DSCHED_CLI_L3CACHE    "l3cache"
#define DSCHED_CLI_NUMA       "numa"
#define DSCHED_CLI_PACKAGE    "package"
#define DSCHED_CLI_NODE       "node"
#define DSCHED_CLI_SEQ        "seq"
#define DSCHED_CLI_DIST       "dist"
#define DSCHED_CLI_PPR        "ppr"
#define DSCHED_CLI_RANKFILE   "rankfile"
#define DSCHED_CLI_NONE       "none"
#define DSCHED_CLI_HWTCPUS    "hwtcpus"
#define DSCHED_CLI_PELIST     "pe-list="
#define DSCHED_CLI_LIKWID     "likwid"

// Ranking directives
// DSCHED_CLI_SLOT, DSCHED_CLI_NODE, DSCHED_CLI_SPAN reused here
#define DSCHED_CLI_FILL       "fill"
#define DSCHED_CLI_OBJ        "object"


// Output directives
#define DSCHED_CLI_TAG        "tag"
#define DSCHED_CLI_TAG_DET    "tag-detailed"
#define DSCHED_CLI_TAG_FULL   "tag-fullname"
#define DSCHED_CLI_RANK       "rank"
#define DSCHED_CLI_TIMESTAMP  "timestamp"
#define DSCHED_CLI_XML        "xml"
#define DSCHED_CLI_MERGE_ERROUT   "merge-stderr-to-stdout"
#define DSCHED_CLI_DIR        "directory"
#define DSCHED_CLI_FILE       "filename"

// Display directives
#define DSCHED_CLI_ALLOC      "allocation"
#define DSCHED_CLI_MAP        "map"
#define DSCHED_CLI_BIND       "bind"
#define DSCHED_CLI_MAPDEV     "map-devel"
#define DSCHED_CLI_TOPO       "topo="
#define DSCHED_CLI_CPUS       "cpus="

// Runtime directives
#define DSCHED_CLI_ERROR_NZ           "error-nonzero-status"          // optional arg
#define DSCHED_CLI_NOLAUNCH           "donotlaunch"                   // no arg
#define DSCHED_CLI_SHOW_PROGRESS      "show-progress"                 // optional arg
#define DSCHED_CLI_RECOVERABLE        "recoverable"                   // optional arg
#define DSCHED_CLI_AUTORESTART        "autorestart"                   // optional arg
#define DSCHED_CLI_CONTINUOUS         "continuous"                    // optional arg
#define DSCHED_CLI_MAX_RESTARTS       "max-restarts"                  // reqd arg
#define DSCHED_CLI_EXEC_AGENT         "exec-agent"                    // reqd arg
#define DSCHED_CLI_DEFAULT_EXEC_AGENT "default-exec-agent"            // no arg
#define DSCHED_CLI_STOP_ON_EXEC       "stop-on-exec"                  // optional arg
#define DSCHED_CLI_STOP_IN_INIT       "stop-in-init"                  // optional arg
#define DSCHED_CLI_STOP_IN_APP        "stop-in-app"                   // optional arg
#define DSCHED_CLI_TIMEOUT            "timeout"                       // reqd arg
#define DSCHED_CLI_SPAWN_TIMEOUT      "spawn-timeout"                 // reqd arg
#define DSCHED_CLI_REPORT_STATE       "report-state-on-timeout"       // optional arg
#define DSCHED_CLI_STACK_TRACES       "get-stack-traces"              // optional arg
#define DSCHED_CLI_REPORT_CHILD_SEP   "report-child-jobs-separately"  // optional arg
#define DSCHED_CLI_AGG_HELP           "aggregate-help"                // optional arg
#define DSCHED_CLI_NOTIFY_ERRORS      "notifyerrors"                  // optional flag
#define DSCHED_CLI_OUTPUT_PROCTABLE   "output-proctable"              // optional arg


/* define the command line qualifiers PRRTE recognizes */

// Placement qualifiers
#define DSCHED_CLI_PE         "pe="
#define DSCHED_CLI_SPAN       "span"
#define DSCHED_CLI_OVERSUB    "oversubscribe"
#define DSCHED_CLI_NOOVER     "nooversubscribe"
#define DSCHED_CLI_NOLOCAL    "nolocal"
// DSCHED_CLI_HWTCPUS reused here
#define DSCHED_CLI_CORECPUS   "corecpus"
#define DSCHED_CLI_DEVICE     "device="
#define DSCHED_CLI_INHERIT    "inherit"
#define DSCHED_CLI_NOINHERIT  "noinherit"
#define DSCHED_CLI_QDIR       "dir="
#define DSCHED_CLI_QFILE      "file="
#define DSCHED_CLI_OVERLOAD   "overload-allowed"
#define DSCHED_CLI_NOOVERLOAD "no-overload"
#define DSCHED_CLI_IF_SUPP    "if-supported"
#define DSCHED_CLI_ORDERED    "ordered"
#define DSCHED_CLI_REPORT     "report"
#define DSCHED_CLI_DISPALLOC  "displayalloc"
// DSCHED_CLI_DISPLAY reused here
#define DSCHED_CLI_DISPDEV    "displaydevel"

// Output qualifiers
#define DSCHED_CLI_NOCOPY     "nocopy"
#define DSCHED_CLI_RAW        "raw"
#define DSCHED_CLI_PATTERN    "pattern"

END_C_DECLS

#endif /* DSCHED_CMD_LINE_H */
