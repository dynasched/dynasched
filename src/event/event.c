/*
 * Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2017      Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "dsched_config.h"

#include "src/include/dsched_constants.h"
#include "src/include/dsched_globals.h"
#include "src/event/event-internal.h"

/*
 * Globals
 */
static bool initialized = false;

int dsched_event_base_open(void)
{
    if (initialized) {
        return DSCHED_SUCCESS;
    }

    /* Declare our intent to use threads */
    dsched_event_use_threads();

    /* get our event base */
    if (NULL == (dsched_globals.evbase = event_base_new())) {
        return DSCHED_ERROR;
    }

    /* set the number of priorities */
    if (1 < dsched_globals.evpri) {
        event_base_priority_init(dsched_globals.evbase, dsched_globals.evpri);
    }

    initialized = true;
    return DSCHED_SUCCESS;
}

int dsched_event_base_close(void)
{
    if (!initialized) {
        return DSCHED_SUCCESS;
    }
    dsched_event_base_free(dsched_globals.evbase);

    initialized = false;
    return DSCHED_SUCCESS;
}

dsched_event_t *dsched_event_alloc(void)
{
    dsched_event_t *ev;

    ev = (dsched_event_t *) malloc(sizeof(dsched_event_t));
    return ev;
}

int dsched_event_assign(struct event *ev, dsched_event_base_t *evbase, int fd, short arg,
                      event_callback_fn cbfn, void *cbd)
{
    event_assign(ev, evbase, fd, arg, cbfn, cbd);
    return 0;
}

PMIX_CLASS_INSTANCE(dsched_event_list_item_t, pmix_list_item_t, NULL, NULL);
