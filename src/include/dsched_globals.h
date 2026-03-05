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
typedef struct{
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
    /** Base object so this can be put on a list */
    pmix_list_item_t super;
    /* index of this node object in global array */
    int32_t index;
    /** String node name */
    char *name;
    char *rawname;  // name originally given in allocation, if different from name
    /** aliases */
    char **aliases;
    /* daemon on this node */
    struct dsched_proc_t *daemon;
    /* track the unassigned cpus */
    hwloc_cpuset_t available;
    /* cache the cpuset prior to mapping a job for easy reset */
    hwloc_cpuset_t jobcache;
    /** State of this node */
    dsched_node_state_t state;
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
        node. This is for some environments (e.g. grid) there may be
        fixed limits on the number of slots that can be used.

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

// global variables
typedef struct {
    dsched_event_base_t *evbase;
    int evpri;
    bool initialized;
    const char *version_string;
    char *basename;
    pmix_show_help_file_t *show_help_data;
    char hostname[DSCHED_MAXHOSTNAMELEN];
    pmix_pointer_array_t nodes;
    pmix_pointer_array_t sessions;
    pmix_pointer_array_t topologies;
    pmix_pointer_array_t cache;
    char *param_files;
    char *override_param_file;
    bool suppress_override_warning;
    int clean_output;
    char *tmpdir;
} dsched_globals_t;
DSCHED_EXPORT extern dsched_globals_t dsched_globals;

/** Get session object */
DSCHED_EXPORT dsched_session_t *dsched_get_session_object(const uint32_t session_id);
DSCHED_EXPORT dsched_session_t *dsched_get_session_object_from_id(const char *id);
DSCHED_EXPORT dsched_session_t *dsched_get_session_object_from_refid(const char *refid);

DSCHED_EXPORT int dsched_set_session_object(dsched_session_t *session);

DSCHED_EXPORT bool dsched_sessions_related(dsched_session_t *session1, dsched_session_t *session2);

/* check to see if two nodes match */
DSCHED_EXPORT dsched_node_t* dsched_node_match(pmix_list_t *nodes, const char *name);
DSCHED_EXPORT bool dsched_nptr_match(dsched_node_t *n1, dsched_node_t *n2);
DSCHED_EXPORT bool dsched_quickmatch(dsched_node_t *nd, char *name);

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

#define DSCHED_MCA_BASE_VERSION_1_0_0(type, type_major, type_minor, type_release) \
    PMIX_MCA_BASE_VERSION_2_1_0("dsched", DSCHED_MAJOR_VERSION, DSCHED_MINOR_VERSION, \
                                DSCHED_RELEASE_VERSION, type, type_major, type_minor, type_release)

END_C_DECLS

#endif /* DSCHED_GLOBALS_H */
