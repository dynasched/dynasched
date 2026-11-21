/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2026      Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/*
 * dsterm - stop a running dsched.
 *
 * Connects to dsched as a PMIx tool and asks it to terminate with
 * PMIx_Job_control_nb(PMIX_JOB_CTRL_TERMINATE) and no targets - the
 * request dsched's job control handler (src/tools/dsched/job_ctrl.c)
 * answers by leaving its event loop and exiting 0. dsterm then waits for
 * its connection to dsched to drop, which is how it knows dsched has
 * actually gone rather than merely heard the request.
 */

#include "dsched_config.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif

#include "pmix_tool.h"

#include "src/threads/pmix_mutex.h"
#include "src/threads/pmix_threads.h"
#include "src/util/pmix_argv.h"
#include "src/util/pmix_basename.h"
#include "src/util/pmix_cmd_line.h"
#include "src/util/pmix_fd.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_printf.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_constants.h"
#include "src/event/event-internal.h"
#include "src/include/dsched_globals.h"
#include "src/runtime/dsched_progress_threads.h"
#include "src/runtime/dsched_rte.h"
#include "src/util/dsched_cmd_line.h"

static bool verbose = false;
static bool forcibly_die = false;
static dsched_event_t term_handler;
static int term_pipe[2];
static pmix_mutex_t dsterm_abort_inprogress_lock = PMIX_MUTEX_STATIC_INIT;
static dsched_event_base_t *myevbase = NULL;

static void abort_signal_callback(int signal);
static void clean_abort(int fd, short flags, void *arg);

static struct option dstermoptions[] = {
    PMIX_OPTION_SHORT_DEFINE(DSCHED_CLI_HELP, PMIX_ARG_OPTIONAL, 'h'),
    PMIX_OPTION_SHORT_DEFINE(DSCHED_CLI_VERSION, PMIX_ARG_NONE, 'V'),
    PMIX_OPTION_SHORT_DEFINE(DSCHED_CLI_VERBOSE, PMIX_ARG_NONE, 'v'),

    PMIX_OPTION_DEFINE(DSCHED_CLI_PMIXMCA, PMIX_ARG_REQD),

    PMIX_OPTION_DEFINE(DSCHED_CLI_URI, PMIX_ARG_REQD),

    /* End of list */
    PMIX_OPTION_END
};
static char *dstermshorts = "h::vV";

/* dsched's answer to the request - it may have refused */
static void infocb(pmix_status_t status, pmix_info_t *info, size_t ninfo, void *cbdata,
                   pmix_release_cbfunc_t release_fn, void *release_cbdata)
{
    pmix_lock_t *lock = (pmix_lock_t *) cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(info, ninfo);

    lock->status = status;
    if (NULL != release_fn) {
        release_fn(release_cbdata);
    }
    /* This runs on the PMIx progress thread. The waiter blocks on the
     * lock's condition variable and drives no event base, so the direct
     * wakeup is correct here (see the thread rules in AGENTS.md). */
    PMIX_WAKEUP_THREAD(lock);
}

static void regcbfunc(pmix_status_t status, size_t ref, void *cbdata)
{
    pmix_lock_t *lock = (pmix_lock_t *) cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(ref);

    lock->status = status;
    PMIX_WAKEUP_THREAD(lock);
}

/* we are registered only for the loss of our connection to dsched, which
 * is how we learn that it has gone */
static void evhandler(size_t evhdlr_registration_id, pmix_status_t status,
                      const pmix_proc_t *source, pmix_info_t info[], size_t ninfo,
                      pmix_info_t *results, size_t nresults,
                      pmix_event_notification_cbfunc_fn_t cbfunc, void *cbdata)
{
    pmix_lock_t *lock = NULL;
    size_t n;
    DSCHED_HIDE_UNUSED_PARAMS(evhdlr_registration_id, source, results, nresults);

    if (verbose) {
        pmix_output(0, "%s: connection event %s", dsched_globals.basename,
                    PMIx_Error_string(status));
    }
    for (n = 0; NULL != info && n < ninfo; n++) {
        if (PMIX_CHECK_KEY(&info[n], PMIX_EVENT_RETURN_OBJECT)) {
            lock = (pmix_lock_t *) info[n].value.data.ptr;
        }
    }
    if (NULL != lock) {
        lock->status = PMIX_SUCCESS;
        /* as in infocb: the waiter is on a condition variable */
        PMIX_WAKEUP_THREAD(lock);
    }

    /* we _always_ have to execute the evhandler callback or
     * else the event progress engine will hang */
    if (NULL != cbfunc) {
        cbfunc(PMIX_EVENT_ACTION_COMPLETE, NULL, 0, NULL, NULL, cbdata);
    }
}

int main(int argc, char *argv[])
{
    pmix_status_t rc, code[2] = {PMIX_ERR_UNREACH, PMIX_ERR_LOST_CONNECTION};
    pmix_cli_result_t results;
    pmix_cli_item_t *opt;
    pmix_lock_t lock, rellock;
    pmix_info_t *iptr, info;
    pmix_proc_t myproc;
    pmix_data_array_t darray;
    pmix_rank_t rank = 0;
    void *ilist;
    size_t ninfo;
    char hostname[PMIX_MAXHOSTNAMELEN] = {0};
    char *str, *ptr;
    bool flag = true;
    int ret;
    DSCHED_HIDE_UNUSED_PARAMS(argc);

    /* Every failure exits with 1: main() hands the shell only the low
     * eight bits of what it returns, so a DynaSched or PMIx error code
     * would arrive as a meaningless 251 or 255. */

    dsched_globals.basename = pmix_basename(argv[0]);
    if (DSCHED_SUCCESS != (ret = dsched_init_util())) {
        fprintf(stderr, "%s: dsched_init_util failed: %s\n", dsched_globals.basename,
                PMIx_Error_string(ret));
        return 1;
    }

    /* parse the input argv */
    PMIX_CONSTRUCT(&results, pmix_cli_result_t);
    rc = pmix_cmd_line_parse(argv, dstermshorts, dstermoptions, NULL,
                             &results, "help-dsterm.txt");
    if (PMIX_SUCCESS != rc) {
        PMIX_DESTRUCT(&results);
        if (PMIX_OPERATION_SUCCEEDED == rc) {
            /* --help or --version */
            return 0;
        }
        if (PMIX_ERR_SILENT != rc) {
            fprintf(stderr, "%s: command line error (%s)\n", dsched_globals.basename,
                    PMIx_Error_string(rc));
        }
        return 1;
    }

    // we do NOT accept arguments other than our own
    if (NULL != results.tail) {
        str = PMIx_Argv_join(results.tail, ' ');
        if (0 != strcmp(str, argv[0])) {
            ptr = pmix_show_help_string("help-dsterm.txt", "no-args", false,
                                        dsched_globals.basename, str, dsched_globals.basename);
            if (NULL != ptr) {
                printf("%s", ptr);
                free(ptr);
            }
            free(str);
            return 1;
        }
        free(str);
    }

    verbose = pmix_cmd_line_is_taken(&results, DSCHED_CLI_VERBOSE);

    /* pass along any PMIx MCA parameters */
    dsched_process_cli_mca_params(&results);

    ilist = PMIx_Info_list_start();

    /* dsched accepts only tools that name themselves */
    gethostname(hostname, sizeof(hostname) - 1);
    pmix_asprintf(&str, "%s.%s.%lu", dsched_globals.basename, hostname,
                  (unsigned long) getpid());
    PMIx_Info_list_add(ilist, PMIX_TOOL_NSPACE, str, PMIX_STRING);
    free(str);
    PMIx_Info_list_add(ilist, PMIX_TOOL_RANK, &rank, PMIX_PROC_RANK);

    /* either the dsched we were pointed at, or the scheduler PMIx
     * advertises on this host */
    opt = pmix_cmd_line_get_param(&results, DSCHED_CLI_URI);
    if (NULL != opt && NULL != opt->values && NULL != opt->values[0]) {
        PMIx_Info_list_add(ilist, PMIX_SERVER_URI, opt->values[0], PMIX_STRING);
    } else {
        PMIx_Info_list_add(ilist, PMIX_CONNECT_TO_SCHEDULER, &flag, PMIX_BOOL);
    }

    rc = PMIx_Info_list_convert(ilist, &darray);
    PMIx_Info_list_release(ilist);
    if (PMIX_SUCCESS != rc) {
        fprintf(stderr, "%s: %s\n", dsched_globals.basename, PMIx_Error_string(rc));
        return 1;
    }
    iptr = (pmix_info_t *) darray.array;
    ninfo = darray.size;

    /* Setup callbacks for abort signals, as pterm does. From this point
     * forward we need to abort in a manner that allows us to cleanup, but
     * we cannot directly use libevent to trap these signals, as otherwise
     * we could not respond to them while stuck in an event. So use the
     * basic POSIX trap functions to handle the signal, and have the
     * handler poke a pipe tied to an event on a progress thread - the
     * "safe" place where the termination can actually be done.
     *
     * NOTE: posix traps don't allow us to do anything major in them. */
    if (0 != pipe(term_pipe)) {
        PMIx_Info_free(iptr, ninfo);
        return 1;
    }
    myevbase = dsched_progress_thread_init(NULL);
    if (NULL == myevbase) {
        PMIx_Info_free(iptr, ninfo);
        return 1;
    }
    dsched_event_set(myevbase, &term_handler, term_pipe[0], DSCHED_EV_READ, clean_abort, NULL);
    dsched_event_add(&term_handler, NULL);

    /* Set both ends of this pipe to be close-on-exec so that no
     * children inherit it */
    if (PMIX_SUCCESS != pmix_fd_set_cloexec(term_pipe[0]) ||
        PMIX_SUCCESS != pmix_fd_set_cloexec(term_pipe[1])) {
        fprintf(stderr, "%s: unable to set the pipe to CLOEXEC\n", dsched_globals.basename);
        dsched_progress_thread_finalize(NULL);
        PMIx_Info_free(iptr, ninfo);
        return 1;
    }

    /* point the signal trap to a function that will activate that event */
    signal(SIGTERM, abort_signal_callback);
    signal(SIGINT, abort_signal_callback);
    signal(SIGHUP, abort_signal_callback);

    rc = PMIx_tool_init(&myproc, iptr, ninfo);
    PMIx_Info_free(iptr, ninfo);
    if (PMIX_SUCCESS != rc) {
        /* say what PMIx said - "no scheduler running" is only the most
         * common reason, and a wrong URI looks the same otherwise */
        fprintf(stderr, "%s: unable to connect to dsched: %s\n", dsched_globals.basename,
                PMIx_Error_string(rc));
        return 1;
    }

    /* trap the loss of the connection, before asking for it */
    PMIX_CONSTRUCT_LOCK(&rellock);
    PMIX_CONSTRUCT_LOCK(&lock);
    PMIx_Info_load(&info, PMIX_EVENT_RETURN_OBJECT, &rellock, PMIX_POINTER);
    PMIx_Register_event_handler(code, 2, &info, 1, evhandler, regcbfunc, &lock);
    PMIX_WAIT_THREAD(&lock);
    rc = lock.status;
    PMIX_DESTRUCT_LOCK(&lock);
    PMIx_Info_destruct(&info);
    if (PMIX_SUCCESS != rc) {
        fprintf(stderr, "%s: unable to register for connection events: %s\n",
                dsched_globals.basename, PMIx_Error_string(rc));
        PMIX_DESTRUCT_LOCK(&rellock);
        PMIx_tool_finalize();
        return 1;
    }

    PMIx_Info_load(&info, PMIX_JOB_CTRL_TERMINATE, &flag, PMIX_BOOL);
    PMIX_CONSTRUCT_LOCK(&lock);
    rc = PMIx_Job_control_nb(NULL, 0, &info, 1, infocb, &lock);
    PMIx_Info_destruct(&info);
    if (PMIX_SUCCESS != rc) {
        /* PMIx did not take the request, so infocb will never be called */
        PMIX_DESTRUCT_LOCK(&lock);
        PMIX_DESTRUCT_LOCK(&rellock);
        fprintf(stderr, "%s: request refused: %s\n", dsched_globals.basename,
                PMIx_Error_string(rc));
        PMIx_tool_finalize();
        return 1;
    }
    PMIX_WAIT_THREAD(&lock);
    rc = lock.status;
    PMIX_DESTRUCT_LOCK(&lock);

    /* dsched answers and then exits, and a PMIx server that finalizes
     * before the queued answer is written drops it - so the request can
     * fail with the connection, as though dsched had refused. It went
     * away because we told it to, which is what we wait for below: losing
     * the connection is how we know it is gone. */
    if (PMIX_ERR_COMM_FAILURE == rc || PMIX_ERR_LOST_CONNECTION == rc ||
        PMIX_ERR_UNREACH == rc) {
        if (verbose) {
            pmix_output(0, "%s: dsched went away before acknowledging (%s)",
                        dsched_globals.basename, PMIx_Error_string(rc));
        }
        rc = PMIX_SUCCESS;
    }
    if (PMIX_SUCCESS != rc && PMIX_OPERATION_SUCCEEDED != rc) {
        /* dsched refused - it is not going anywhere, so there is no
         * departure to wait for */
        PMIX_DESTRUCT_LOCK(&rellock);
        fprintf(stderr, "%s: dsched refused to terminate: %s\n", dsched_globals.basename,
                PMIx_Error_string(rc));
        PMIx_tool_finalize();
        return 1;
    }

    /* wait for the connection to drop */
    PMIX_WAIT_THREAD(&rellock);
    PMIX_DESTRUCT_LOCK(&rellock);
    if (verbose) {
        pmix_output(0, "%s: dsched has terminated", dsched_globals.basename);
    }

    PMIx_tool_finalize();
    PMIX_DESTRUCT(&results);
    return 0;
}

/*
 * Runs on the progress thread when the signal handler below pokes
 * term_pipe. This is the "safe place" the handler cannot itself reach,
 * so the actual teardown belongs here.
 */
static void clean_abort(int fd, short flags, void *arg)
{
    DSCHED_HIDE_UNUSED_PARAMS(fd, flags, arg);

    /* if we have already ordered this once, don't keep
     * doing it to avoid race conditions
     */
    if (pmix_mutex_trylock(&dsterm_abort_inprogress_lock)) { /* returns 1 if already locked */
        if (forcibly_die) {
            /* exit with a non-zero status */
            exit(1);
        }
        fprintf(stderr,
                "%s: abort is already in progress...hit ctrl-c again to forcibly terminate\n\n",
                dsched_globals.basename);
        forcibly_die = true;
        /* reset the event */
        dsched_event_add(&term_handler, NULL);
        return;
    }

    /* we asked dsched to terminate and are waiting on that; the user has
     * given up on us, so drop the connection and go away. Nothing here
     * has to be undone in dsched - a job control request either reached
     * it or did not */
    fflush(stderr);
    PMIx_tool_finalize();
    exit(1);
}

static bool first = true;
static bool second = true;

/*
 * Attempt to terminate cleanly, but do not let a wedged connection trap
 * the user: the second ctrl-c warns, the third one takes the process out
 * from under whatever it is stuck in.
 */
static void abort_signal_callback(int fd)
{
    uint8_t foo = 1;
    char *msg = "Abort is in progress...hit ctrl-c again to forcibly terminate\n\n";
    DSCHED_HIDE_UNUSED_PARAMS(fd);

    if (first) {
        first = false;
        /* tell the event lib to attempt to abnormally terminate */
        if (-1 == write(term_pipe[1], &foo, 1)) {
            _exit(1);
        }
    } else if (second) {
        second = false;
        if (-1 == write(2, (void *) msg, strlen(msg))) {
            _exit(1);
        }
    } else {
        /* a signal handler may not call exit(), and by now there is
         * nothing left worth flushing */
        _exit(1);
    }
}
