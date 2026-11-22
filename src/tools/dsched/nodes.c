/*
 * Copyright (c) 2026      Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/*
 * The node pool: the nodes dsched allocates from.
 *
 * The pool is built once, at startup, from the first of these that
 * applies:
 *
 *  1. the system controller we attached to. We ask it for the nodes in
 *     its DVM (PMIX_QUERY_ALLOCATION), with their slots. A --hostfile
 *     and/or --host given as well do not add to that: they filter it,
 *     keeping only the controller's nodes they name, and a slot count
 *     they give caps the controller's for that node.
 *  2. a --hostfile and/or --host, when there is no controller.
 *  3. the dsched_default_hosts MCA parameter, in --host format, which
 *     exists so that a default can be put in a parameter file.
 *
 * Without any of them the pool is empty, and dsched says so.
 *
 * The pool is built during startup, from dsched_server_init, after we
 * attach to the controller and before our server module is registered.
 * The order matters: a tool that has a server module has its own
 * queries answered by that module, never by its server, so once ours is
 * registered a PMIX_QUERY_ALLOCATION would come back to us - posted to
 * an event loop that is not running yet - and not reach the
 * controller. Waiting for the answer here parks nothing: the event loop
 * has not started.
 */

#include "dsched_config.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/util/pmix_argv.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_constants.h"
#include "src/include/dsched_globals.h"
#include "src/tools/dsched/dsched.h"
#include "src/util/dsched_cmd_line.h"

/* One host a --host, --hostfile or dsched_default_hosts names. A host
 * named without a slot count counts one slot for each time it is named,
 * as in PRRTE; "given" records whether any count was explicit, which
 * matters only when the entry filters a controller's node: there, a
 * host named without one keeps the controller's count. */
typedef struct {
    pmix_list_item_t super;
    char *name;
    int slots;
    bool given;
    bool found;
} dsched_host_entry_t;

static void hecon(dsched_host_entry_t *p)
{
    p->name = NULL;
    p->slots = 0;
    p->given = false;
    p->found = false;
}
static void hedes(dsched_host_entry_t *p)
{
    free(p->name);
}
static PMIX_CLASS_INSTANCE(dsched_host_entry_t, pmix_list_item_t, hecon, hedes);

/* where the pool came from, for the messages that describe it */
static const char *pool_source = NULL;

/* the name a host is known by here: our own host by our own name */
static const char *canonical(const char *name)
{
    return dsched_check_host_is_local(name) ? dsched_globals.hostname : name;
}

static dsched_host_entry_t *find_entry(pmix_list_t *hosts, const char *name)
{
    dsched_host_entry_t *h;

    PMIX_LIST_FOREACH(h, hosts, dsched_host_entry_t) {
        if (0 == strcmp(h->name, name)) {
            return h;
        }
    }
    return NULL;
}

/* add slots for a host, merging a host named more than once */
static bool add_host(pmix_list_t *hosts, const char *name, int slots, bool given)
{
    dsched_host_entry_t *h;
    const char *nm = canonical(name);

    h = find_entry(hosts, nm);
    if (NULL == h) {
        h = PMIX_NEW(dsched_host_entry_t);
        h->name = strdup(nm);
        if (NULL == h->name) {
            PMIX_RELEASE(h);
            return false;
        }
        pmix_list_append(hosts, &h->super);
    }
    if (INT_MAX - h->slots < slots) {
        return false;
    }
    h->slots += slots;
    h->given = h->given || given;
    return true;
}

/* A slot count: digits only, and at least 1 */
static bool parse_slots(const char *str, int *slots)
{
    char *end = NULL;
    long val;

    if (NULL == str || !isdigit((unsigned char) str[0])) {
        return false;
    }
    errno = 0;
    val = strtol(str, &end, 10);
    if (0 != errno || '\0' != *end || 1 > val || INT_MAX < val) {
        return false;
    }
    *slots = (int) val;
    return true;
}

/* A host name, as dsched takes one: not empty, and nothing that would
 * make it a login (user@host) or a separator */
static bool good_name(const char *name)
{
    if ('\0' == name[0]) {
        return false;
    }
    return NULL == strpbrk(name, "@,:= \t");
}

/* Parse a --host format list - "host[:slots],..." - into hosts.
 * origin names where the list came from, for the message on an error. */
static pmix_status_t parse_host_list(const char *list, const char *origin, pmix_list_t *hosts)
{
    char **entries, *colon;
    const char *reason = NULL;
    int n, slots;

    entries = PMIx_Argv_split_with_empty(list, ',');
    if (NULL == entries) {
        pmix_show_help("help-dsched.txt", "bad-host-list", true, origin, list,
                       "it names no host");
        return DSCHED_ERR_SILENT;
    }
    for (n = 0; NULL != entries[n]; n++) {
        slots = 1;
        colon = strchr(entries[n], ':');
        if (NULL != colon) {
            *colon = '\0';
            if (!parse_slots(colon + 1, &slots)) {
                reason = "a slot count is not a positive number";
                break;
            }
        }
        if (!good_name(entries[n])) {
            reason = ('\0' == entries[n][0]) ? "an entry names no host"
                                             : "a host name holds a character a host name cannot";
            break;
        }
        if (!add_host(hosts, entries[n], slots, NULL != colon)) {
            reason = "a host's slot count is too large";
            break;
        }
    }
    PMIx_Argv_free(entries);
    if (NULL != reason) {
        pmix_show_help("help-dsched.txt", "bad-host-list", true, origin, list, reason);
        return DSCHED_ERR_SILENT;
    }
    return PMIX_SUCCESS;
}

/* Parse a hostfile into hosts. Each line names one host, optionally
 * followed by "slots=N"; "max_slots=N" (or "max-slots=N") is accepted
 * and ignored, since dsched allocates whole nodes. A "#" starts a
 * comment, and blank lines are skipped. */
static pmix_status_t parse_hostfile(const char *path, pmix_list_t *hosts)
{
    FILE *fp;
    char line[1024], *ptr, **tokens, *eq;
    const char *reason = NULL;
    int lineno = 0, n, slots;
    bool given;

    fp = fopen(path, "r");
    if (NULL == fp) {
        pmix_show_help("help-dsched.txt", "bad-hostfile", true, path, 0,
                       strerror(errno));
        return DSCHED_ERR_SILENT;
    }
    while (NULL != fgets(line, sizeof(line), fp)) {
        ++lineno;
        if (NULL == strchr(line, '\n') && !feof(fp)) {
            reason = "the line is too long";
            break;
        }
        if (NULL != (ptr = strchr(line, '#'))) {
            *ptr = '\0';
        }
        for (ptr = line; '\0' != *ptr; ptr++) {
            if ('\t' == *ptr || '\n' == *ptr || '\r' == *ptr) {
                *ptr = ' ';
            }
        }
        tokens = PMIx_Argv_split(line, ' ');
        if (NULL == tokens || NULL == tokens[0]) {
            PMIx_Argv_free(tokens);
            continue;
        }
        if (!good_name(tokens[0])) {
            reason = "the host name holds a character a host name cannot";
            PMIx_Argv_free(tokens);
            break;
        }
        slots = 1;
        given = false;
        for (n = 1; NULL != tokens[n]; n++) {
            eq = strchr(tokens[n], '=');
            if (NULL == eq) {
                reason = "an entry after the host name is not of the form key=value";
                break;
            }
            *eq = '\0';
            if (0 == strcmp(tokens[n], "slots")) {
                if (!parse_slots(eq + 1, &slots)) {
                    reason = "the slot count is not a positive number";
                    break;
                }
                given = true;
            } else if (0 == strcmp(tokens[n], "max_slots") ||
                       0 == strcmp(tokens[n], "max-slots")) {
                int maxslots;
                // read, so a bad value is reported, but dsched allocates
                // whole nodes and so has no use for the limit
                if (!parse_slots(eq + 1, &maxslots) && 0 != strcmp(eq + 1, "0")) {
                    reason = "the maximum slot count is not a number";
                    break;
                }
            } else {
                reason = "an entry after the host name is not slots= or max_slots=";
                break;
            }
        }
        if (NULL == reason && !add_host(hosts, tokens[0], slots, given)) {
            reason = "the host's slot count is too large";
        }
        PMIx_Argv_free(tokens);
        if (NULL != reason) {
            break;
        }
    }
    fclose(fp);
    if (NULL != reason) {
        pmix_show_help("help-dsched.txt", "bad-hostfile", true, path, lineno, reason);
        return DSCHED_ERR_SILENT;
    }
    if (0 == pmix_list_get_size(hosts)) {
        pmix_show_help("help-dsched.txt", "bad-hostfile", true, path, lineno,
                       "the file names no host");
        return DSCHED_ERR_SILENT;
    }
    return PMIX_SUCCESS;
}

/* Add one node to the pool */
static pmix_status_t pool_add(const char *name, char **aliases, int slots)
{
    dsched_node_t *node;

    if (INT_MAX - dsched_globals.avail.nslots < slots) {
        return PMIX_ERR_BAD_PARAM;
    }
    node = PMIX_NEW(dsched_node_t);
    if (NULL == node) {
        return PMIX_ERR_NOMEM;
    }
    node->name = strdup(name);
    if (NULL == node->name) {
        PMIX_RELEASE(node);
        return PMIX_ERR_NOMEM;
    }
    if (NULL != aliases) {
        node->aliases = PMIx_Argv_copy(aliases);
    }
    node->slots = slots;
    node->slots_available = slots;
    node->index = pmix_pointer_array_add(&dsched_globals.nodes, node);
    if (0 > node->index) {
        PMIX_RELEASE(node);
        return PMIX_ERR_NOMEM;
    }
    dsched_globals.avail.nnodes++;
    dsched_globals.avail.nslots += slots;
    pmix_output_verbose(2, dsched_globals.output,
                        "%s node pool: %s with %d slot%s",
                        PMIX_NAME_PRINT(&dsched_globals.myid), name, slots,
                        (1 == slots) ? "" : "s");
    return PMIX_SUCCESS;
}

/* is this controller node named by an entry of the filter? */
static dsched_host_entry_t *filter_match(pmix_list_t *filter, const char *name, char **aliases)
{
    dsched_host_entry_t *h;
    int m;

    if (NULL != (h = find_entry(filter, canonical(name)))) {
        return h;
    }
    for (m = 0; NULL != aliases && NULL != aliases[m]; m++) {
        if (NULL != (h = find_entry(filter, canonical(aliases[m])))) {
            return h;
        }
    }
    return NULL;
}

/* One PMIX_NODE_INFO entry of the controller's answer */
static pmix_status_t take_controller_node(const pmix_value_t *val, pmix_list_t *filter,
                                          bool *kept)
{
    pmix_info_t *info;
    size_t n, ninfo;
    const char *name = NULL;
    char **aliases = NULL;
    uint32_t nslots = 0;
    bool have_slots = false;
    dsched_host_entry_t *h;
    int slots;
    pmix_status_t rc;

    *kept = false;
    if (PMIX_DATA_ARRAY != val->type || NULL == val->data.darray ||
        PMIX_INFO != val->data.darray->type) {
        return PMIX_ERR_BAD_PARAM;
    }
    info = (pmix_info_t *) val->data.darray->array;
    ninfo = val->data.darray->size;
    for (n = 0; NULL != info && n < ninfo; n++) {
        if (PMIX_CHECK_KEY(&info[n], PMIX_HOSTNAME)) {
            if (PMIX_STRING != info[n].value.type || NULL == info[n].value.data.string) {
                return PMIX_ERR_BAD_PARAM;
            }
            name = info[n].value.data.string;
        } else if (PMIX_CHECK_KEY(&info[n], PMIX_HOSTNAME_ALIASES)) {
            if (PMIX_STRING == info[n].value.type && NULL != info[n].value.data.string &&
                NULL == aliases) {
                aliases = PMIx_Argv_split(info[n].value.data.string, ',');
            }
        } else if (PMIX_CHECK_KEY(&info[n], PMIX_NUM_SLOTS)) {
            if (PMIX_SUCCESS != PMIx_Value_get_number(&info[n].value, &nslots, PMIX_UINT32)) {
                PMIx_Argv_free(aliases);
                return PMIX_ERR_BAD_PARAM;
            }
            have_slots = true;
        }
    }
    if (NULL == name || !good_name(name)) {
        PMIx_Argv_free(aliases);
        return PMIX_ERR_BAD_PARAM;
    }
    /* a node the controller gives no slots has none to allocate */
    if (!have_slots || 0 == nslots) {
        pmix_output_verbose(2, dsched_globals.output,
                            "%s node pool: controller node %s has no slots - skipped",
                            PMIX_NAME_PRINT(&dsched_globals.myid), name);
        PMIx_Argv_free(aliases);
        return PMIX_SUCCESS;
    }
    slots = (INT_MAX < nslots) ? INT_MAX : (int) nslots;

    if (0 < pmix_list_get_size(filter)) {
        h = filter_match(filter, name, aliases);
        if (NULL == h) {
            PMIx_Argv_free(aliases);
            return PMIX_SUCCESS;
        }
        h->found = true;
        if (h->given && h->slots < slots) {
            slots = h->slots;
        }
    }
    rc = pool_add(name, aliases, slots);
    PMIx_Argv_free(aliases);
    *kept = (PMIX_SUCCESS == rc);
    return rc;
}

static void pool_done(void)
{
    if (0 == dsched_globals.avail.nnodes) {
        pmix_show_help("help-dsched.txt", "no-nodes", true, pool_source);
    } else {
        pmix_output_verbose(1, dsched_globals.output,
                            "%s node pool: %d nodes, %d slots, from %s",
                            PMIX_NAME_PRINT(&dsched_globals.myid), dsched_globals.avail.nnodes,
                            dsched_globals.avail.nslots, pool_source);
    }
}

/* Ask the controller for the nodes in its DVM, keeping those the
 * filter names if it names any. */
static pmix_status_t pool_from_controller(pmix_list_t *filter)
{
    pmix_query_t query;
    pmix_info_t *results = NULL, *entries;
    size_t nresults = 0, n, m, nentries;
    pmix_status_t rc;
    dsched_host_entry_t *h;
    char **missing = NULL, *tmp;
    const char *reason = NULL;
    bool kept;

    PMIX_QUERY_CONSTRUCT(&query);
    PMIx_Argv_append_nosize(&query.keys, PMIX_QUERY_ALLOCATION);
    rc = PMIx_Query_info(&query, 1, &results, &nresults);
    PMIX_QUERY_DESTRUCT(&query);
    if (PMIX_SUCCESS != rc && PMIX_ERR_PARTIAL_SUCCESS != rc) {
        reason = PMIx_Error_string(rc);
    }
    for (n = 0; NULL == reason && n < nresults; n++) {
        if (!PMIX_CHECK_KEY(&results[n], PMIX_QUERY_ALLOCATION)) {
            continue;
        }
        if (PMIX_DATA_ARRAY != results[n].value.type || NULL == results[n].value.data.darray ||
            PMIX_INFO != results[n].value.data.darray->type) {
            reason = "its answer could not be read";
            break;
        }
        entries = (pmix_info_t *) results[n].value.data.darray->array;
        nentries = results[n].value.data.darray->size;
        rc = PMIX_SUCCESS;
        for (m = 0; PMIX_SUCCESS == rc && NULL != entries && m < nentries; m++) {
            // the topologies that follow the nodes are not used yet
            if (PMIX_CHECK_KEY(&entries[m], PMIX_NODE_INFO)) {
                rc = take_controller_node(&entries[m].value, filter, &kept);
            }
        }
        if (PMIX_SUCCESS != rc) {
            reason = "its description of a node could not be read";
        }
        break;
    }
    if (NULL != results) {
        PMIx_Info_free(results, nresults);
    }
    if (NULL != reason) {
        pmix_show_help("help-dsched.txt", "controller-nodes", true,
                       PMIX_NAME_PRINT(&dsched_globals.syscontroller), reason);
        return DSCHED_ERR_SILENT;
    }

    /* the named hosts the controller does not have */
    PMIX_LIST_FOREACH(h, filter, dsched_host_entry_t) {
        if (!h->found) {
            PMIx_Argv_append_nosize(&missing, h->name);
        }
    }
    if (NULL != missing) {
        tmp = PMIx_Argv_join(missing, ',');
        pmix_show_help("help-dsched.txt", "hosts-not-in-dvm", true,
                       PMIX_NAME_PRINT(&dsched_globals.syscontroller), tmp);
        free(tmp);
        PMIx_Argv_free(missing);
    }
    return PMIX_SUCCESS;
}

static pmix_status_t pool_from_hosts(pmix_list_t *hosts)
{
    dsched_host_entry_t *h;
    pmix_status_t rc;

    PMIX_LIST_FOREACH(h, hosts, dsched_host_entry_t) {
        rc = pool_add(h->name, NULL, h->slots);
        if (PMIX_SUCCESS != rc) {
            return rc;
        }
    }
    return PMIX_SUCCESS;
}

pmix_status_t dsched_nodes_init(pmix_cli_result_t *results)
{
    pmix_list_t hosts;
    pmix_cli_item_t *opt;
    char *list;
    bool hostfile_given = false, host_given = false;
    pmix_status_t rc = PMIX_SUCCESS;

    PMIX_CONSTRUCT(&hosts, pmix_list_t);

    /* the hosts named on the command line */
    opt = pmix_cmd_line_get_param(results, DSCHED_CLI_HOSTFILE);
    if (NULL != opt && NULL != opt->values) {
        hostfile_given = true;
        for (int n = 0; PMIX_SUCCESS == rc && NULL != opt->values[n]; n++) {
            rc = parse_hostfile(opt->values[n], &hosts);
        }
    }
    opt = pmix_cmd_line_get_param(results, DSCHED_CLI_HOST);
    if (PMIX_SUCCESS == rc && NULL != opt && NULL != opt->values) {
        host_given = true;
        list = PMIx_Argv_join(opt->values, ',');
        rc = parse_host_list(list, "--host", &hosts);
        free(list);
    }
    if (PMIX_SUCCESS != rc) {
        PMIX_LIST_DESTRUCT(&hosts);
        return rc;
    }

    if (dsched_globals.controller_connected) {
        // the hosts named filter what the controller answers
        pool_source = "the system controller";
        rc = pool_from_controller(&hosts);
    } else if (0 < pmix_list_get_size(&hosts)) {
        pool_source = (hostfile_given && host_given) ? "--hostfile and --host"
                      : (hostfile_given ? "--hostfile" : "--host");
        rc = pool_from_hosts(&hosts);
    } else if (NULL != dsched_globals.default_hosts && '\0' != dsched_globals.default_hosts[0]) {
        pool_source = "dsched_default_hosts";
        rc = parse_host_list(dsched_globals.default_hosts, "dsched_default_hosts", &hosts);
        if (PMIX_SUCCESS == rc) {
            rc = pool_from_hosts(&hosts);
        }
    } else {
        pool_source = "nowhere - nothing names any";
    }
    PMIX_LIST_DESTRUCT(&hosts);
    if (PMIX_SUCCESS != rc) {
        return rc;
    }
    pool_done();
    return PMIX_SUCCESS;
}
