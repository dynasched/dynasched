/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2021 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2010 Oracle and/or its affiliates.  All rights reserved.
 * Copyright (c) 2007-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2011-2013 Los Alamos National Security, LLC.
 *                         All rights reserved.
 * Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2017-2020 IBM Corporation.  All rights reserved.
 * Copyright (c) 2017-2019 Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/**
 * @file
 *
 * Global params for DSCHED
 */
#ifndef DSCHED_GLOBALS_H
#define DSCHED_GLOBALS_H

#include "dsched_config.h"
#include "dsched_types.h"

#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#endif

#include <pmix.h>
#include <hwloc.h>

#include "src/class/pmix_pointer_array.h"
#include "src/event/event-internal.h"
#include "src/threads/pmix_threads.h"
#include "src/util/pmix_show_help.h"

#include "src/util/dsched_attr.h"

BEGIN_C_DECLS

DSCHED_EXPORT extern pmix_show_help_file_t dsched_show_help_data[];

typedef int16_t dsched_node_state_t;

#define DSCHED_GLOBAL_ARRAY_BLOCK_SIZE 64
#define DSCHED_GLOBAL_ARRAY_MAX_SIZE   INT_MAX

/* define an object for timer events */
typedef struct {
    pmix_object_t super;
    struct timeval tv;
    dsched_event_t *ev;
    void *payload;
} dsched_timer_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_timer_t);


/* define an object for storing node topologies */
typedef struct {
    pmix_object_t super;
    int index;
    hwloc_topology_t topo;
    char *sig;
} dsched_topology_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_topology_t);

/* Object for tracking allocations */
typedef struct {
    pmix_object_t super;
    int index;
    uint32_t session_id;
    char *user_refid;  // PMIX_ALLOC_REQ_ID
    char *alloc_refid; // PMIX_ALLOC_ID
    struct timeval timeout;  // time limit on session
    pmix_pointer_array_t *nodes;
} dsched_session_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_session_t);

typedef struct {
    /** Base object  */
    pmix_object_t super;
    /* index of this node object in our global array */
    int32_t index;
    /** String node name */
    char *name;
    char *rawname;  // name originally given in allocation, if different from name
    /** aliases */
    char **aliases;
    uint32_t nodeid; // from the runtime
    /** A "soft" limit on the number of slots available on the node.
        This will typically correspond to the number of physical CPUs
        that we have been allocated on this note and would be the
        "ideal" number of processes for us to launch. */
    int32_t slots;
    /** Slots available for use in the current mapping operation. This
     *  may differ on a per-job basis from the overall allocated slots
     *  thru use of the -host option and possibly other means */
    int32_t slots_available;
    /** How many processes have already been launched, used by one or
        more jobs on this node. */
    int32_t slots_inuse;
    /** A "hard" limit (if set -- a value of 0 implies no hard limit)
        on the number of slots that can be allocated on a given
        node. This is for some environments (e.g. grid) where there
        may be fixed limits on the number of slots that can be used.

        This value also could have been a boolean - but we may want to
        allow the hard limit be different than the soft limit - in
        other words allow the node to be oversubscribed up to a
        specified limit.  For example, if we have two processors, we
        may want to allow up to four processes but no more. */
    int32_t slots_max;
    /* system topology for this node */
    dsched_topology_t *topology;
    /* flags */
    dsched_node_flags_t flags;
    /* list of dsched_attribute_t */
    pmix_list_t attributes;
} dsched_node_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_node_t);

typedef struct {
    pmix_list_item_t super;
    // name of scheduler that computed it
    char *scheduler;
    // priority of scheduler component that computed it
    int pri;
    /* array of resources that are assigned to this allocation */
    pmix_pointer_array_t allocation;
    //
} dsched_alloc_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_alloc_t);

/* track a session throughout its lifecycle - time is specified in
 * usual time format of months:days:hours:minutes:seconds, scanning
 * from right to left (i.e., a value of "2" equates to 2 seconds)
*/
typedef struct {
    /** Base object so this can be put on a list */
    pmix_list_item_t super;
    dsched_event_t ev;
    // allocation request info
    pmix_proc_t requestor;
    pmix_alloc_directive_t directive;
    int32_t index;
    // whether the data is a local copy
    bool copy;
    // original info keys
    pmix_info_t *data;
    size_t ndata;
    // callback upon completion
    pmix_info_cbfunc_t cbfunc;
    void *cbdata;
    // result of allocation request
    pmix_status_t status;
    // processed directives
    char *user_refid;
    char *alloc_refid;
    int num_nodes;
    char *nlist;
    char *exclude;
    int num_cpus;
    char *ncpulist;
    char *cpulist;
    float memsize;
    char *time;         // length of time allocation should be granted for
    char *queue;
    bool preemptible;   // jobs in allocation are preemptible
    char *lend;
    char *image;
    bool waitall;
    bool share;
    bool noshell;
    char *dependency;
    // Direct the scheduler to defer allocation until the specified time. Time
    // may be of the form HH:MM:SS to schedule the session to start at a specific
    // time of day (seconds are optional). If that time is already past, the
    // next day is assumed. You may also specify "midnight" or "noon", and you
    // can have a time-of-day suffixed with AM or PM for running in the morning
    // or the evening. You can also say what day the session should start by
    // specifying a date of the form MMDDYY or MM/DD/YY YYYY-MM-DD. Combine date
    // and time using the usual format YYYY-MM-DD[THH:MM[:SS]]. You can also give
    // times like "now + count" time-units, where the time-units can be "seconds"
    // (default), "minutes", "hours", days, or weeks, or you can ask that the
    // allocation be made "today" or "tomorrow".
    char *begintime;
    // assigned session info
    uint32_t sessionID;
    // time request was received
    time_t received;
    // assigned allocation - array of resources
    dsched_alloc_t *allocation;
} dsched_req_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_req_t);

// list caddy for requests
typedef struct {
    pmix_list_item_t super;
    dsched_req_t *req;
} dsched_req_item_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_req_item_t);

// callback object for meta components
typedef struct {
    pmix_object_t super;
    dsched_event_t ev;
    // number of active meta components
    int nactive;
    // number that have responded
    int nresponded;
    // the allocation request triggering this operation
    dsched_req_t *req;
    // allocation request being activated - if NULL, this
    // means that no request is ready to be fulfilled.
    dsched_req_t *activated;
} dsched_meta_t;
PMIX_CLASS_DECLARATION(dsched_meta_t);

// callback object for sched components
typedef struct {
    pmix_object_t super;
    dsched_event_t ev;
    // the allocation request associated with this response
    dsched_req_t *req;
    // recommended allocation request - if NULL, this
    // means that the scheduler could not activate
    // any pending request
    dsched_req_t *activated;
    // array of allocations (dsched_alloc_t) selected by
    // each meta component, one per active component
    pmix_pointer_array_t allocations;
} dsched_sched_t;
PMIX_CLASS_DECLARATION(dsched_sched_t);

// framework-level tracker
typedef struct {
    pmix_object_t super;
    dsched_event_t ev;
    int nactive;
    int nresponded;
    void *cbdata;
} dsched_op_tracker_t;
PMIX_CLASS_DECLARATION(dsched_op_tracker_t);

// threadshift caddy
typedef struct {
    pmix_object_t super;
    dsched_event_t ev;
    pmix_status_t status;
    int order;
    pmix_info_t *info;
    size_t ninfo;
    void *cbdata;
    pmix_proc_t target;
    uint32_t uid;
    uint32_t gid;
    pid_t pid;
    bool flag;
    char *hostname;
    char *cmdline;
    bool launcher;
    bool scheduler;
    pmix_query_t *queries;
    size_t nqueries;
    dsched_req_t *req;
    dsched_meta_t *mt;
    dsched_alloc_t *alloc;
    dsched_op_tracker_t *trk;
    pmix_pointer_array_t allocations;
    dsched_event_cbfunc_fn_t evcbfunc;
    pmix_info_cbfunc_t infocbfunc;
    pmix_tool_connection_cbfunc_t toolcbfunc;
} dsched_shift_caddy_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_shift_caddy_t);


// global variables
typedef struct {
    pmix_proc_t myid;
    pid_t pid;
    dsched_event_base_t *evbase;
    bool evactive;
    bool initialized;
    const char *version_string;
    char *basename;
    char *hostname;
    char **aliases;
    pmix_pointer_array_t nodes;
    pmix_pointer_array_t sessions;
    pmix_pointer_array_t topologies;
    pmix_pointer_array_t requests;
    struct {
        int nnodes;
        int nslots;
    } avail;
    char *param_files;
    char *override_param_file;
    bool suppress_override_warning;
    int clean_output;
    char *tmpdir;
    char *prohibited_session_dirs;
    int exit_status;
    bool debug;
    int verbosity;
    int output;
    int pmix_output;
    char *report_uri;
    pmix_list_t tools;
    pmix_proc_t syscontroller;
    bool controller_connected;
    int parent_fd;
    bool server_initialized;
    char *progress_thread_cpus;
    bool bind_progress_thread_reqd;
    bool keep_fqdn_hostnames;
    char *strip_prefixes;
} dsched_globals_t;
DSCHED_EXPORT extern dsched_globals_t dsched_globals;

/** Get session object */
DSCHED_EXPORT dsched_session_t *dsched_get_session_object(const uint32_t session_id);
DSCHED_EXPORT dsched_session_t *dsched_get_session_object_from_id(const char *id);
DSCHED_EXPORT dsched_session_t *dsched_get_session_object_from_refid(const char *refid);

DSCHED_EXPORT int dsched_set_session_object(dsched_session_t *session);

DSCHED_EXPORT bool dsched_sessions_related(dsched_session_t *session1, dsched_session_t *session2);

/* check to see if two nodes match */
DSCHED_EXPORT bool dsched_check_host_is_local(const char *name);
DSCHED_EXPORT dsched_node_t* dsched_node_match(pmix_list_t *nodes,
                                               const char *name,
                                               uint32_t nodeid);
DSCHED_EXPORT bool dsched_nptr_match(dsched_node_t *n1, dsched_node_t *n2);


#if DSCHED_PICKY_COMPILERS
#define DSCHED_HIDE_UNUSED_PARAMS(...)                \
    do {                                            \
        int __x = 3;                                \
        dsched_hide_unused_params(__x, __VA_ARGS__);  \
} while(0)

PMIX_EXPORT void dsched_hide_unused_params(int x, ...);

#else
#define DSCHED_HIDE_UNUSED_PARAMS(...)
#endif

#define DSCHED_UPDATE_EXIT_STATUS(r)            \
    do {                                        \
        if (0 == dsched_globals.exit_status) {  \
            dsched_globals.exit_status = (r);   \
        }                                       \
    } while(0)

#define DSCHED_MCA_BASE_VERSION_1_0_0(type, type_major, type_minor, type_release) \
    PMIX_MCA_BASE_VERSION_2_1_0("dsched", DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION, \
                                DSCHED_RELEASE_VERSION, type, type_major, type_minor, type_release)

END_C_DECLS

#endif /* DSCHED_GLOBALS_H */
