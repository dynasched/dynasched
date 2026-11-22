/*
 * Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"
#include <limits.h>
#include <time.h>

#include "src/mca/preg/preg.h"

#include "src/event/event-internal.h"
#include "src/mca/dmeta/base/base.h"
#include "src/tools/dsched/dsched.h"

/* A directive that names a string is copied only if it carries one: any
 * other type, or a string with no value, is refused as an error in the
 * request, recording the first error in *rcerr. A key given twice keeps
 * its last value. */
static void take_string(const pmix_info_t *info, char **dest, pmix_status_t *rcerr)
{
    if (PMIX_STRING != info->value.type || NULL == info->value.data.string) {
        pmix_output_verbose(2, dsched_globals.pmix_output,
                            "%s scheduler:dsched: directive %s does not carry a string",
                            PMIX_NAME_PRINT(&dsched_globals.myid), info->key);
        if (PMIX_SUCCESS == *rcerr) {
            *rcerr = PMIX_ERR_BAD_PARAM;
        }
        return;
    }
    free(*dest);
    *dest = strdup(info->value.data.string);
}

/* A number a directive must carry, as a uint32 */
static void take_uint32(const pmix_info_t *info, uint32_t *dest, bool *given,
                        pmix_status_t *rcerr)
{
    if (PMIX_SUCCESS != PMIx_Value_get_number(&info->value, dest, PMIX_UINT32)) {
        if (PMIX_SUCCESS == *rcerr) {
            *rcerr = PMIX_ERR_BAD_PARAM;
        }
        return;
    }
    *given = true;
}

static void request_init(int fd, short args, void *cbdata)
{
    dsched_req_t *req = (dsched_req_t*)cbdata;
    size_t n;
    uint64_t u64;
    pmix_status_t rc, rcerr = PMIX_SUCCESS;
    bool notwaiting = false;
    DSCHED_HIDE_UNUSED_PARAMS(fd, args);

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s scheduler:dsched: init request",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    // process the incoming directives
    for (n=0; n < req->ndata; n++) {
        if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_REQ_ID)) {
            take_string(&req->data[n], &req->user_refid, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_ID)) {
            take_string(&req->data[n], &req->alloc_refid, &rcerr);

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
            rc = PMIx_Value_get_number(&req->data[n].value, &u64, PMIX_UINT64);
            if (PMIX_SUCCESS != rc) {
                PMIX_ERROR_LOG(rc);
                // track the first error
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = rc;
                }
                // continue processing as we may need some of the info
                // when reporting back the error
                continue;
            }
            if (INT_MAX < u64) {
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = PMIX_ERR_BAD_PARAM;
                }
                continue;
            }
            req->num_nodes = (int)u64;

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NODE_LIST)) {
            take_string(&req->data[n], &req->nlist, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_EXCLUDE)) {
            take_string(&req->data[n], &req->exclude, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NUM_CPUS)) {
            rc = PMIx_Value_get_number(&req->data[n].value, &u64, PMIX_UINT64);
            if (PMIX_SUCCESS != rc) {
                PMIX_ERROR_LOG(rc);
                // track the first error
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = rc;
                }
                // continue processing as we may need some of the info
                // when reporting back the error
                continue;
            }
            if (INT_MAX < u64) {
                if (PMIX_SUCCESS == rcerr) {
                    rcerr = PMIX_ERR_BAD_PARAM;
                }
                continue;
            }
            req->num_cpus = (int)u64;

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NUM_CPU_LIST)) {
            take_string(&req->data[n], &req->ncpulist, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_CPU_LIST)) {
            take_string(&req->data[n], &req->cpulist, &rcerr);

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
            take_string(&req->data[n], &req->time, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_QUEUE)) {
            take_string(&req->data[n], &req->queue, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_PREEMPTIBLE)) {
            req->preemptible = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_LEND)) {
            take_string(&req->data[n], &req->lend, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_IMAGE)) {
            take_string(&req->data[n], &req->image, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_WAIT_ALL_NODES)) {
            req->waitall = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_SHARE)) {
            req->share = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NOSHELL)) {
            req->noshell = PMIx_Value_true(&req->data[n].value);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_DEPENDENCY)) {
            take_string(&req->data[n], &req->dependency, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_BEGIN)) {
            take_string(&req->data[n], &req->begintime, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_NOT_WAITING)) {
            notwaiting = true;

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_USERID)) {
            /* PMIx stamps the requester's user and group on every request
             * it passes up, replacing any the requester supplied */
            if (PMIX_SUCCESS == PMIx_Value_get_number(&req->data[n].value, &req->uid,
                                                      PMIX_UINT32)) {
                req->have_uid = true;
            }

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_GRPID)) {
            (void) PMIx_Value_get_number(&req->data[n].value, &req->gid, PMIX_UINT32);

#ifdef PMIX_ALLOC_UID
        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_UID)) {
            take_uint32(&req->data[n], &req->alloc_uid, &req->have_alloc_uid, &rcerr);

        } else if (PMIX_CHECK_KEY(&req->data[n], PMIX_ALLOC_GID)) {
            take_uint32(&req->data[n], &req->alloc_gid, &req->have_alloc_gid, &rcerr);
#endif
        }
    }

    /* the hosts the request names, as names: a list, or a PMIx regex */
    if (PMIX_SUCCESS == rcerr && NULL != req->nlist &&
        PMIX_SUCCESS != pmix_preg.parse_nodes(req->nlist, &req->nodes_wanted)) {
        rcerr = PMIX_ERR_BAD_PARAM;
    }
    if (PMIX_SUCCESS == rcerr && NULL != req->exclude &&
        PMIX_SUCCESS != pmix_preg.parse_nodes(req->exclude, &req->nodes_excluded)) {
        rcerr = PMIX_ERR_BAD_PARAM;
    }
    if (PMIX_SUCCESS == rcerr && (0 > req->num_nodes || 0 > req->num_cpus)) {
        rcerr = PMIX_ERR_BAD_PARAM;
    }

    if (PMIX_SUCCESS != rcerr) {
        // inform them of the error, regardless of waiting or not
        dsched_reply(req, rcerr, NULL);
        // cannot continue processing the request
        PMIX_RELEASE(req);
        return;
    }

    switch (req->directive) {
    case PMIX_ALLOC_NEW:
        /* queue it - the queue holds the reference we were created
         * with - and if they are waiting, answer when it is decided */
        pmix_list_append(&dsched_globals.pending, &req->super);
        req->pending = true;
        if (notwaiting) {
            // need to reply to requestor so they don't hang. That is
            // the one reply the request gets
            dsched_reply(req, PMIX_SUCCESS, NULL);
        }
        dsched_schedule_requests();
        return;

    case PMIX_ALLOC_RELEASE:
        rc = dsched_release_allocation(req);
        dsched_reply(req, rc, NULL);
        PMIX_RELEASE(req);
        if (PMIX_SUCCESS == rc) {
            // the freed nodes may meet the request at the head of the queue
            dsched_schedule_requests();
        }
        return;

    default:
        // extending or reacquiring an allocation is not supported yet
        dsched_reply(req, PMIX_ERR_NOT_SUPPORTED, NULL);
        PMIX_RELEASE(req);
        return;
    }
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
    time(&req->received);
    DSCHED_THREADSHIFT(req, dsched_globals.evbase, request_init);
    return PMIX_SUCCESS;
}

static void session_control(int fd, short args, void *cbdata)
{
    dsched_req_t *req = (dsched_req_t*)cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(fd, args);

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s scheduler:dsched: session control",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    // no session control directive is supported yet - say so, rather
    // than leave the requestor waiting for a reply
    if (NULL != req->cbfunc) {
        req->cbfunc(PMIX_ERR_NOT_SUPPORTED, NULL, 0, req->cbdata, NULL, NULL);
    }
    PMIX_RELEASE(req);
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
