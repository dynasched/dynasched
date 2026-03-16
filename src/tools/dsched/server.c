/*
 * Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2011 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2006-2013 Los Alamos National Security, LLC.
 *                         All rights reserved.
 * Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2011      Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2014-2017 Mellanox Technologies, Inc.
 *                         All rights reserved.
 * Copyright (c) 2014-2019 Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * Copyright (c) 2023      Triad National Security, LLC. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */

#include "dsched_config.h"
#include "src/include/dsched_types.h"

#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#endif
#include <fcntl.h>
#ifdef HAVE_NETINET_IN_H
#    include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#    include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#    include <netdb.h>
#endif
#include <ctype.h>

#include "src/class/pmix_hotel.h"
#include "src/class/pmix_list.h"
#include "src/util/pmix_argv.h"
#include "src/util/pmix_os_dirpath.h"
#include "src/util/pmix_os_path.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_printf.h"
#include "src/util/pmix_environ.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_globals.h"
#include "src/include/dsched_stdint.h"
#include "src/threads/pmix_threads.h"
#include "src/util/dsched_cmd_line.h"
#include "src/util/dsched_error.h"
#include "src/util/dsched_session_dir.h"

#include "src/tools/dsched/dsched.h"

static pmix_server_module_t dsched_server = {
    .query = dsched_query_fn,
    .tool_connected2 = dsched_tool_connected_fn,
    .allocate = dsched_alloc_fn,
    .session_control = dsched_session_ctrl_fn
};

typedef struct {
    char *function;
    char **attrs;
} dsched_regattr_input_t;

static dsched_regattr_input_t dsched_attributes[] = {
    {.function = "PMIx_Query_info",
     .attrs = (char *[]){"PMIX_QUERY_NAMESPACES",
                         "PMIX_QUERY_NAMESPACE_INFO",
                         "PMIX_QUERY_SPAWN_SUPPORT",
                         "PMIX_QUERY_DEBUG_SUPPORT",
                         "PMIX_HWLOC_XML_V1",
                         "PMIX_HWLOC_XML_V2",
                         "PMIX_PROC_URI",
                         "PMIX_QUERY_PROC_TABLE",
                         "PMIX_QUERY_LOCAL_PROC_TABLE",
                         "PMIX_QUERY_NUM_PSETS",
                         "PMIX_QUERY_PSET_NAMES",
                         "PMIX_JOB_SIZE",
                         "PMIX_QUERY_NUM_GROUPS",
                         "PMIX_QUERY_GROUP_NAMES",
                         "PMIX_QUERY_GROUP_MEMBERSHIP",
                         NULL}},
    {.function = "PMIx_Query_info_nb",
     .attrs = (char *[]){"PMIX_QUERY_NAMESPACES",
                         "PMIX_QUERY_NAMESPACE_INFO",
                         "PMIX_QUERY_SPAWN_SUPPORT",
                         "PMIX_QUERY_DEBUG_SUPPORT",
                         "PMIX_HWLOC_XML_V1",
                         "PMIX_HWLOC_XML_V2",
                         "PMIX_PROC_URI",
                         "PMIX_QUERY_PROC_TABLE",
                         "PMIX_QUERY_LOCAL_PROC_TABLE",
                         "PMIX_QUERY_NUM_PSETS",
                         "PMIX_QUERY_PSET_NAMES",
                         "PMIX_JOB_SIZE",
                         "PMIX_QUERY_NUM_GROUPS",
                         "PMIX_QUERY_GROUP_NAMES",
                         "PMIX_QUERY_GROUP_MEMBERSHIP",
                         "PMIX_QUERY_ALLOCATION",
                         "PMIX_QUERY_ALLOC_STATUS",
                         NULL}},
    {.function = "PMIx_Log", .attrs = (char *[]){"NONE", NULL}},
    {.function = "PMIx_Log_nb", .attrs = (char *[]){"NONE", NULL}},
    {.function = "PMIx_Register_event_handler", .attrs = (char *[]){"NONE", NULL}},
    {.function = "PMIx_Deregister_event_handler", .attrs = (char *[]){"N/A", NULL}},
    {.function = "PMIx_Notify_event", .attrs = (char *[]){"NONE", NULL}},
    {.function = "PMIx_Allocate_resources",
     .attrs = (char *[]){"PMIX_ALLOC_REQ_ID",
                         "PMIX_ALLOC_NUM_NODES",
                         "PMIX_ALLOC_NODE_LIST",
                         "PMIX_ALLOC_NUM_CPUS",
                         "PMIX_ALLOC_NUM_CPU_LIST",
                         "PMIX_ALLOC_CPU_LIST",
                         "PMIX_ALLOC_MEM_SIZE",
                         "PMIX_ALLOC_TIME",
                         "PMIX_ALLOC_QUEUE",
                         "PMIX_ALLOC_PREEMPTIBLE",
                         NULL}},
#if PMIX_NUMERIC_VERSION >= 0x00050000
    {.function = "PMIx_Session_control",
     .attrs = (char *[]){"PMIX_SESSION_CTRL_ID",
                         "PMIX_SESSION_APP",
                         "PMIX_SESSION_PAUSE",
                         "PMIX_SESSION_RESUME",
                         "PMIX_SESSION_TERMINATE",
                         "PMIX_SESSION_PREEMPT",
                         "PMIX_SESSION_RESTORE",
                         "PMIX_SESSION_SIGNAL",
                         "PMIX_SESSION_COMPLETE",
                         NULL}},
#endif
    {.function = ""},
};


/* provide a callback function for lost connections to allow us
 * to cleanup after any tools once they depart */
static void lost_connection_hdlr(size_t evhdlr_registration_id, pmix_status_t status,
                                 const pmix_proc_t *source, pmix_info_t info[], size_t ninfo,
                                 pmix_info_t *results, size_t nresults,
                                 pmix_event_notification_cbfunc_fn_t cbfunc, void *cbdata)
{
    pmix_proclist_t *tl;
    DSCHED_HIDE_UNUSED_PARAMS(evhdlr_registration_id, status,
                              info, ninfo, results, nresults);

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s lost connection from tool %s",
                        PMIX_NAME_PRINT(&dsched_globals.myid),
                        PMIX_NAME_PRINT(source));

    /* scan the list of attached tools to see if this one is there */
    PMIX_LIST_FOREACH(tl, &dsched_globals.tools, pmix_proclist_t)
    {
        if (PMIx_Check_procid(&tl->proc, source)) {
            /* take this tool off the list */
            pmix_list_remove_item(&dsched_globals.tools, &tl->super);
            /* release it */
            PMIX_RELEASE(tl);
            break;
        }
    }

    /* we _always_ have to execute the evhandler callback or
     * else the event progress engine will hang */
    if (NULL != cbfunc) {
        cbfunc(PMIX_EVENT_ACTION_COMPLETE, NULL, 0, NULL, NULL, cbdata);
    }
}

static void regcbfunc(pmix_status_t status, size_t ref, void *cbdata)
{
    pmix_lock_t *lock = (pmix_lock_t *) cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(ref);

    PMIX_ACQUIRE_OBJECT(lock);
    lock->status = status;
    PMIX_WAKEUP_THREAD(lock);
}

/*
 * Initialize global variables used w/in the server.
 */
int dsched_server_init(pmix_cli_result_t *results)
{
    void *ilist;
    pmix_data_array_t darray;
    pmix_info_t *info;
    size_t n, ninfo;
    char *tmp;
    pmix_status_t rc;
    pmix_lock_t lock;
    DSCHED_HIDE_UNUSED_PARAMS(results);

    if (dsched_globals.server_initialized) {
        return PMIX_SUCCESS;
    }
    dsched_globals.server_initialized = true;

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s server:dsched: initialize",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    ilist = PMIx_Info_list_start();

    /* tell the server that we are the scheduler */
    rc = PMIx_Info_list_add(ilist, PMIX_SERVER_SCHEDULER,
                            NULL, PMIX_BOOL);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* tell the server our name so we agree on our identifier */
    rc = PMIx_Info_list_add(ilist, PMIX_TOOL_NSPACE,
                            dsched_globals.myid.nspace, PMIX_STRING);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }
    rc = PMIx_Info_list_add(ilist, PMIX_TOOL_RANK,
                            &dsched_globals.myid.rank, PMIX_PROC_RANK);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* tell the server our hostname so we agree on it */
    rc = PMIx_Info_list_add(ilist, PMIX_HOSTNAME,
                            dsched_globals.hostname, PMIX_STRING);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    // check for aliases
    if (NULL != dsched_globals.aliases) {
        tmp = PMIx_Argv_join(dsched_globals.aliases, ',');
        rc = PMIx_Info_list_add(ilist, PMIX_HOSTNAME_ALIASES,
                                tmp, PMIX_STRING);
        free(tmp);
        if (PMIX_SUCCESS != rc) {
            PMIx_Info_list_release(ilist);
            return rc;
        }
    }

    /* tell the server what we are doing with FQDN */
    rc = PMIx_Info_list_add(ilist, PMIX_HOSTNAME_KEEP_FQDN,
                            &dsched_globals.keep_fqdn_hostnames, PMIX_BOOL);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* give the server our event base to use for signal trapping */
    rc = PMIx_Info_list_add(ilist, PMIX_EXTERNAL_AUX_EVENT_BASE,
                            dsched_globals.evbase, PMIX_POINTER);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* tell the server our temp directory */
    rc = PMIx_Info_list_add(ilist, PMIX_SERVER_TMPDIR,
                            dsched_globals.tmpdir,
                            PMIX_STRING);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* if requested, tell the server library to output our PMIx URI */
    if (NULL != dsched_globals.report_uri) {
        rc = PMIx_Info_list_add(ilist, PMIX_TCP_REPORT_URI,
                                dsched_globals.report_uri, PMIX_STRING);
        if (PMIX_SUCCESS != rc) {
            PMIx_Info_list_release(ilist);
            return rc;
        }
    }

    if (NULL != dsched_globals.progress_thread_cpus) {
        rc = PMIx_Info_list_add(ilist, PMIX_BIND_PROGRESS_THREAD,
                           dsched_globals.progress_thread_cpus, PMIX_STRING);
        rc = PMIx_Info_list_add(ilist, PMIX_BIND_REQUIRED,
                           &dsched_globals.bind_progress_thread_reqd, PMIX_BOOL);
    }

    /* schedulers always allow remote tool connections */
    rc = PMIx_Info_list_add(ilist, PMIX_SERVER_REMOTE_CONNECTIONS,
                            NULL, PMIX_BOOL);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* if the system controller is present, connect to it */
    rc = PMIx_Info_list_add(ilist, PMIX_CONNECT_TO_SYS_CONTROLLER,
                            NULL, PMIX_BOOL);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* don't require the connection else we will abort
     * if the system controller isn't found */
    rc = PMIx_Info_list_add(ilist, PMIX_TOOL_CONNECT_OPTIONAL,
                            NULL, PMIX_BOOL);
    if (PMIX_SUCCESS != rc) {
        PMIx_Info_list_release(ilist);
        return rc;
    }

    /* convert to an info array */
    rc = PMIx_Info_list_convert(ilist, &darray);
    if (PMIX_SUCCESS != rc) {
        PMIX_INFO_LIST_RELEASE(ilist);
        return rc;
    }
    PMIx_Info_list_release(ilist);
    info = (pmix_info_t*)darray.array;
    ninfo = darray.size;

    /* initialize as a tool */
    rc = PMIx_tool_init(&dsched_globals.myid, info, ninfo);
    if (PMIX_SUCCESS != rc) {
        /* pmix will provide a nice show_help output here */
        PMIx_Info_free(info, ninfo);
        return rc;
    }
    PMIx_Info_free(info, ninfo);

    /* register our attributes */
    for (n = 0; 0 != strlen(dsched_attributes[n].function); n++) {
        rc = PMIx_Register_attributes(dsched_attributes[n].function, dsched_attributes[n].attrs);
        if (PMIX_SUCCESS != rc) {
            return rc;
        }
    }

    /* register the "lost-connection" event handler */
    PMIX_CONSTRUCT_LOCK(&lock);
    rc = PMIX_ERR_LOST_CONNECTION;
    PMIx_Register_event_handler(&rc, 1, NULL, 0, lost_connection_hdlr, regcbfunc, &lock);
    PMIX_WAIT_THREAD(&lock);
    rc = lock.status;
    PMIX_DESTRUCT_LOCK(&lock);
    if (PMIX_SUCCESS != rc) {
        return rc;
    }

    /* register our server function module */
    rc = PMIx_tool_set_server_module(&dsched_server);
    if (PMIX_SUCCESS != rc) {
        return rc;
    }

    return PMIX_SUCCESS;
}

void dsched_server_finalize(void)
{
    pmix_status_t rc;

    if (!dsched_globals.server_initialized) {
        return;
    }

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s Finalizing PMIX server",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    rc = PMIx_tool_finalize();
    if (PMIX_SUCCESS != rc) {
        PMIX_ERROR_LOG(rc);
    }
}

static void req_con(dsched_req_t *p)
{
    PMIx_Load_procid(&p->requestor, NULL, PMIX_RANK_INVALID);
    p->directive = 0;
    p->index = -1;
    p->copy = false;  // data is not a local copy
    p->data = NULL;
    p->ndata = 0;
    p->user_refid = NULL;
    p->alloc_refid = NULL;
    p->num_nodes = 0;
    p->nlist = NULL;
    p->exclude = NULL;
    p->num_cpus = 0;
    p->ncpulist = NULL;
    p->cpulist = NULL;
    p->memsize = 0.0;
    p->time = NULL;
    p->queue = NULL;
    p->preemptible = false;
    p->lend = NULL;
    p->image = NULL;
    p->waitall = false;
    p->share = false;
    p->noshell = false;
    p->dependency = NULL;
    p->begintime = NULL;
    p->sessionID = UINT32_MAX;
}
static void req_des(dsched_req_t *p)
{
    if (NULL != p->data && p->copy) {
        PMIx_Info_free(p->data, p->ndata);
    }
    if (NULL != p->user_refid) {
        free(p->user_refid);
    }
    if (NULL != p->alloc_refid) {
        free(p->alloc_refid);
    }
    if (NULL != p->nlist) {
        free(p->nlist);
    }
    if (NULL != p->exclude) {
        free(p->exclude);
    }
    if (NULL != p->ncpulist) {
        free(p->ncpulist);
    }
    if (NULL != p->cpulist) {
        free(p->cpulist);
    }
    if (NULL != p->time) {
        free(p->time);
    }
    if (NULL != p->queue) {
        free(p->queue);
    }
    if (NULL != p->lend) {
        free(p->lend);
    }
    if (NULL != p->image) {
        free(p->image);
    }
    if (NULL != p->dependency) {
        free(p->dependency);
    }
    if (NULL != p->begintime) {
        free(p->begintime);
    }
}
PMIX_CLASS_INSTANCE(dsched_req_t,
                    pmix_list_item_t,
                    req_con, req_des);
