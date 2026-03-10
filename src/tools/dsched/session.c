/*
 * Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"

#include "src/event/event-internal.h"

#include "src/tools/dsched/dsched.h"

static void request_init(int fd, short args, void *cbdata)
{
    dsched_req_t *req = (dsched_req_t*)cbdata;
    size_t n;
    pmix_status_t rc, rcerr = PMIX_SUCCESS;
    bool notwaiting = false;
    DSCHED_HIDE_UNUSED_PARAMS(fd, args);

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s scheduler:dsched: init request",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    // process the incoming directives
    for (n=0; n < req->ndata; n++) {
        if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_REQ_ID)) {
            req->user_refid = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_ID)) {
            req->alloc_refid = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_SESSION_ID)) {
            rc = PMIx_Value_get_number(&req->data[n].value, &req->sessionID, PMIX_UINT32);
            if (PMIX_SUCCESS != rc) {
                PMIX_ERROR_LOG(rc);
                // track the first error
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = rc;
                }
            }
            // continue processing as we may need some of the info
            // when reporting back the error

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NUM_NODES)) {
            rc = PMIx_Value_get_number(&req->data[n].value, &req->num_nodes, PMIX_UINT64);
            if (PMIX_SUCCESS != rc) {
                PMIX_ERROR_LOG(rc);
                // track the first error
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = rc;
                }
                // continue processing as we may need some of the info
                // when reporting back the error
            }

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NODE_LIST)) {
            req->nlist = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_EXCLUDE)) {
            req->exclude = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NUM_CPUS)) {
            rc = PMIx_Value_get_number(&req->data[n].value, &req->num_cpus, PMIX_UINT64);
            if (PMIX_SUCCESS != rc) {
                PMIX_ERROR_LOG(rc);
                // track the first error
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = rc;
                }
                // continue processing as we may need some of the info
                // when reporting back the error
            }

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NUM_CPU_LIST)) {
            req->ncpulist = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_CPU_LIST)) {
            req->cpulist = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_MEM_SIZE)) {
            rc = PMIx_Value_get_number(&req->data[n].value, &req->memsize, PMIX_FLOAT);
            if (PMIX_SUCCESS != rc) {
                PMIX_ERROR_LOG(rc);
                // track the first error
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = rc;
                }
                // continue processing as we may need some of the info
                // when reporting back the error
            }

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_TIME)) {
            req->time = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_QUEUE)) {
            req->queue = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_PREEMPTIBLE)) {
            req->preemptible = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_LEND)) {
            req->lend = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_IMAGE)) {
            req->image = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_WAIT_ALL_NODES)) {
            req->waitall = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_SHARE)) {
            req->share = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NOSHELL)) {
            req->noshell = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_DEPENDENCY)) {
            req->dependency = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_BEGIN)) {
            req->begintime = strdup(req->data[n].value.data.string);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NOT_WAITING)) {
            notwaiting = true;
        }
    }
    if (notwaiting) {
        // we callback with the current status so the requestor
        // can be told if we are accepting the request
        if (NULL != req->cbfunc) {
            req->cbfunc(rcerr, NULL, 0, req->cbdata, NULL, NULL);
        }
        if (PMIX_SUCCESS == rcerr) {
            // add request to the queue
            req->index = pmix_pointer_array_add(&dsched_globals.requests, req);
        } else {
            PMIX_RELEASE(req);
        }
    } else if (PMIX_SUCCESS == rcerr) {
            // add request to the queue
            req->index = pmix_pointer_array_add(&dsched_globals.requests, req);
    } else {
        // need to reply to requestor so they don't hang
        if (NULL != req->cbfunc) {
            req->cbfunc(rcerr, NULL, 0, req->cbdata, NULL, NULL);
        }
        // cannot continue processing the request
        PMIX_RELEASE(req);
    }
    return;
}


pmix_status_t dsched_alloc_fn(const pmix_proc_t *client,
                              pmix_alloc_directive_t directive,
                              const pmix_info_t data[], size_t ndata,
                              pmix_info_cbfunc_t cbfunc, void *cbdata)
{
    dsched_req_t *req;

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s allocate upcalled on behalf of proc %s with %" PRIsize_t " infos",
                        PMIX_NAME_PRINT(&dsched_globals.myid), PMIX_NAME_PRINT(client), ndata);

    req = PMIX_NEW(dsched_req_t);
    PMIx_Load_procid(&req->requestor, client->nspace, client->rank);
    req->directive = directive;
    req->data = (pmix_info_t *) data;
    req->ndata = ndata;
    req->cbfunc = cbfunc;
    req->cbdata = cbdata;
    DSCHED_THREADSHIFT(req, dsched_globals.evbase, request_init);
    return PMIX_SUCCESS;
}

static void session_control(int fd, short args, void *cbdata)
{
    dsched_req_t *req = (dsched_req_t*)cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(fd, args, req);

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s scheduler:dsched: session control",
                        PMIX_NAME_PRINT(&dsched_globals.myid));
}

pmix_status_t dsched_session_ctrl_fn(const pmix_proc_t *requestor,
                                     uint32_t sessionID,
                                     const pmix_info_t directives[], size_t ndirs,
                                     pmix_info_cbfunc_t cbfunc, void *cbdata)
{
    dsched_req_t *req;


    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s session ctrl upcalled on behalf of proc %s with %" PRIsize_t " directives",
                        PMIX_NAME_PRINT(&dsched_globals.myid), PMIX_NAME_PRINT(requestor), ndirs);

    req = PMIX_NEW(dsched_req_t);
    PMIx_Load_procid(&req->requestor, requestor->nspace, requestor->rank);
    req->sessionID = sessionID;
    req->data = (pmix_info_t *) directives;
    req->ndata = ndirs;
    req->cbfunc = cbfunc;
    req->cbdata = cbdata;
    DSCHED_THREADSHIFT(req, dsched_globals.evbase, session_control);
    return PMIX_SUCCESS;
}
