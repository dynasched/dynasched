/*
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2016      Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * Copyright (c) 2021      The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef DSCHED_ATTRS_H
#define DSCHED_ATTRS_H

#include "dsched_config.h"
#include "dsched_types.h"

typedef uint16_t dsched_node_flags_t;
#define DSCHED_NODE_NON_USABLE 0x0001

typedef uint16_t dsched_attribute_key_t;
typedef struct {
    pmix_list_item_t super;     /* required for this to be on lists */
    dsched_attribute_key_t key; /* key identifier */
    pmix_value_t data;
} dsched_attribute_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_attribute_t);

/*** FLAG OPS ***/
#define DSCHED_FLAG_SET(p, f)   ((p)->flags |= (f))
#define DSCHED_FLAG_UNSET(p, f) ((p)->flags &= ~(f))
#define DSCHED_FLAG_TEST(p, f)  ((p)->flags & (f))

DSCHED_EXPORT const char *dsched_attr_key_to_str(dsched_attribute_key_t key);

/* Retrieve the named attribute from a list */
DSCHED_EXPORT bool dsched_get_attribute(pmix_list_t *attributes, dsched_attribute_key_t key, void **data,
                                        pmix_data_type_t type);

/* Set the named attribute in a list, overwriting any prior entry */
DSCHED_EXPORT int dsched_set_attribute(pmix_list_t *attributes, dsched_attribute_key_t key,
                                       void *data, pmix_data_type_t type);

/* Remove the named attribute from a list */
DSCHED_EXPORT void dsched_remove_attribute(pmix_list_t *attributes, dsched_attribute_key_t key);

DSCHED_EXPORT dsched_attribute_t *dsched_fetch_attribute(pmix_list_t *attributes, dsched_attribute_t *prev,
                                                         dsched_attribute_key_t key);

DSCHED_EXPORT int dsched_prepend_attribute(pmix_list_t *attributes, dsched_attribute_key_t key,
                                           void *data, pmix_data_type_t type);

DSCHED_EXPORT int dsched_attr_load(dsched_attribute_t *kv, void *data, pmix_data_type_t type);

DSCHED_EXPORT pmix_status_t dsched_attr_unload(dsched_attribute_t *kv, void **data, pmix_data_type_t type);

DSCHED_EXPORT char *dsched_attr_print_list(pmix_list_t *attributes);

/** FOR DIAGNOSTIC PURPOSES **/
#define DSCHED_SHOW_ATTRS(a)                                                          \
    do {                                                                            \
        char *_output = dsched_attr_print_list((a));                                  \
        fprintf(stderr, "[%s:%s:%d]\n%s\n", __FILE__, __func__, __LINE__, _output); \
        free(_output);                                                              \
    } while (0)
#endif

// forward declarations
struct dsched_node_t;

DSCHED_EXPORT char* dsched_print_node_flags(struct dsched_node_t *p);
