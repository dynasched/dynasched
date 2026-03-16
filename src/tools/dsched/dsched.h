/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2012-2013 Los Alamos National Security, LLC.
 *                         All rights reserved
 * Copyright (c) 2014-2019 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef DSCHED_H
#define DSCHED_H

#include "dsched_config.h"

#include <pmix.h>

#include "src/class/pmix_list.h"
#include "src/class/pmix_pointer_array.h"
#include "src/util/pmix_cmd_line.h"

#include "src/event/event-internal.h"
#include "src/include/dsched_globals.h"

BEGIN_C_DECLS


extern dsched_globals_t dsched_globals;

extern int dsched_server_init(pmix_cli_result_t *results);
extern void dsched_server_finalize(void);

extern pmix_status_t dsched_query_fn(pmix_proc_t *proct,
                                     pmix_query_t *queries, size_t nqueries,
                                     pmix_info_cbfunc_t cbfunc, void *cbdata);

extern pmix_status_t dsched_tool_connected_fn(pmix_info_t *info, size_t ninfo,
                                              pmix_tool_connection_cbfunc_t cbfunc, void *cbdata);

extern pmix_status_t dsched_alloc_fn(const pmix_proc_t *client,
                                     pmix_alloc_directive_t directive,
                                     const pmix_info_t data[], size_t ndata,
                                     pmix_info_cbfunc_t cbfunc, void *cbdata);

extern pmix_status_t dsched_session_ctrl_fn(const pmix_proc_t *requestor,
                                            uint32_t sessionID,
                                            const pmix_info_t directives[], size_t ndirs,
                                            pmix_info_cbfunc_t cbfunc, void *cbdata);

/* track a session throughout its lifecycle */
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
    // processed directives
    char *user_refid;
    char *alloc_refid;
    uint64_t num_nodes;
    char *nlist;
    char *exclude;
    uint64_t num_cpus;
    char *ncpulist;
    char *cpulist;
    float memsize;
    char *time;
    char *queue;
    bool preemptible;
    char *lend;
    char *image;
    bool waitall;
    bool share;
    bool noshell;
    char *dependency;
    char *begintime;
    // assigned session info
    uint32_t sessionID;
} dsched_req_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_req_t);

END_C_DECLS

#endif /* DSCHED_H */
