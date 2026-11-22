/*
 * Copyright (c) 2026      Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/*
 * Scheduling rounds, and what they grant.
 *
 * Allocation requests wait on dsched_globals.pending, oldest first. A
 * round hands the schedulers a snapshot of the pool and of that queue;
 * they propose, the meta component chooses one proposal, and the round
 * applies it here: a grant takes the nodes and answers the requester
 * with them, and a refusal answers it with the reason. A round that did
 * either runs another, since the next request may now be decided too.
 * Only one round runs at a time, so a proposal is always made from the
 * state it is applied to.
 *
 * Everything here runs on dsched_globals.evbase.
 */

#include "dsched_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/util/pmix_argv.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_printf.h"

#include "src/mca/dmeta/base/base.h"
#include "src/tools/dsched/dsched.h"

static void reply_release(void *cbdata)
{
    pmix_data_array_t *d = (pmix_data_array_t *) cbdata;

    PMIx_Data_array_destruct(d);
    free(d);
}

/* Answer a request, with the info list ilist if one is given (it is
 * consumed), and make sure it is never answered again: PMIx releases
 * its tracker, and the directives with it, once the answer is made. */
void dsched_reply(dsched_req_t *req, pmix_status_t status, void *ilist)
{
    pmix_data_array_t *d = NULL;
    pmix_status_t rc;

    if (NULL == req->cbfunc) {
        if (NULL != ilist) {
            PMIx_Info_list_release(ilist);
        }
        return;
    }
    if (NULL != ilist) {
        d = (pmix_data_array_t *) calloc(1, sizeof(pmix_data_array_t));
        if (NULL == d) {
            status = PMIX_ERR_NOMEM;
        } else if (PMIX_SUCCESS != (rc = PMIx_Info_list_convert(ilist, d))) {
            free(d);
            d = NULL;
            status = rc;
        }
        PMIx_Info_list_release(ilist);
    }
    if (NULL != d) {
        req->cbfunc(status, (pmix_info_t *) d->array, d->size, req->cbdata, reply_release, d);
    } else {
        req->cbfunc(status, NULL, 0, req->cbdata, NULL, NULL);
    }
    req->cbfunc = NULL;
    req->cbdata = NULL;
    req->data = NULL;
    req->ndata = 0;
}

/* take a request off the queue, dropping the queue's reference */
static void dequeue(dsched_req_t *req)
{
    if (req->pending) {
        pmix_list_remove_item(&dsched_globals.pending, &req->super);
        req->pending = false;
        PMIX_RELEASE(req);
    }
}

/* Apply a grant: the request gets the proposal's nodes. Returns false,
 * changing nothing, if one of them is no longer free - which a round
 * run on the state it applies to cannot propose. */
static bool grant(dsched_req_t *req, dsched_alloc_t *alloc)
{
    dsched_session_t *session;
    dsched_node_t *node;
    char **names = NULL, *list;
    void *ilist;
    uint64_t nnodes = 0, ncpus = 0;
    int n;

    for (n = 0; n < alloc->allocation.size; n++) {
        node = (dsched_node_t *) pmix_pointer_array_get_item(&alloc->allocation, n);
        if (NULL != node && NULL != node->session) {
            pmix_output_verbose(2, dsched_globals.output,
                                "%s proposal for request %s names allocated node %s - not applied",
                                PMIX_NAME_PRINT(&dsched_globals.myid),
                                PMIX_NAME_PRINT(&req->requestor), node->name);
            return false;
        }
    }

    session = PMIX_NEW(dsched_session_t);
    session->session_id = dsched_globals.next_session_id++;
    if (UINT32_MAX == dsched_globals.next_session_id) {
        // UINT32_MAX means "no session" - never hand it out
        dsched_globals.next_session_id = 1;
    }
    pmix_asprintf(&session->alloc_refid, "%s.%u", dsched_globals.myid.nspace,
                  dsched_globals.next_alloc_id++);
    if (NULL != req->user_refid) {
        session->user_refid = strdup(req->user_refid);
    }
    PMIx_Load_procid(&session->requestor, req->requestor.nspace, req->requestor.rank);
    session->uid = req->uid;
    session->gid = req->gid;
    for (n = 0; n < alloc->allocation.size; n++) {
        node = (dsched_node_t *) pmix_pointer_array_get_item(&alloc->allocation, n);
        if (NULL == node) {
            continue;
        }
        PMIX_RETAIN(node);
        pmix_pointer_array_add(&session->nodes, node);
        node->session = session;
        PMIx_Argv_append_nosize(&names, node->name);
        ++nnodes;
        ncpus += (uint64_t) node->slots;
    }
    session->index = pmix_pointer_array_add(&dsched_globals.sessions, session);

    list = PMIx_Argv_join(names, ',');
    PMIx_Argv_free(names);
    pmix_output_verbose(1, dsched_globals.output,
                        "%s granted %s to %s: %s",
                        PMIX_NAME_PRINT(&dsched_globals.myid), session->alloc_refid,
                        PMIX_NAME_PRINT(&req->requestor), (NULL == list) ? "" : list);

    /* the answer: what was granted, and how to name it */
    ilist = PMIx_Info_list_start();
    PMIx_Info_list_add(ilist, PMIX_ALLOC_ID, session->alloc_refid, PMIX_STRING);
    PMIx_Info_list_add(ilist, PMIX_SESSION_ID, &session->session_id, PMIX_UINT32);
    PMIx_Info_list_add(ilist, PMIX_ALLOC_NODE_LIST, list, PMIX_STRING);
    PMIx_Info_list_add(ilist, PMIX_ALLOC_NUM_NODES, &nnodes, PMIX_UINT64);
    PMIx_Info_list_add(ilist, PMIX_ALLOC_NUM_CPUS, &ncpus, PMIX_UINT64);
    if (NULL != session->user_refid) {
        PMIx_Info_list_add(ilist, PMIX_ALLOC_REQ_ID, session->user_refid, PMIX_STRING);
    }
#ifdef PMIX_ALLOC_UID
    /* whom the allocation is for, if the request said: a requester
     * acting for someone else checks that we agree */
    if (req->have_alloc_uid) {
        PMIx_Info_list_add(ilist, PMIX_ALLOC_UID, &req->alloc_uid, PMIX_UINT32);
    }
    if (req->have_alloc_gid) {
        PMIx_Info_list_add(ilist, PMIX_ALLOC_GID, &req->alloc_gid, PMIX_UINT32);
    }
#endif
    free(list);

    dsched_reply(req, PMIX_SUCCESS, ilist);
    dequeue(req);
    return true;
}

/* the round is over - apply what it chose, and run another if that
 * may decide the next request */
static void round_done(dsched_meta_t *mt)
{
    dsched_alloc_t *alloc = mt->alloc;
    dsched_req_t *req;
    bool decided = false;

    dsched_globals.round_active = false;

    /* the request may have left the queue while the round ran - its
     * requester went away - and then there is nothing to apply */
    if (NULL != alloc && NULL != alloc->req && alloc->req->pending) {
        req = alloc->req;
        if (PMIX_SUCCESS == alloc->status) {
            decided = grant(req, alloc);
            if (!decided) {
                dsched_globals.round_again = true;
            }
        } else {
            pmix_output_verbose(1, dsched_globals.output,
                                "%s refused a request from %s: %s",
                                PMIX_NAME_PRINT(&dsched_globals.myid),
                                PMIX_NAME_PRINT(&req->requestor),
                                PMIx_Error_string(alloc->status));
            dsched_reply(req, alloc->status, NULL);
            dequeue(req);
            decided = true;
        }
    }
    PMIX_RELEASE(mt);

    if (decided || dsched_globals.round_again) {
        dsched_globals.round_again = false;
        dsched_schedule_requests();
    }
}

/* Start a round, if there is anything to decide and no round running.
 * A round asked for while one runs is run after it. */
void dsched_schedule_requests(void)
{
    dsched_meta_t *mt;
    dsched_node_t *node;
    dsched_req_t *req;
    int n, cnt;

    if (dsched_globals.round_active) {
        dsched_globals.round_again = true;
        return;
    }
    if (0 == pmix_list_get_size(&dsched_globals.pending)) {
        return;
    }

    mt = PMIX_NEW(dsched_meta_t);
    mt->cbfunc = round_done;

    /* the pool, as the round sees it */
    cnt = 0;
    for (n = 0; n < dsched_globals.nodes.size; n++) {
        if (NULL != pmix_pointer_array_get_item(&dsched_globals.nodes, n)) {
            ++cnt;
        }
    }
    if (0 < cnt) {
        mt->pool = (dsched_node_snap_t *) calloc(cnt, sizeof(dsched_node_snap_t));
    }
    for (n = 0; NULL != mt->pool && n < dsched_globals.nodes.size; n++) {
        node = (dsched_node_t *) pmix_pointer_array_get_item(&dsched_globals.nodes, n);
        if (NULL == node) {
            continue;
        }
        PMIX_RETAIN(node);
        mt->pool[mt->npool].node = node;
        mt->pool[mt->npool].slots = node->slots;
        mt->pool[mt->npool].free = (NULL == node->session);
        mt->npool++;
    }

    /* the pending requests, oldest first */
    cnt = (int) pmix_list_get_size(&dsched_globals.pending);
    mt->pending = (dsched_req_t **) calloc(cnt, sizeof(dsched_req_t *));
    if (NULL != mt->pending) {
        PMIX_LIST_FOREACH(req, &dsched_globals.pending, dsched_req_t) {
            PMIX_RETAIN(req);
            mt->pending[mt->npending++] = req;
        }
    }

    dsched_globals.round_active = true;
    dsched_meta_base_schedule(mt);
}

static bool name_matches(const dsched_node_t *node, const char *name)
{
    int m;

    if (0 == strcmp(node->name, name)) {
        return true;
    }
    for (m = 0; NULL != node->aliases && NULL != node->aliases[m]; m++) {
        if (0 == strcmp(node->aliases[m], name)) {
            return true;
        }
    }
    return false;
}

/* Release an allocation, or some of its nodes: the request names it by
 * PMIX_ALLOC_ID or PMIX_SESSION_ID, and names the nodes to release with
 * PMIX_ALLOC_NODE_LIST, else all of them. Only root and the user who
 * was granted the allocation may release it. */
pmix_status_t dsched_release_allocation(dsched_req_t *req)
{
    dsched_session_t *session = NULL, *s;
    dsched_node_t *node;
    const char *name;
    int n, m, nleft;

    for (n = 0; NULL == session && n < dsched_globals.sessions.size; n++) {
        s = (dsched_session_t *) pmix_pointer_array_get_item(&dsched_globals.sessions, n);
        if (NULL == s) {
            continue;
        }
        if (NULL != req->alloc_refid) {
            if (0 == strcmp(s->alloc_refid, req->alloc_refid)) {
                session = s;
            }
        } else if (UINT32_MAX != req->sessionID && s->session_id == req->sessionID) {
            session = s;
        }
    }
    if (NULL == session) {
        return (NULL == req->alloc_refid && UINT32_MAX == req->sessionID) ? PMIX_ERR_BAD_PARAM
                                                                        : PMIX_ERR_NOT_FOUND;
    }
    if (!req->have_uid || (0 != req->uid && req->uid != session->uid)) {
        return PMIX_ERR_NO_PERMISSIONS;
    }

    /* every node named must be in the allocation - check them all before
     * releasing any */
    for (m = 0; NULL != req->nodes_wanted && NULL != req->nodes_wanted[m]; m++) {
        name = dsched_check_host_is_local(req->nodes_wanted[m]) ? dsched_globals.hostname
                                                                : req->nodes_wanted[m];
        for (n = 0; n < session->nodes.size; n++) {
            node = (dsched_node_t *) pmix_pointer_array_get_item(&session->nodes, n);
            if (NULL != node && name_matches(node, name)) {
                break;
            }
        }
        if (n == session->nodes.size) {
            return PMIX_ERR_BAD_PARAM;
        }
    }

    nleft = 0;
    for (n = 0; n < session->nodes.size; n++) {
        node = (dsched_node_t *) pmix_pointer_array_get_item(&session->nodes, n);
        if (NULL == node) {
            continue;
        }
        if (NULL != req->nodes_wanted) {
            for (m = 0; NULL != req->nodes_wanted[m]; m++) {
                name = dsched_check_host_is_local(req->nodes_wanted[m]) ? dsched_globals.hostname
                                                                        : req->nodes_wanted[m];
                if (name_matches(node, name)) {
                    break;
                }
            }
            if (NULL == req->nodes_wanted[m]) {
                ++nleft;
                continue;
            }
        }
        pmix_output_verbose(2, dsched_globals.output,
                            "%s released %s from %s",
                            PMIX_NAME_PRINT(&dsched_globals.myid), node->name,
                            session->alloc_refid);
        pmix_pointer_array_set_item(&session->nodes, n, NULL);
        node->session = NULL;
        PMIX_RELEASE(node);
    }
    pmix_output_verbose(1, dsched_globals.output,
                        "%s released %s%s",
                        PMIX_NAME_PRINT(&dsched_globals.myid), session->alloc_refid,
                        (0 < nleft) ? " in part" : "");
    if (0 == nleft) {
        pmix_pointer_array_set_item(&dsched_globals.sessions, session->index, NULL);
        PMIX_RELEASE(session);
    }
    return PMIX_SUCCESS;
}

/* A requester has gone. Its requests that are still waiting for an
 * answer can no longer be given one, so they leave the queue - answered
 * all the same, which is what releases PMIx's hold on them. A request
 * that was not waiting stays: its requester was never going to hear.
 * What was granted stays granted until it is released. */
void dsched_drop_requests(const pmix_proc_t *requestor)
{
    dsched_req_t *req, *next;
    bool dropped = false;

    PMIX_LIST_FOREACH_SAFE(req, next, &dsched_globals.pending, dsched_req_t) {
        if (NULL != req->cbfunc && PMIx_Check_procid(&req->requestor, requestor)) {
            pmix_output_verbose(1, dsched_globals.output,
                                "%s dropped a request from departed %s",
                                PMIX_NAME_PRINT(&dsched_globals.myid),
                                PMIX_NAME_PRINT(requestor));
            dsched_reply(req, PMIX_ERR_LOST_CONNECTION, NULL);
            dequeue(req);
            dropped = true;
        }
    }
    if (dropped) {
        // the request that was holding up the queue may have gone
        dsched_schedule_requests();
    }
}
