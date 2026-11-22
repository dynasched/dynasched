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

#include "src/include/dsched_mca.h"
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

/* An allocation that has been granted, from the grant until the
 * requester releases the last of its nodes */
typedef struct {
    pmix_object_t super;
    int index;              // in dsched_globals.sessions
    uint32_t session_id;    // PMIX_SESSION_ID
    char *user_refid;       // PMIX_ALLOC_REQ_ID the requester gave
    char *alloc_refid;      // PMIX_ALLOC_ID we assigned
    pmix_proc_t requestor;
    uint32_t uid;           // the requester's, as PMIx established it
    uint32_t gid;
    pmix_pointer_array_t nodes;  // dsched_node_t, each one retained
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
    /* the allocation the node is in, or NULL if it is free. Read and
     * written only on dsched_globals.evbase. Not a reference: the
     * session holds one on the node, and clears this before it goes */
    dsched_session_t *session;
    /* flags */
    dsched_node_flags_t flags;
    /* list of dsched_attribute_t */
    pmix_list_t attributes;
} dsched_node_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_node_t);

/* A scheduler's proposal for one scheduling round: grant a pending
 * request these nodes (status PMIX_SUCCESS), or refuse it (any other
 * status - the request cannot be met even with every node free). A
 * scheduler that would neither grant nor refuse proposes nothing. */
struct dsched_req_s;
typedef struct {
    pmix_list_item_t super;
    // name of scheduler that computed it
    char *scheduler;
    // priority of scheduler component that computed it
    int pri;
    // the request it is for - retained
    struct dsched_req_s *req;
    // grant or refuse
    pmix_status_t status;
    /* the nodes to grant (dsched_node_t), each one retained */
    pmix_pointer_array_t allocation;
} dsched_alloc_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_alloc_t);

/* track a session throughout its lifecycle - time is specified in
 * usual time format of months:days:hours:minutes:seconds, scanning
 * from right to left (i.e., a value of "2" equates to 2 seconds)
*/
typedef struct dsched_req_s {
    /** Base object so this can be put on a list */
    pmix_list_item_t super;
    dsched_event_t ev;
    // allocation request info
    pmix_proc_t requestor;
    pmix_alloc_directive_t directive;
    // whether it is on dsched_globals.pending
    bool pending;
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
    // the hosts named by nlist and exclude, as names
    char **nodes_wanted;
    char **nodes_excluded;
    // who asked: the user and group PMIx established for the requester
    uint32_t uid;
    uint32_t gid;
    bool have_uid;
    // whom the allocation is for, if the request said (PMIX_ALLOC_UID/GID)
    uint32_t alloc_uid;
    uint32_t alloc_gid;
    bool have_alloc_uid;
    bool have_alloc_gid;
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

/* One node as a scheduling round sees it. The round's schedulers run
 * on their own threads, so they read this copy and never the node's
 * own state, which only dsched_globals.evbase reads and writes. */
typedef struct {
    dsched_node_t *node;    // retained; only its name and aliases are read
    int32_t slots;
    bool free;
} dsched_node_snap_t;

/* One scheduling round. Rounds run one at a time: the schedulers are
 * handed a snapshot of the pool and of the pending requests, in the
 * order they arrived, and propose; the meta component chooses one
 * proposal; and the base applies it on dsched_globals.evbase. */
struct dsched_meta_s;
typedef void (*dsched_meta_cbfunc_t)(struct dsched_meta_s *mt);
typedef struct dsched_meta_s {
    pmix_object_t super;
    dsched_event_t ev;
    // called on dsched_globals.evbase once every meta component has
    // answered; it owns the round from then on, and releases it
    dsched_meta_cbfunc_t cbfunc;
    // number of active meta components
    int nactive;
    // number that have responded
    int nresponded;
    // the pool, as the round sees it
    dsched_node_snap_t *pool;
    int npool;
    // the pending requests, oldest first, each one retained
    dsched_req_t **pending;
    int npending;
    // the proposal chosen, retained - NULL if none
    dsched_alloc_t *alloc;
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
    // allocation requests waiting to be granted, oldest first
    pmix_list_t pending;
    // a scheduling round is running, and another is wanted after it
    bool round_active;
    bool round_again;
    // the next session and allocation numbers to assign
    uint32_t next_session_id;
    uint32_t next_alloc_id;
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
    // the hosts, in --host format, to allocate when nothing else names any
    char *default_hosts;
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

END_C_DECLS

#endif /* DSCHED_GLOBALS_H */
