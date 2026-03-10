#include "dsched_config.h"

#include <stdarg.h>

#include "src/include/dsched_globals.h"

dsched_globals_t dsched_globals = {
    .myid = PMIX_PROC_STATIC_INIT,
    .pid = 0,
    .evbase = NULL,
    .evactive = false,
    .initialized = false,
    .version_string = NULL,
    .basename = NULL,
    .hostname = NULL,
    .aliases = NULL,
    .nodes = PMIX_POINTER_ARRAY_STATIC_INIT,
    .sessions = PMIX_POINTER_ARRAY_STATIC_INIT,
    .topologies = PMIX_POINTER_ARRAY_STATIC_INIT,
    .requests = PMIX_POINTER_ARRAY_STATIC_INIT,
    .param_files = NULL,
    .override_param_file = NULL,
    .suppress_override_warning = false,
    .clean_output = -1,
    .tmpdir = NULL,
    .prohibited_session_dirs = NULL,
    .exit_status = 0,
    .debug = false,
    .verbosity = -1,
    .output = -1,
    .pmix_output = -1,
    .report_uri = NULL,
    .tools = PMIX_LIST_STATIC_INIT,
    .syscontroller = PMIX_PROC_STATIC_INIT,
    .controller_connected = false,
    .parent_fd = -1,
    .server_initialized = false,
    .progress_thread_cpus = NULL,
    .bind_progress_thread_reqd = false,
    .keep_fqdn_hostnames = false,
    .strip_prefixes = NULL
};

#if DSCHED_PICKY_COMPILERS
void dsched_hide_unused_params(int x, ...)
{
    va_list ap;
    (void)x;
    va_start(ap, x);
    va_end(ap);
}
#endif

bool dsched_check_host_is_local(const char *name)
{
    int i;

    if (0 == strcmp(name, dsched_globals.hostname) ||
        0 == strcmp(name, "localhost") ||
        0 == strcmp(name, "127.0.0.1")) {
        return true;
    }

    if (NULL != dsched_globals.aliases) {
        for (i = 0; NULL != dsched_globals.aliases[i]; i++) {
            if (0 == strcmp(name, dsched_globals.aliases[i])) {
                return true;
            }
        }
    }
    return false;
}


dsched_node_t* dsched_node_match(pmix_list_t *nodes,
                                 const char *name,
                                 uint32_t nodeid)
{
    int m, n;
    dsched_node_t *nptr;
    char *nm;

    /* does the name refer to me? */
    if (NULL != name) {
        if (dsched_check_host_is_local(name)) {
            nm = dsched_globals.hostname;
        } else {
            nm = (char*)name;
        }
    } else {
        nm = NULL;
    }

    if (NULL != nodes) {
        PMIX_LIST_FOREACH(nptr, nodes, dsched_node_t) {
            if (UINT32_MAX != nodeid) {
                if (nptr->nodeid == nodeid) {
                    return nptr;
                }
            }
            if (NULL != nm) {
                if (0 == strcmp(nptr->name, nm)) {
                    return nptr;
                }
                if (NULL == nptr->aliases) {
                    continue;
                }
                /* no choice but an exhaustive search - fortunately, these lists are short! */
                for (m = 0; NULL != nptr->aliases[m]; m++) {
                    if (0 == strcmp(name, nptr->aliases[m])) {
                        /* this is the node! */
                        return nptr;
                    }
                }
            }
        }
    } else {
        if (UINT32_MAX != nodeid) {
            /* get the node object at that index */
            nptr = (dsched_node_t *) pmix_pointer_array_get_item(&dsched_globals.nodes, nodeid);
            if (NULL != nptr) {
                return nptr;
            }
        }
        if (NULL != nm) {
            /* check the node pool */
            for (n=0; n < dsched_globals.nodes.size; n++) {
                nptr = (dsched_node_t*)pmix_pointer_array_get_item(&dsched_globals.nodes, n);
                if (NULL == nptr) {
                    continue;
                }
                if (0 == strcmp(nptr->name, nm)) {
                    return nptr;
                }
                if (NULL == nptr->aliases) {
                    continue;
                }
                /* no choice but an exhaustive search - fortunately, these lists are short! */
                for (m = 0; NULL != nptr->aliases[m]; m++) {
                    if (0 == strcmp(name, nptr->aliases[m])) {
                        /* this is the node! */
                        return nptr;
                    }
                }
            }
        }
    }

    return NULL;
}

bool dsched_nptr_match(dsched_node_t *n1, dsched_node_t *n2)
{
    size_t i, m;

    /* start with the simple check */
    if (0 == strcmp(n1->name, n2->name)) {
        return true;
    }

    if (NULL != n1->aliases) {
        for (i = 0; NULL != n1->aliases[i]; i++) {
            if (0 == strcmp(n1->aliases[i], n2->name)) {
                return true;
            }
            if (NULL != n2->aliases) {
                for (m = 0; NULL != n2->aliases[m]; m++) {
                    if (0 == strcmp(n2->aliases[m], n1->name)) {
                        return true;
                    }
                    if (0 == strcmp(n1->aliases[i], n2->aliases[m])) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


static void dsched_node_construct(dsched_node_t *node)
{
    node->index = -1;
    node->name = NULL;
    node->rawname = NULL;
    node->aliases = NULL;
    node->nodeid = UINT32_MAX;
    node->slots = 0;
    node->slots_available = 0;
    node->slots_inuse = 0;
    node->slots_max = 0;
    node->topology = NULL;

    node->flags = 0;
    PMIX_CONSTRUCT(&node->attributes, pmix_list_t);
}

static void dsched_node_destruct(dsched_node_t *node)
{
    if (NULL != node->name) {
        free(node->name);
        node->name = NULL;
    }
    if (NULL != node->rawname) {
        free(node->rawname);
        node->rawname = NULL;
    }
    if (NULL != node->aliases) {
        PMIx_Argv_free(node->aliases);
        node->aliases = NULL;
    }
    /* do NOT destroy the topology */

    /* release the attributes */
    PMIX_LIST_DESTRUCT(&node->attributes);
}

PMIX_CLASS_INSTANCE(dsched_node_t, pmix_list_item_t,
                    dsched_node_construct, dsched_node_destruct);


static void scon(dsched_shift_caddy_t *p)
{
    p->info = NULL;
    p->ninfo = 0;
    p->toolcbfunc = NULL;
    p->cbdata = NULL;
    PMIx_Load_procid(&p->target, NULL, PMIX_RANK_INVALID);
    p->uid = 0;
    p->gid = 0;
    p->pid = 0;
    p->flag = false;
    p->hostname = NULL;
    p->cmdline = NULL;
    p->launcher = false;
    p->scheduler = false;
    p->queries = NULL;
    p->nqueries = 0;
    p->infocbfunc = NULL;
}
PMIX_CLASS_INSTANCE(dsched_shift_caddy_t,
                    pmix_object_t,
                    scon, NULL);
