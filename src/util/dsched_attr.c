/*
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2014-2017 Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2018-2020 Cisco Systems, Inc.  All rights reserved
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

#include "dsched_config.h"
#include "dsched_constants.h"
#include "dsched_types.h"

#include <pmix_common.h>

#include "src/util/pmix_argv.h"
#include "src/util/pmix_error.h"
#include "src/util/pmix_output.h"
#include "src/util/pmix_printf.h"
#include "src/util/pmix_string_copy.h"

#include "src/include/dsched_globals.h"
#include "src/util/dsched_attr.h"
#include "src/util/dsched_error.h"

bool dsched_get_attribute(pmix_list_t *attributes, dsched_attribute_key_t key, void **data,
                        pmix_data_type_t type)
{
    dsched_attribute_t *kv;
    int rc;

    PMIX_LIST_FOREACH(kv, attributes, dsched_attribute_t)
    {
        if (key == kv->key) {
            if (kv->data.type != type) {
                DSCHED_ERROR_LOG(DSCHED_ERR_TYPE_MISMATCH);
                pmix_output(0, "KV %s TYPE %s", PMIx_Data_type_string(kv->data.type), PMIx_Data_type_string(type));
                return false;
            }
            if (NULL != data) {
                if (DSCHED_SUCCESS != (rc = dsched_attr_unload(kv, data, type))) {
                    DSCHED_ERROR_LOG(rc);
                }
            }
            return true;
        }
    }
    /* not found */
    return false;
}

int dsched_set_attribute(pmix_list_t *attributes, dsched_attribute_key_t key,
                         void *data, pmix_data_type_t type)
{
    dsched_attribute_t *kv;
    bool *bl, bltrue = true;
    int rc;

    PMIX_LIST_FOREACH(kv, attributes, dsched_attribute_t)
    {
        if (key == kv->key) {
            if (kv->data.type != type) {
                return DSCHED_ERR_TYPE_MISMATCH;
            }
            if (PMIX_BOOL == type) {
                if (NULL == data) {
                    bl = &bltrue;
                } else {
                    bl = (bool*)data;
                }
                if (false == *bl) {
                    pmix_list_remove_item(attributes, &kv->super);
                    PMIX_RELEASE(kv);
                    return DSCHED_SUCCESS;
                }
            }
            if (DSCHED_SUCCESS != (rc = dsched_attr_load(kv, data, type))) {
                DSCHED_ERROR_LOG(rc);
            }
            return rc;
        }
    }
    /* not found - add it */
    kv = PMIX_NEW(dsched_attribute_t);
    kv->key = key;
    if (DSCHED_SUCCESS != (rc = dsched_attr_load(kv, data, type))) {
        PMIX_RELEASE(kv);
        return rc;
    }
    pmix_list_append(attributes, &kv->super);
    return DSCHED_SUCCESS;
}

dsched_attribute_t *dsched_fetch_attribute(pmix_list_t *attributes, dsched_attribute_t *prev,
                                           dsched_attribute_key_t key)
{
    dsched_attribute_t *kv, *end, *next;

    /* if prev is NULL, then find the first attr on the list
     * that matches the key */
    if (NULL == prev) {
        PMIX_LIST_FOREACH(kv, attributes, dsched_attribute_t)
        {
            if (key == kv->key) {
                return kv;
            }
        }
        /* if we get, then the key isn't on the list */
        return NULL;
    }

    /* if we are at the end of the list, then nothing to do */
    end = (dsched_attribute_t *) pmix_list_get_end(attributes);
    if (prev == end || end == (dsched_attribute_t *) pmix_list_get_next(&prev->super)
        || NULL == pmix_list_get_next(&prev->super)) {
        return NULL;
    }

    /* starting with the next item on the list, search
     * for the next attr with the matching key */
    next = (dsched_attribute_t *) pmix_list_get_next(&prev->super);
    while (NULL != next) {
        if (next->key == key) {
            return next;
        }
        next = (dsched_attribute_t *) pmix_list_get_next(&next->super);
    }

    /* if we get here, then no matching key was found */
    return NULL;
}

int dsched_prepend_attribute(pmix_list_t *attributes, dsched_attribute_key_t key,
                             void *data, pmix_data_type_t type)
{
    dsched_attribute_t *kv;
    int rc;

    kv = PMIX_NEW(dsched_attribute_t);
    kv->key = key;
    if (DSCHED_SUCCESS != (rc = dsched_attr_load(kv, data, type))) {
        PMIX_RELEASE(kv);
        return rc;
    }
    pmix_list_prepend(attributes, &kv->super);
    return DSCHED_SUCCESS;
}

void dsched_remove_attribute(pmix_list_t *attributes, dsched_attribute_key_t key)
{
    dsched_attribute_t *kv;

    PMIX_LIST_FOREACH(kv, attributes, dsched_attribute_t)
    {
        if (key == kv->key) {
            pmix_list_remove_item(attributes, &kv->super);
            PMIX_RELEASE(kv);
            return;
        }
    }
}

char *dsched_attr_print_list(pmix_list_t *attributes)
{
    char *out1, **cache = NULL;
    dsched_attribute_t *attr;

    PMIX_LIST_FOREACH(attr, attributes, dsched_attribute_t)
    {
        PMIx_Argv_append_nosize(&cache, dsched_attr_key_to_str(attr->key));
    }
    if (NULL != cache) {
        out1 = PMIx_Argv_join(cache, '\n');
        PMIx_Argv_free(cache);
    } else {
        out1 = NULL;
    }
    return out1;
}

int dsched_attr_load(dsched_attribute_t *kv, void *data, pmix_data_type_t type)
{
    pmix_byte_object_t *boptr;
    struct timeval *tv;
    pmix_envar_t *envar;
    pmix_status_t rc;

    kv->data.type = type;
    if (NULL == data) {
        /* if the type is BOOL, then the user wanted to
         * use the presence of the attribute to indicate
         * "true" - so let's mark it that way just in
         * case a subsequent test looks for the value */
        if (PMIX_BOOL == type) {
            kv->data.data.flag = true;
        } else {
            /* otherwise, check to see if this type has storage
             * that is already allocated, and free it if so */
            if (PMIX_STRING == type && NULL != kv->data.data.string) {
                free(kv->data.data.string);
            } else if (PMIX_BYTE_OBJECT == type && NULL != kv->data.data.bo.bytes) {
                free(kv->data.data.bo.bytes);
            } else if (PMIX_PROC_NSPACE == type && NULL != kv->data.data.proc) {
                PMIX_PROC_FREE(kv->data.data.proc, 1);
            } else if (PMIX_PROC == type && NULL != kv->data.data.proc) {
                PMIX_PROC_FREE(kv->data.data.proc, 1);
            } else if (PMIX_ENVAR == type) {
                if (NULL != kv->data.data.envar.envar) {
                    free(kv->data.data.envar.envar);
                }
                if (NULL != kv->data.data.envar.value) {
                    free(kv->data.data.envar.value);
                }
            }
            /* just set the fields to zero */
            memset(&kv->data.data, 0, sizeof(kv->data.data));
        }
        return DSCHED_SUCCESS;
    }

    switch (type) {
    case PMIX_BOOL:
        kv->data.data.flag = *(bool *) (data);
        break;
    case PMIX_BYTE:
        kv->data.data.byte = *(uint8_t *) (data);
        break;
    case PMIX_STRING:
        if (NULL != kv->data.data.string) {
            free(kv->data.data.string);
        }
        kv->data.data.string = strdup((const char *) data);
        break;
    case PMIX_SIZE:
        kv->data.data.size = *(size_t *) (data);
        break;
    case PMIX_PID:
        kv->data.data.pid = *(pid_t *) (data);
        break;

    case PMIX_INT:
        kv->data.data.integer = *(int *) (data);
        break;
    case PMIX_INT8:
        kv->data.data.int8 = *(int8_t *) (data);
        break;
    case PMIX_INT16:
        kv->data.data.int16 = *(int16_t *) (data);
        break;
    case PMIX_INT32:
        kv->data.data.int32 = *(int32_t *) (data);
        break;
    case PMIX_INT64:
        kv->data.data.int64 = *(int64_t *) (data);
        break;

    case PMIX_UINT:
        kv->data.data.uint = *(unsigned int *) (data);
        break;
    case PMIX_UINT8:
        kv->data.data.uint8 = *(uint8_t *) (data);
        break;
    case PMIX_UINT16:
        kv->data.data.uint16 = *(uint16_t *) (data);
        break;
    case PMIX_UINT32:
        kv->data.data.uint32 = *(uint32_t *) data;
        break;
    case PMIX_UINT64:
        kv->data.data.uint64 = *(uint64_t *) (data);
        break;

    case PMIX_BYTE_OBJECT:
        if (NULL != kv->data.data.bo.bytes) {
            free(kv->data.data.bo.bytes);
        }
        boptr = (pmix_byte_object_t *) data;
        if (NULL != boptr && NULL != boptr->bytes && 0 < boptr->size) {
            kv->data.data.bo.bytes = (char *) malloc(boptr->size);
            memcpy(kv->data.data.bo.bytes, boptr->bytes, boptr->size);
            kv->data.data.bo.size = boptr->size;
        } else {
            kv->data.data.bo.bytes = NULL;
            kv->data.data.bo.size = 0;
        }
        break;

    case PMIX_FLOAT:
        kv->data.data.fval = *(float *) (data);
        break;

    case PMIX_TIMEVAL:
        tv = (struct timeval *) data;
        kv->data.data.tv.tv_sec = tv->tv_sec;
        kv->data.data.tv.tv_usec = tv->tv_usec;
        break;

    case PMIX_POINTER:
        kv->data.data.ptr = data;
        break;

    case PMIX_PROC_RANK:
        kv->data.data.rank = *(pmix_rank_t *) data;
        break;

    case PMIX_PROC_NSPACE:
        if (NULL == kv->data.data.proc) {
            PMIX_PROC_CREATE(kv->data.data.proc, 1);
            if (NULL == kv->data.data.proc) {
                return DSCHED_ERR_OUT_OF_RESOURCE;
            }
        }
        PMIX_LOAD_NSPACE(kv->data.data.proc->nspace, (char *) data);
        break;

    case PMIX_PROC:
        if (NULL == kv->data.data.proc) {
            PMIX_PROC_CREATE(kv->data.data.proc, 1);
            if (NULL == kv->data.data.proc) {
                return DSCHED_ERR_OUT_OF_RESOURCE;
            }
        }
        PMIX_XFER_PROCID(kv->data.data.proc, (pmix_proc_t *) data);
        break;

    case PMIX_ENVAR:
        envar = (pmix_envar_t *) data;
        if (NULL != envar) {
            if (NULL != kv->data.data.envar.envar) {
                free(kv->data.data.envar.envar);
            }
            if (NULL != kv->data.data.envar.value) {
                free(kv->data.data.envar.value);
            }
            if (NULL != envar->envar) {
                kv->data.data.envar.envar = strdup(envar->envar);
            }
            if (NULL != envar->value) {
                kv->data.data.envar.value = strdup(envar->value);
            }
            kv->data.data.envar.separator = envar->separator;
        }
        break;

    case PMIX_DATA_ARRAY:
        rc = PMIx_Data_copy((void**)&kv->data.data.darray, data, PMIX_DATA_ARRAY);
        return rc;
        break;

    default:
        DSCHED_ERROR_LOG(DSCHED_ERR_NOT_SUPPORTED);
        return DSCHED_ERR_NOT_SUPPORTED;
    }
    return DSCHED_SUCCESS;
}

pmix_status_t dsched_attr_unload(dsched_attribute_t *kv, void **data, pmix_data_type_t type)
{
    pmix_byte_object_t *boptr;
    pmix_envar_t *envar;
    pmix_data_array_t *darray;
    pmix_status_t rc;
    pmix_data_type_t pointers[] = {
        PMIX_STRING,
        PMIX_BYTE_OBJECT,
        PMIX_POINTER,
        PMIX_PROC_NSPACE,
        PMIX_PROC,
        PMIX_ENVAR,
        PMIX_DATA_ARRAY,
        PMIX_UNDEF};
    int n;
    bool found = false;

    if (type != kv->data.type) {
        return PMIX_ERR_TYPE_MISMATCH;
    }
    if (NULL == data) {
        PMIX_ERROR_LOG(PMIX_ERR_BAD_PARAM);
        return PMIX_ERR_BAD_PARAM;
    }
    /* if they didn't give us a storage address
     * and the data type isn't one where we can
     * create storage, then this is an error */
    for (n = 0; PMIX_UNDEF != pointers[n]; n++) {
        if (type == pointers[n]) {
            found = true;
            break;
        }
    }
    if (!found && NULL == *data) {
        PMIX_ERROR_LOG(PMIX_ERR_BAD_PARAM);
        return PMIX_ERR_BAD_PARAM;
    }

    switch (type) {
    case PMIX_BOOL:
        memcpy(*data, &kv->data.data.flag, sizeof(bool));
        break;
    case PMIX_BYTE:
        memcpy(*data, &kv->data.data.byte, sizeof(uint8_t));
        break;
    case PMIX_STRING:
        if (NULL != kv->data.data.string) {
            *data = strdup(kv->data.data.string);
        } else {
            *data = NULL;
        }
        break;
    case PMIX_SIZE:
        memcpy(*data, &kv->data.data.size, sizeof(size_t));
        break;
    case PMIX_PID:
        memcpy(*data, &kv->data.data.pid, sizeof(pid_t));
        break;

    case PMIX_INT:
        memcpy(*data, &kv->data.data.integer, sizeof(int));
        break;
    case PMIX_INT8:
        memcpy(*data, &kv->data.data.int8, sizeof(int8_t));
        break;
    case PMIX_INT16:
        memcpy(*data, &kv->data.data.int16, sizeof(int16_t));
        break;
    case PMIX_INT32:
        memcpy(*data, &kv->data.data.int32, sizeof(int32_t));
        break;
    case PMIX_INT64:
        memcpy(*data, &kv->data.data.int64, sizeof(int64_t));
        break;

    case PMIX_UINT:
        memcpy(*data, &kv->data.data.uint, sizeof(unsigned int));
        break;
    case PMIX_UINT8:
        memcpy(*data, &kv->data.data.uint8, 1);
        break;
    case PMIX_UINT16:
        memcpy(*data, &kv->data.data.uint16, 2);
        break;
    case PMIX_UINT32:
        memcpy(*data, &kv->data.data.uint32, 4);
        break;
    case PMIX_UINT64:
        memcpy(*data, &kv->data.data.uint64, 8);
        break;

    case PMIX_BYTE_OBJECT:
        boptr = (pmix_byte_object_t *) malloc(sizeof(pmix_byte_object_t));
        if (NULL == boptr) {
            return PMIX_ERR_OUT_OF_RESOURCE;
        }
        if (NULL != kv->data.data.bo.bytes && 0 < kv->data.data.bo.size) {
            boptr->bytes = (char *) malloc(kv->data.data.bo.size);
            memcpy(boptr->bytes, kv->data.data.bo.bytes, kv->data.data.bo.size);
            boptr->size = kv->data.data.bo.size;
        } else {
            boptr->bytes = NULL;
            boptr->size = 0;
        }
        *data = boptr;
        break;

    case PMIX_FLOAT:
        memcpy(*data, &kv->data.data.fval, sizeof(float));
        break;

    case PMIX_TIMEVAL:
        memcpy(*data, &kv->data.data.tv, sizeof(struct timeval));
        break;

    case PMIX_POINTER:
        *data = kv->data.data.ptr;
        break;

    case PMIX_PROC_RANK:
        memcpy(*data, &kv->data.data.rank, sizeof(pmix_rank_t));
        break;

    case PMIX_PROC_NSPACE:
        PMIX_PROC_CREATE(*data, 1);
        if (NULL == *data) {
            return PMIX_ERR_OUT_OF_RESOURCE;
        }
        memcpy(*data, kv->data.data.proc->nspace, sizeof(pmix_nspace_t));
        break;

    case PMIX_PROC:
        PMIX_PROC_CREATE(*data, 1);
        if (NULL == *data) {
            return PMIX_ERR_OUT_OF_RESOURCE;
        }
        memcpy(*data, kv->data.data.proc, sizeof(pmix_proc_t));
        break;

    case PMIX_ENVAR:
        PMIX_ENVAR_CREATE(envar, 1);
        if (NULL == envar) {
            return PMIX_ERR_OUT_OF_RESOURCE;
        }
        if (NULL != kv->data.data.envar.envar) {
            envar->envar = strdup(kv->data.data.envar.envar);
        }
        if (NULL != kv->data.data.envar.value) {
            envar->value = strdup(kv->data.data.envar.value);
        }
        envar->separator = kv->data.data.envar.separator;
        *data = envar;
        break;

    case PMIX_DATA_ARRAY:
        rc = PMIx_Data_copy((void**)&darray, kv->data.data.darray, PMIX_DATA_ARRAY);
        if (PMIX_SUCCESS != rc) {
            *data = NULL;
            return rc;
        }
        *data = darray;
        break;

    default:
        PMIX_ERROR_LOG(PMIX_ERR_NOT_SUPPORTED);
        return PMIX_ERR_NOT_SUPPORTED;
    }
    return PMIX_SUCCESS;
}

const char *dsched_attr_key_to_str(dsched_attribute_key_t key)
{
    switch(key) {
        default:
            return "UNKNOWN";
    }
}

char* dsched_print_node_flags(struct dsched_node_t *ptr)
{
    dsched_node_t *p = (dsched_node_t*)ptr;
    char **tmp = NULL;
    char *ans;

    // start with the node name
    PMIx_Argv_append_nosize(&tmp, p->name);
    PMIx_Argv_append_nosize(&tmp, ": ");

    if (DSCHED_FLAG_TEST(p, DSCHED_NODE_NON_USABLE)) {
        PMIx_Argv_append_nosize(&tmp, "NONUSABLE");
    }
    ans = PMIx_Argv_join(tmp, '|');
    PMIx_Argv_free(tmp);
    return ans;
}

static void dsched_attr_cons(dsched_attribute_t *p)
{
    p->key = 0;
    memset(&p->data, 0, sizeof(p->data));
}
static void dsched_attr_des(dsched_attribute_t *p)
{
    PMIX_VALUE_DESTRUCT(&p->data);
}
PMIX_CLASS_INSTANCE(dsched_attribute_t, pmix_list_item_t,
                    dsched_attr_cons, dsched_attr_des);

