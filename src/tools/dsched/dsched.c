/***************************************************************************
 *                                                                         *
 *              DynaSched: Dynamic Scheduler Platform                      *
 *                                                                         *
 *              https://github.com/dynasched/dynasched                     *
 *                                                                         *
 ***************************************************************************/

#include "dsched_config.h"

#include <sys/wait.h>

#include "src/util/pmix_argv.h"
#include "src/util/pmix_basename.h"
#include "src/util/pmix_cmd_line.h"
#include "src/util/pmix_environ.h"
#include "src/util/pmix_fd.h"
#include "src/util/pmix_printf.h"
#include "src/threads/pmix_threads.h"

#include "src/include/dsched_constants.h"
#include "src/include/dsched_globals.h"
#include "src/runtime/dsched_rte.h"
#include "src/util/dsched_cmd_line.h"
#include "src/util/dsched_daemon_init.h"
#include "src/util/dsched_error.h"
#include "src/util/dsched_session_dir.h"

#include "src/tools/dsched/dsched.h"

typedef struct {
    pmix_lock_t lock;
    pmix_status_t status;
    pmix_info_t *info;
    size_t ninfo;
} myxfer_t;

static int wait_pipe[2];
static int term_pipe[2];
static pmix_mutex_t dsched_abort_inprogress_lock = PMIX_MUTEX_STATIC_INIT;
static void abort_signal_callback(int signal);
static void clean_abort(int fd, short flags, void *arg);
static bool keepalive = false;
static bool forcibly_die = false;
static dsched_event_t term_handler;

static void parent_died_fn(size_t evhdlr_registration_id, pmix_status_t status,
                           const pmix_proc_t *source, pmix_info_t info[], size_t ninfo,
                           pmix_info_t res[], size_t nres,
                           pmix_event_notification_cbfunc_fn_t cbfunc, void *cbdata)
{
    dsched_shift_caddy_t *cd;
    DSCHED_HIDE_UNUSED_PARAMS(evhdlr_registration_id, status, source, info, ninfo, res, nres);

    // allow the pmix event base to continue
    cbfunc(PMIX_EVENT_ACTION_COMPLETE, NULL, 0, NULL, NULL, cbdata);

    // shift this into our event base
    cd = PMIX_NEW(dsched_shift_caddy_t);
    dsched_event_set(dsched_globals.evbase, &(cd->ev), -1, DSCHED_EV_WRITE, clean_abort, cd);
    dsched_event_active(&(cd->ev), DSCHED_EV_WRITE, 1);
}

static void evhandler_reg_callbk(pmix_status_t status, size_t evhandler_ref, void *cbdata)
{
    myxfer_t *lock = (myxfer_t *) cbdata;
    DSCHED_HIDE_UNUSED_PARAMS(evhandler_ref);

    lock->status = status;
    PMIX_WAKEUP_THREAD(&lock->lock);
}

static int wait_dvm(pid_t pid)
{
    char reply;
    int rc;
    int status;

    close(wait_pipe[1]);
    do {
        rc = read(wait_pipe[0], &reply, 1);
    } while (0 > rc && EINTR == errno);

    if (1 == rc && 'K' == reply) {
        return 0;
    } else if (0 == rc) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }
    return 255;
}

static void clean_abort(int fd, short flags, void *arg)
{
    DSCHED_HIDE_UNUSED_PARAMS(fd, flags);

    if (keepalive && NULL == arg) {
        // ignore this
        return;
    }

    /* if we have already ordered this once, don't keep
     * doing it to avoid race conditions
     */
    if (pmix_mutex_trylock(&dsched_abort_inprogress_lock)) { /* returns 1 if already locked */
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

    /* ensure we exit with a non-zero status */
    DSCHED_UPDATE_EXIT_STATUS(1);
    if (NULL != arg) {
        PMIX_RELEASE(arg);
    }
    // shutdown the pmix server
    dsched_server_finalize();
    // cleanup the session dir
    dsched_session_dir_finalize();
    exit(1);
}

static bool first = true;
static bool second = true;

/*
 * Attempt to terminate the job and wait for callback indicating
 * the job has been aborted.
 */
static void abort_signal_callback(int fd)
{
    uint8_t foo = 1;
    char *msg = "Abort is in progress...hit ctrl-c again to forcibly terminate\n\n";
    DSCHED_HIDE_UNUSED_PARAMS(fd);

    /* if this is the first time thru, just get
     * the current time
     */
    if (first) {
        first = false;
        /* tell the event lib to attempt to abnormally terminate */
        if (-1 == write(term_pipe[1], &foo, 1)) {
            exit(1);
        }
    } else if (second) {
        if (-1 == write(2, (void *) msg, strlen(msg))) {
            exit(1);
        }
        fflush(stderr);
        second = false;
    } else {
        // shutdown the pmix server
        dsched_server_finalize();
        // cleanup the session dir
        dsched_session_dir_finalize();
        exit(1);
    }
}

static void allow_run_as_root(pmix_cli_result_t *cli)
{
    char *r1, *r2;

    if (pmix_cmd_line_is_taken(cli, "allow-run-as-root")) {
        return;
    }

    if (NULL != (r1 = getenv("DSCHED_ALLOW_RUN_AS_ROOT"))
        && NULL != (r2 = getenv("DSCHED_ALLOW_RUN_AS_ROOT_CONFIRM"))) {
        if (0 == strcmp(r1, "1") && 0 == strcmp(r2, "1")) {
            return;
        }
    }

    fprintf(stderr, "%s has detected an attempt to run as root.\n\n", dsched_globals.basename);
    fprintf(stderr, "Running as root is *strongly* discouraged as any mistake (e.g., in\n");
    fprintf(stderr, "defining TMPDIR) or bug can result in catastrophic damage to the OS\n");
    fprintf(stderr, "file system, leaving your system in an unusable state.\n\n");

    fprintf(stderr, "We strongly suggest that you run %s as a non-root user.\n\n",
            dsched_globals.basename);

    fprintf(stderr, "You can override this protection by adding the --allow-run-as-root\n");
    fprintf(stderr, "option to your command line.  However, we reiterate our strong advice\n");
    fprintf(stderr, "against doing so - please do so at your own risk.\n");
    fprintf(stderr, "--------------------------------------------------------------------------\n");
    exit(1);
}

// define the dsched cmd line
static struct option dschedoptions[] = {
    /* basic options */
    PMIX_OPTION_SHORT_DEFINE(DSCHED_CLI_HELP, PMIX_ARG_OPTIONAL, 'h'),
    PMIX_OPTION_SHORT_DEFINE(DSCHED_CLI_VERSION, PMIX_ARG_NONE, 'V'),
    PMIX_OPTION_SHORT_DEFINE(DSCHED_CLI_VERBOSE, PMIX_ARG_NONE, 'v'),

    // MCA parameters
    PMIX_OPTION_DEFINE(DSCHED_CLI_DMCA, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_MCA, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_PMIXMCA, PMIX_ARG_REQD),

    // DVM options
    PMIX_OPTION_DEFINE(DSCHED_CLI_RUN_AS_ROOT, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(DSCHED_CLI_DAEMONIZE, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(DSCHED_CLI_NO_READY_MSG, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(DSCHED_CLI_SET_SID, PMIX_ARG_NONE),
    PMIX_OPTION_DEFINE(DSCHED_CLI_TMPDIR, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_REPORT_PID, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_REPORT_URI, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_KEEPALIVE, PMIX_ARG_REQD),
    PMIX_OPTION_DEFINE(DSCHED_CLI_CONTROLLER_URI, PMIX_ARG_REQD),

    // debug options
    PMIX_OPTION_DEFINE(DSCHED_CLI_DEBUG, PMIX_ARG_NONE),

    PMIX_OPTION_END
};
static char *dschedshorts = "h::vV";


int main(int argc, char *argv[])
{
    int ret = 0;
    pmix_status_t rc, code;
    pmix_proc_t pname;
    pmix_status_t prc;
    myxfer_t xfer;
    char *mypidfile = NULL, *evar;
    pmix_info_t info;
    char **pargv, *str, *error = NULL;
    pmix_cli_result_t results;
    pmix_cli_item_t *opt;
    bool ready_msg = true;
    DSCHED_HIDE_UNUSED_PARAMS(argc, argv);

    /* protect against problems if someone passes us thru a pipe
     * and then abnormally terminates the pipe early */
    signal(SIGPIPE, SIG_IGN);

    // protect the inputs
    pargv = pmix_argv_copy_strip(argv);  // strip any quoted arguments

    /* Initialize the argv parsing stuff */
    dsched_globals.basename = pmix_basename(pargv[0]);
    dsched_globals.pid = dsched_globals.pid;
    if (DSCHED_SUCCESS != (ret = dsched_init_util())) {
        fprintf(stderr, "dsched_init_util failed: %s\n", PMIx_Error_string(ret));
        exit(ret);
    }

    /** setup callbacks for abort signals - from this point
     * forward, we need to abort in a manner that allows us
     * to cleanup. However, we cannot directly use libevent
     * to trap these signals as otherwise we cannot respond
     * to them if we are stuck in an event! So instead use
     * the basic POSIX trap functions to handle the signal,
     * and then let that signal handler do some magic to
     * avoid the hang
     *
     * NOTE: posix traps don't allow us to do anything major
     * in them, so use a pipe tied to a libevent event to
     * reach a "safe" place where the termination event can
     * be created
     */
    if (0 != (rc = pipe(term_pipe))) {
        exit(1);
    }
    /* setup an event to attempt normal termination on signal */
    dsched_event_set(dsched_globals.evbase, &term_handler, term_pipe[0], DSCHED_EV_READ, clean_abort, NULL);
    dsched_event_add(&term_handler, NULL);

    /* point the signal trap to a function that will activate that event */
    signal(SIGTERM, abort_signal_callback);
    signal(SIGINT, abort_signal_callback);
    signal(SIGHUP, abort_signal_callback);

    /* parse the input argv to get values, including everyone's MCA params */
    PMIX_CONSTRUCT(&results, pmix_cli_result_t);
    rc = pmix_cmd_line_parse(pargv, dschedshorts, dschedoptions, NULL,
                             &results, "help-dsched.txt");
    if (PMIX_SUCCESS != rc) {
        PMIX_DESTRUCT(&results);
        if (PMIX_OPERATION_SUCCEEDED == rc) {
            return PMIX_SUCCESS;
        }
        if (PMIX_ERR_SILENT != rc) {
            fprintf(stderr, "%s: command line error (%s)\n", dsched_globals.basename, PMIx_Error_string(rc));
        }
        return rc;
    }

    // we do NOT accept arguments other than our own
    if (NULL != results.tail) {
        str = PMIx_Argv_join(results.tail, ' ');
        if (0 != strcmp(str, pargv[0])) {
            char *ptr;
            ptr = pmix_show_help_string("help-dsched_info.txt", "no-args", false,
                                        dsched_globals.basename, str, dsched_globals.basename);
            free(str);
            if (NULL != ptr) {
                printf("%s", ptr);
                free(ptr);
            }
            return -1;
        }
        free(str);
    }

    /* check if we are running as root - if we are, then only allow
     * us to proceed if the allow-run-as-root flag was given. Otherwise,
     * exit with a giant warning message
     */
    if (0 == geteuid()) {
        allow_run_as_root(&results); // will exit us if not allowed
    }

    /* we may have been passed a PMIx nspace to use */
    if (NULL != (evar = getenv("PMIX_SERVER_NSPACE"))) {
        PMIX_LOAD_NSPACE(dsched_globals.myid.nspace, evar);
    } else {
        /* use basename-hostname-pid as our base nspace */
        pmix_asprintf(&str, "%s-%s-%u", dsched_globals.basename,
                      dsched_globals.hostname, (uint32_t) dsched_globals.pid);
        PMIX_LOAD_NSPACE(dsched_globals.myid.nspace, str);
        free(str);
    }
    if (NULL != (evar = getenv("PMIX_SERVER_RANK"))) {
        dsched_globals.myid.rank = strtoul(evar, NULL, 10);
    } else {
        dsched_globals.myid.rank = 0;
    }

    /* setup the global session and node arrays */
    PMIX_CONSTRUCT(&dsched_globals.nodes, pmix_pointer_array_t);
    ret = pmix_pointer_array_init(&dsched_globals.nodes, DSCHED_GLOBAL_ARRAY_BLOCK_SIZE,
                                  DSCHED_GLOBAL_ARRAY_MAX_SIZE,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE);
    if (PMIX_SUCCESS != ret) {
        PMIX_ERROR_LOG(ret);
        error = "setup node array";
        goto DONE;
    }
    PMIX_CONSTRUCT(&dsched_globals.sessions, pmix_pointer_array_t);
    ret = pmix_pointer_array_init(&dsched_globals.sessions,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE,
                                  DSCHED_GLOBAL_ARRAY_MAX_SIZE,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE);
    if (PMIX_SUCCESS != ret) {
        PMIX_ERROR_LOG(ret);
        error = "setup session array";
        goto DONE;
    }

    // setup an array for node topologies
    PMIX_CONSTRUCT(&dsched_globals.topologies, pmix_pointer_array_t);
    ret = pmix_pointer_array_init(&dsched_globals.topologies, DSCHED_GLOBAL_ARRAY_BLOCK_SIZE,
                                  DSCHED_GLOBAL_ARRAY_MAX_SIZE,
                                  DSCHED_GLOBAL_ARRAY_BLOCK_SIZE);
    if (PMIX_SUCCESS != ret) {
        PMIX_ERROR_LOG(ret);
        error = "setup node topologies array";
        goto DONE;
    }

    /* initialize the requests cache */
    PMIX_CONSTRUCT(&dsched_globals.requests, pmix_pointer_array_t);
    pmix_pointer_array_init(&dsched_globals.requests, 1, INT_MAX, 1);

    /* if we were given a keepalive pipe, set up to monitor it now */
    opt = pmix_cmd_line_get_param(&results, DSCHED_CLI_KEEPALIVE);
    if (NULL != opt) {
        keepalive = true;
        PMIx_Setenv("PMIX_KEEPALIVE_PIPE", opt->values[0], true, &environ);
    }

    /* check for debug options */
    if (pmix_cmd_line_is_taken(&results, DSCHED_CLI_DEBUG)) {
        dsched_globals.debug = true;
        if (dsched_globals.verbosity <= 0) {
            // verbosity not previously set, so do so now
            dsched_globals.verbosity = 10;
            dsched_globals.output = pmix_output_open(NULL);
            pmix_output_set_verbosity(dsched_globals.output,
                                      dsched_globals.verbosity);
            dsched_globals.pmix_output = pmix_output_open(NULL);
            pmix_output_set_verbosity(dsched_globals.pmix_output,
                                      dsched_globals.verbosity);
        }
    }

    // detach from controlling terminal, if so directed
    if (pmix_cmd_line_is_taken(&results, DSCHED_CLI_DAEMONIZE)) {
        ret = pipe(wait_pipe);
        if (0 > ret) {
            fprintf(stderr, "Error opening pipe: %s\n", strerror(errno));
            ret = DSCHED_ERR_SILENT;
            goto DONE;
        }
        dsched_globals.parent_fd = wait_pipe[1];
        dsched_daemon_init_callback(NULL, wait_dvm);
        close(wait_pipe[0]);
#if defined(HAVE_SETSID)
        /* see if we were directed to separate from current session */
        if (pmix_cmd_line_is_taken(&results, DSCHED_CLI_SET_SID)) {
            setsid();
        }
#endif
    }

    if (pmix_cmd_line_is_taken(&results, DSCHED_CLI_NO_READY_MSG)) {
        ready_msg = false;
    }

    /* if we were asked to report a uri, set the MCA param to do so */
    opt = pmix_cmd_line_get_param(&results, DSCHED_CLI_REPORT_URI);
    if (NULL != opt) {
        dsched_globals.report_uri = strdup(opt->values[0]);
    }

    opt = pmix_cmd_line_get_param(&results, DSCHED_CLI_REPORT_PID);
    if (NULL != opt) {
        /* if the string is a "-", then output to stdout */
        if (0 == strcmp(opt->values[0], "-")) {
            fprintf(stdout, "%lu\n", (unsigned long) dsched_globals.pid);
        } else if (0 == strcmp(opt->values[0], "+")) {
            /* output to stderr */
            fprintf(stderr, "%lu\n", (unsigned long) dsched_globals.pid);
        } else {
            char *leftover;
            int outpipe;
            /* see if it is an integer pipe */
            leftover = NULL;
            outpipe = strtol(opt->values[0], &leftover, 10);
            if (NULL == leftover || 0 == strlen(leftover)) {
                /* stitch together the var names and URI */
                pmix_asprintf(&leftover, "%lu", (unsigned long) dsched_globals.pid);
                /* output to the pipe */
                prc = pmix_fd_write(outpipe, strlen(leftover) + 1, leftover);
                if (PMIX_SUCCESS != prc) {
                    fprintf(stderr, "Error writing PID to pipe: %s\n", strerror(errno));
                }
                free(leftover);
                close(outpipe);
            } else {
                /* must be a file */
                FILE *fp;
                fp = fopen(opt->values[0], "w");
                if (NULL == fp) {
                    pmix_output(0, "Impossible to open the file %s in write mode\n", opt->values[0]);
                    DSCHED_UPDATE_EXIT_STATUS(1);
                    goto DONE;
                }
                /* output my PID */
                fprintf(fp, "%lu\n", (unsigned long) dsched_globals.pid);
                fclose(fp);
                mypidfile = strdup(opt->values[0]);
            }
        }
    }

    /* create the directory tree */
    ret = dsched_session_dir_init();
    if (DSCHED_SUCCESS != ret) {
        pmix_show_help("help-dsched-runtime",
                       "dsched_init:startup:internal-failure", true,
                       "session_dir", DSCHED_ERROR_NAME(ret), ret);
        DSCHED_UPDATE_EXIT_STATUS(ret);
        goto DONE;
    }

    /* setup the PMIx server library */
    if (DSCHED_SUCCESS != (ret = dsched_server_init(&results))) {
        /* the server code already barked, so let's be quiet */
        dsched_globals.exit_status = DSCHED_ERR_SILENT;
        goto DONE;
    }

    if (pmix_cmd_line_is_taken(&results, DSCHED_CLI_KEEPALIVE)) {
        /* setup the keepalive event registration */
        memset(&xfer, 0, sizeof(myxfer_t));
        PMIX_CONSTRUCT_LOCK(&xfer.lock);
        code = PMIX_ERR_JOB_TERMINATED;
        PMIX_LOAD_PROCID(&pname, "PMIX_KEEPALIVE_PIPE", PMIX_RANK_UNDEF);
        PMIX_INFO_LOAD(&info, PMIX_EVENT_AFFECTED_PROC, &pname, PMIX_PROC);
        PMIx_Register_event_handler(&code, 1, &info, 1, parent_died_fn, evhandler_reg_callbk,
                                    (void *) &xfer);
        PMIX_WAIT_THREAD(&xfer.lock);
        PMIX_INFO_DESTRUCT(&info);
        PMIX_DESTRUCT_LOCK(&xfer.lock);
    }

    /* output a message indicating we are alive, our name, and our pid */
    if (ready_msg) {
        fprintf(stderr, "Scheduler checking in as pid %ld on host %s\n",
                (long) dsched_globals.pid, dsched_globals.hostname);
    }

    /* loop the event lib until an exit event is detected */
    while (dsched_globals.evactive) {
        dsched_event_loop(dsched_globals.evbase, DSCHED_EVLOOP_ONCE);
    }
    PMIX_ACQUIRE_OBJECT(dsched_globals.evactive);

DONE:
    if (DSCHED_ERR_SILENT != ret && NULL != error) {
        pmix_show_help("help-dsched-runtime", "dsched_init:startup:internal-failure", true,
                       error, DSCHED_ERROR_NAME(ret), ret);
    }

    /* update the exit status, in case it wasn't done */
    DSCHED_UPDATE_EXIT_STATUS(ret);

    /* cleanup and leave */
    if (NULL != mypidfile) {
        unlink(mypidfile);
    }
    dsched_server_finalize();

    // purge the session directory tree
    dsched_session_dir_finalize();

    if (dsched_globals.debug) {
        fprintf(stderr, "exiting with status %d\n", dsched_globals.exit_status);
    }
    exit(dsched_globals.exit_status);

    return ret;
}
