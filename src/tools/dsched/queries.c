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

#include <pmix.h>

#include "src/threads/pmix_threads.h"
#include "src/util/pmix_argv.h"
#include "src/util/pmix_os_path.h"
#include "src/util/pmix_path.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_globals.h"
#include "src/tools/dsched/dsched.h"

static void qrel(void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t *) cbdata;
    if (NULL != cd->info) {
        PMIX_INFO_FREE(cd->info, cd->ninfo);
    }
    PMIX_RELEASE(cd);
}

static void _query(int sd, short args, void *cbdata)
{
    dsched_shift_caddy_t *cd = (dsched_shift_caddy_t *) cbdata;
    dsched_shift_caddy_t *rcd;
    pmix_query_t *q;
    pmix_status_t ret = PMIX_SUCCESS;
    void *results;
    dsched_node_t *node = NULL;
    int k, rc;
    size_t m, n, p;
    uint32_t nodeid;
    dsched_topology_t *topo;
    char *xmlbuffer = NULL;
    int len;
    char *hostname;
    pmix_data_array_t dry;
    DSCHED_HIDE_UNUSED_PARAMS(sd, args);

    PMIX_ACQUIRE_OBJECT(cd);

    pmix_output_verbose(2, dsched_globals.pmix_output,
                        "%s processing query",
                        PMIX_NAME_PRINT(&dsched_globals.myid));

    results = PMIx_Info_list_start();

    /* see what they wanted */
    for (m = 0; m < cd->nqueries; m++) {
        q = &cd->queries[m];
        hostname = NULL;
        nodeid = UINT32_MAX;

        /* see if they provided any qualifiers */
        if (NULL != q->qualifiers && 0 < q->nqual) {
            for (n = 0; n < q->nqual; n++) {
                pmix_output_verbose(2, dsched_globals.pmix_output,
                                    "%s qualifier key \"%s\" : value \"%s\"",
                                    PMIX_NAME_PRINT(&dsched_globals.myid), q->qualifiers[n].key,
                                    (q->qualifiers[n].value.type == PMIX_STRING
                                         ? q->qualifiers[n].value.data.string
                                         : "(not a string)"));

                if (PMIX_CHECK_KEY(&q->qualifiers[n], PMIX_HOSTNAME)) {
                    hostname = q->qualifiers[n].value.data.string;

                } else if (PMIX_CHECK_KEY(&q->qualifiers[n], PMIX_NODEID)) {
                    ret = PMIx_Value_get_number(&q->qualifiers[n].value, &nodeid, PMIX_UINT32);
                    if (PMIX_SUCCESS != ret) {
                        goto done;
                    }
                }
            }
            if (NULL != hostname || UINT32_MAX != nodeid) {
                node = dsched_node_match(NULL, hostname, nodeid);
                if (NULL == node) {
                    if (UINT32_MAX != nodeid) {
                        pmix_output_verbose(2, dsched_globals.pmix_output,
                                            "%s qualifier key \"%s\" : value \"%u\" is an unknown nodeID",
                                            PMIX_NAME_PRINT(&dsched_globals.myid),
                                            PMIx_Get_attribute_name(q->qualifiers[n].key),
                                            nodeid);
                     } else {
                        pmix_output_verbose(2, dsched_globals.pmix_output,
                                            "%s qualifier key \"%s\" : value \"%s\" is an unknown node",
                                            PMIX_NAME_PRINT(&dsched_globals.myid),
                                            PMIx_Get_attribute_name(q->qualifiers[n].key),
                                            hostname);
                    }
                    ret = PMIX_ERR_BAD_PARAM;
                    goto done;
                }
            }
        }

        for (n = 0; NULL != q->keys[n]; n++) {
            pmix_output_verbose(2, dsched_globals.pmix_output,
                                "%s processing key %s",
                                PMIX_NAME_PRINT(&dsched_globals.myid), q->keys[n]);

            if (0 == strcmp(q->keys[n], PMIX_HWLOC_XML_V1)) {
                // need to have been given a node qualifier
                if (NULL == node) {
                    ret = PMIX_ERR_BAD_PARAM;
                    goto done;
                }
                // find node's topology
                topo = node->topology;
                if (NULL == topo || NULL == topo->topo) {
                    ret = PMIX_ERR_BAD_PARAM;
                    goto done;
                }
                // convert its topology
                /* get it from the v2 API */
                if (0 != hwloc_topology_export_xmlbuffer(topo->topo, &xmlbuffer, &len,
                                                         HWLOC_TOPOLOGY_EXPORT_XML_FLAG_V1)) {
                    continue;
                }
                rc = PMIx_Info_list_add(results, PMIX_HWLOC_XML_V1, xmlbuffer, PMIX_STRING);
                free(xmlbuffer);
                if (PMIX_SUCCESS != rc) {
                    PMIX_ERROR_LOG(rc);
                    ret = rc;
                    goto done;
                }

            } else if (0 == strcmp(q->keys[n], PMIX_HWLOC_XML_V2)) {
                // need to have been given a node qualifier
                if (NULL == node) {
                    ret = PMIX_ERR_BAD_PARAM;
                    goto done;
                }
                // find node's topology
                topo = node->topology;
                if (NULL == topo || NULL == topo->topo) {
                    ret = PMIX_ERR_BAD_PARAM;
                    goto done;
                }
                // convert its topology
                if (0 != hwloc_topology_export_xmlbuffer(topo->topo, &xmlbuffer, &len, 0)) {
                    continue;
                }
                rc = PMIx_Info_list_add(results, PMIX_HWLOC_XML_V2, xmlbuffer, PMIX_STRING);
                free(xmlbuffer);
                if (PMIX_SUCCESS != rc) {
                    PMIX_ERROR_LOG(rc);
                    ret = rc;
                    goto done;
                }


            } else if (0 == strcmp(q->keys[n], PMIX_QUERY_ALLOCATION)) {
                /* collect all the node info */
                void *nodelist, *nodeinfolist;
                char *str;

                nodelist = PMIx_Info_list_start();
                p = 0;
                for (k=0; k < dsched_globals.nodes.size; k++) {
                    node = (dsched_node_t*)pmix_pointer_array_get_item(&dsched_globals.nodes, k);
                    if (NULL == node) {
                        continue;
                    }
                    nodeinfolist = PMIx_Info_list_start();
                    /* start with the node name */
                    rc = PMIx_Info_list_add(nodeinfolist, PMIX_HOSTNAME, node->name, PMIX_STRING);
                    /* add any aliases */
                    if (NULL != node->aliases) {
                        str = PMIx_Argv_join(node->aliases, ',');
                        rc = PMIx_Info_list_add(nodeinfolist, PMIX_HOSTNAME_ALIASES, str, PMIX_STRING);
                        free(str);
                    }
                    /* add topology index */
                    rc = PMIx_Info_list_add(nodeinfolist, PMIX_TOPOLOGY_INDEX, &node->topology->index, PMIX_INT);
                    /* convert to array */
                    rc = PMIx_Info_list_convert(nodeinfolist, &dry);
                    PMIx_Info_list_release(nodeinfolist);
                    /* now add the entry to the main list */
                    rc = PMIx_Info_list_add(nodelist, PMIX_NODE_INFO, &dry, PMIX_DATA_ARRAY);
                    ++p;
                    PMIX_DATA_ARRAY_DESTRUCT(&dry);
                }
                /* add topology info */
                for (k=0; k < dsched_globals.topologies.size; k++) {
                    topo = (dsched_topology_t*)pmix_pointer_array_get_item(&dsched_globals.topologies, k);
                    if (NULL == topo) {
                        continue;
                    }
                    /* convert the topology to XML representation */
                    if (0 != hwloc_topology_export_xmlbuffer(topo->topo, &str, &len, 0)) {
                        continue;
                    }
                    rc = PMIx_Info_list_add(nodelist, PMIX_HWLOC_XML_V2, str, PMIX_STRING);
                    free(str);
                }
                /* convert list to array */
                rc = PMIx_Info_list_convert(nodelist, &dry);
                PMIx_Info_list_release(nodelist);
                /* add to results */
                rc = PMIx_Info_list_add(nodelist, PMIX_QUERY_ALLOCATION, &dry, PMIX_DATA_ARRAY);
                PMIX_DATA_ARRAY_DESTRUCT(&dry);
                if (PMIX_SUCCESS != rc) {
                    PMIX_ERROR_LOG(rc);
                    goto done;
                }

            } else {
                fprintf(stderr, "Query for unrecognized attribute: %s\n", q->keys[n]);
            }
        } // for
    }     // for

done:
    rcd = PMIX_NEW(dsched_shift_caddy_t);
    rc = PMIx_Info_list_convert(results, &dry);
    if (PMIX_SUCCESS != rc) {
        PMIX_ERROR_LOG(rc);
        ret = rc;
    }
    PMIx_Info_list_release(results);
    if (PMIX_SUCCESS == ret) {
        if (0 == dry.size) {
            ret = PMIX_ERR_NOT_FOUND;
        } else {
            if (dry.size < cd->ninfo) {
                ret = PMIX_QUERY_PARTIAL_SUCCESS;
            } else {
                ret = PMIX_SUCCESS;
            }
        }
    }
    rcd->ninfo = dry.size;
    rcd->info = (pmix_info_t*)dry.array;
    // memory allocated in the data array will be free'd when rcd is released
    cd->infocbfunc(ret, rcd->info, rcd->ninfo, cd->cbdata, qrel, rcd);
    PMIX_RELEASE(cd);
}

pmix_status_t dsched_query_fn(pmix_proc_t *proct,
                              pmix_query_t *queries, size_t nqueries,
                              pmix_info_cbfunc_t cbfunc, void *cbdata)
{
    dsched_shift_caddy_t *cd;

    if (NULL == queries || NULL == cbfunc) {
        return PMIX_ERR_BAD_PARAM;
    }

    /* need to threadshift this request */
    cd = PMIX_NEW(dsched_shift_caddy_t);
    memcpy(&cd->target, proct, sizeof(pmix_proc_t));
    cd->queries = queries;
    cd->nqueries = nqueries;
    cd->infocbfunc = cbfunc;
    cd->cbdata = cbdata;
    DSCHED_THREADSHIFT(cd, dsched_globals.evbase, _query);

    return PMIX_SUCCESS;
}
