/*
 * Copyright (c) 2026      Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"

#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif

#include "src/event/event-internal.h"
#include "src/include/dsched_globals.h"
#include "src/tools/dsched/dsched.h"

/* Runs on dsched_globals.evbase. The only job control DynaSched acts on
 * is PMIX_JOB_CTRL_TERMINATE with no targets: stop the scheduler. It is
 * what dsterm sends, and it is the orderly way to stop dsched - a signal
 * is the abnormal one (see the signal handling in dsched.c). */
static void process_job_ctrl(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t *) cbdata;
    pmix_status_t rc = PMIX_ERR_NOT_SUPPORTED;
    bool terminate = false;
    bool uid_given = false;
    uint32_t uid = 0;
    size_t n;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    PMIX_ACQUIRE_OBJECT(cd);

    for (n = 0; NULL != cd->info && n < cd->ninfo; n++) {
        if (PMIX_CHECK_KEY(&cd->info[n], PMIX_JOB_CTRL_TERMINATE)) {
            terminate = PMIX_INFO_TRUE(&cd->info[n]);
        } else if (PMIX_CHECK_KEY(&cd->info[n], PMIX_USERID)) {
            uid_given = (PMIX_SUCCESS ==
                         PMIx_Value_get_number(&cd->info[n].value, &uid, PMIX_UINT32));
        }
    }

    if (terminate) {
        if (cd->flag) {
            /* DynaSched runs no jobs, so there is nothing a targeted
             * terminate could name - this directive means only the
             * scheduler itself */
            rc = PMIX_ERR_NOT_SUPPORTED;
        } else if (!uid_given || (0 != uid && (uint32_t) geteuid() != uid)) {
            /* PMIx stamps the requester's user on every request it passes
             * up, replacing any the requester supplied. Stopping the
             * scheduler is for root and the user it runs as. */
            rc = PMIX_ERR_NO_PERMISSIONS;
        } else {
            rc = PMIX_SUCCESS;
        }
    }

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s job control from %s: %s",
                        PMIX_NAME_PRINT(&dsched_globals.myid),
                        PMIX_NAME_PRINT(&cd->target),
                        (PMIX_SUCCESS == rc) ? "terminating" : PMIx_Error_string(rc));

    if (NULL != cd->infocbfunc) {
        cd->infocbfunc(rc, NULL, 0, cd->cbdata, NULL, NULL);
    }
    PMIX_RELEASE(cd);

    if (PMIX_SUCCESS == rc) {
        /* leave the event loop - main() then takes its normal way out,
         * finalizing the server, removing the session directory and any
         * pid file, and exiting 0. The acknowledgement just queued may
         * not reach the requester before the server finalizes; dsterm
         * treats the loss of the connection as the same answer. */
        dsched_globals.evactive = false;
        PMIX_POST_OBJECT(dsched_globals.evactive);
    }
}

/* PMIx calls this on its own progress thread - capture the request and
 * shift it to dsched_globals.evbase, where the work is done */
pmix_status_t dsched_job_ctrl_fn(const pmix_proc_t *requestor,
                                 const pmix_proc_t targets[], size_t ntargets,
                                 const pmix_info_t directives[], size_t ndirs,
                                 pmix_info_cbfunc_t cbfunc, void *cbdata)
{
    dsched_shift_caddy_t *cd;
    DSCHED_HIDE_UNUSED_PARAMS(targets);

    cd = PMIX_NEW(dsched_shift_caddy_t);
    if (NULL != requestor) {
        PMIx_Load_procid(&cd->target, requestor->nspace, requestor->rank);
    }
    /* whether the request named any targets - all that is needed of them */
    cd->flag = (0 < ntargets);
    /* PMIx keeps the directives until the callback is called */
    cd->info = (pmix_info_t *) directives;
    cd->ninfo = ndirs;
    cd->infocbfunc = cbfunc;
    cd->cbdata = cbdata;
    DSCHED_THREADSHIFT(cd, dsched_globals.evbase, process_job_ctrl);
    return PMIX_SUCCESS;
}
