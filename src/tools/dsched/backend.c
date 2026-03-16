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
 * Copyright (c) 2020      IBM Corporation.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */

#include "dsched_config.h"

#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#include <fcntl.h>

#include "src/util/pmix_argv.h"
#include "src/util/pmix_name_fns.h"
#include "src/util/pmix_os_dirpath.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_show_help.h"
#include "src/threads/pmix_threads.h"

#include "src/include/dsched_globals.h"
#include "src/tools/dsched/dsched.h"

static void opcbfunc(pmix_status_t status, void *cbdata)
{
    pmix_lock_t *lock = (pmix_lock_t *) cbdata;

    lock->status = status;
    PMIX_WAKEUP_THREAD(lock);
}

/* add any info that the tool couldn't self-assign */
static int register_tool(pmix_nspace_t nspace)
{
    pmix_status_t ret;
    pmix_lock_t lock;
    pmix_proclist_t *tl;

    /* record this tool */
    tl = PMIX_NEW(pmix_proclist_t);
    PMIX_LOAD_PROCID(&tl->proc, nspace, 0);
    pmix_list_append(&dsched_globals.tools, &tl->super);

    PMIX_CONSTRUCT_LOCK(&lock);
    ret = PMIx_server_register_nspace(nspace, 1, NULL, 0,
                                      opcbfunc, &lock);
    if (PMIX_SUCCESS != ret) {
        PMIX_ERROR_LOG(ret);
        PMIX_DESTRUCT_LOCK(&lock);
        return ret;
    }
    PMIX_WAIT_THREAD(&lock);
    ret = lock.status;
    PMIX_DESTRUCT_LOCK(&lock);
    return ret;
}

static void _toolconn(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t *) cbdata;
    size_t n;
    pmix_status_t xrc;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    PMIX_ACQUIRE_OBJECT(cd);

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s PROCESSING TOOL CONNECTION",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    /* check for directives */
    if (NULL != cd->info) {
        for (n = 0; n < cd->ninfo; n++) {
            if (PMIX_CHECK_KEY(&cd->info[n], PMIX_EVENT_SILENT_TERMINATION)) {
                cd->flag = PMIX_INFO_TRUE(&cd->info[n]);

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_VERSION_INFO)) {
                /* we ignore this for now */

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_USERID)) {
                xrc = PMIx_Value_get_number(&cd->info[n].value, &cd->uid, PMIX_UINT32);
                if (PMIX_SUCCESS != xrc) {
                    if (NULL != cd->toolcbfunc) {
                        cd->toolcbfunc(xrc, NULL, cd->cbdata);
                    }
                    PMIX_RELEASE(cd);
                    return;
                }

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_GRPID)) {
                xrc = PMIx_Value_get_number(&cd->info[n].value, &cd->gid, PMIX_UINT32);
                if (PMIX_SUCCESS != xrc) {
                    if (NULL != cd->toolcbfunc) {
                        cd->toolcbfunc(xrc, NULL, cd->cbdata);
                    }
                    PMIX_RELEASE(cd);
                    return;
                }

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_NSPACE)) {
                PMIx_Load_nspace(cd->target.nspace, cd->info[n].value.data.string);

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_RANK)) {
                cd->target.rank = cd->info[n].value.data.rank;

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_HOSTNAME)) {
                cd->hostname = strdup(cd->info[n].value.data.string);

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_CMD_LINE)) {
                cd->cmdline = strdup(cd->info[n].value.data.string);

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_LAUNCHER)) {
                cd->launcher = PMIX_INFO_TRUE(&cd->info[n]);

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_SERVER_SYS_CONTROLLER)) {
                cd->scheduler = PMIX_INFO_TRUE(&cd->info[n]);

            } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_PROC_PID)) {
                xrc = PMIx_Value_get_number(&cd->info[n].value, &cd->pid, PMIX_PID);
                if (PMIX_SUCCESS != xrc) {
                    if (NULL != cd->toolcbfunc) {
                        cd->toolcbfunc(xrc, NULL, cd->cbdata);
                    }
                    PMIX_RELEASE(cd);
                    return;
                }
            }
        }
    }

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s %s CONNECTION FROM UID %d GID %d NSPACE %s",
                        PMIX_NAME_PRINT(&dsched_globals.myid),
                        cd->launcher ? "LAUNCHER" : (cd->scheduler ? "SYSTEM CONTROLLER" : "TOOL"),
                        cd->uid, cd->gid, cd->target.nspace);

    if (cd->scheduler) {
        /* mark that the system controller has attached to us */
        dsched_globals.controller_connected = true;
        PMIx_Load_procid(&dsched_globals.syscontroller,
                         cd->target.nspace, cd->target.rank);
        /* we cannot immediately set the system controller to be our
         * PMIx server as the PMIx library hasn't finished
         * recording it */
    }

    /* if the tool doesn't already have a self-assigned name, then
     * there isn't much we can do about it */
    xrc = PMIX_SUCCESS;
    if (PMIx_Nspace_invalid(cd->target.nspace) || PMIX_RANK_INVALID == cd->target.rank) {
        xrc = PMIX_ERR_BAD_PARAM;
    } else {
        /* the tool is not a client of ours, but we can provide at least some information */
        xrc = register_tool(cd->target.nspace);
    }
    if (NULL != cd->toolcbfunc) {
        cd->toolcbfunc(xrc, &cd->target, cd->cbdata);
    }
    PMIX_RELEASE(cd);
}

pmix_status_t dsched_tool_connected_fn(pmix_info_t *info, size_t ninfo,
                                       pmix_tool_connection_cbfunc_t cbfunc,
                                       void *cbdata)
{
    dsched_shift_caddy_t *cd;

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s TOOL CONNECTION REQUEST RECVD",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    /* need to threadshift this request */
    cd = PMIX_NEW(dsched_shift_caddy_t);
    cd->info = info;
    cd->ninfo = ninfo;
    cd->toolcbfunc = cbfunc;
    cd->cbdata = cbdata;
    cd->target.rank = 0; // set default for tool

    dsched_event_set(dsched_globals.evbase, &(cd->ev), -1, DSCHED_EV_WRITE, _toolconn, cd);
    PMIX_POST_OBJECT(cd);
    dsched_event_active(&(cd->ev), DSCHED_EV_WRITE, 1);

    return PMIX_SUCCESS;
}
