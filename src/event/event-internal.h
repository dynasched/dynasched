/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2010      Oracle and/or its affiliates.  All rights reserved.
 * Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2015      Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2019      Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 *
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 */

#ifndef DSCHED_MCA_EVENT_H
#define DSCHED_MCA_EVENT_H

#include "dsched_config.h"

#ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#endif
#include <stdarg.h>
#include <stdint.h>

#include <event.h>
#include <event2/thread.h>

#include "src/class/pmix_list.h"
#include "src/util/pmix_output.h"

typedef event_callback_fn dsched_event_cbfunc_t;

BEGIN_C_DECLS

#define DSCHED_EVENT_SIGNAL(ev) dsched_event_get_signal(ev)

#define DSCHED_TIMEOUT_DEFAULT \
    {                        \
        1, 0                 \
    }

typedef struct event_base dsched_event_base_t;
typedef struct event dsched_event_t;

DSCHED_EXPORT extern dsched_event_base_t *dsched_sync_event_base;
DSCHED_EXPORT extern dsched_event_base_t *dsched_event_base;

DSCHED_EXPORT int dsched_event_base_open(void);
DSCHED_EXPORT int dsched_event_base_close(void);
DSCHED_EXPORT dsched_event_t *dsched_event_alloc(void);

#define DSCHED_EV_TIMEOUT EV_TIMEOUT
#define DSCHED_EV_READ    EV_READ
#define DSCHED_EV_WRITE   EV_WRITE
#define DSCHED_EV_SIGNAL  EV_SIGNAL
/* Persistent event: won't get removed automatically when activated. */
#define DSCHED_EV_PERSIST EV_PERSIST

#define DSCHED_EVLOOP_ONCE     EVLOOP_ONCE     /**< Block at most once. */
#define DSCHED_EVLOOP_NONBLOCK EVLOOP_NONBLOCK /**< Do not block. */

#define dsched_event_base_create() event_base_new()

#define dsched_event_base_free(x) event_base_free(x)

#define dsched_event_reinit(b) event_reinit((b))

#define dsched_event_base_init_common_timeout (b, t) event_base_init_common_timeout((b), (t))

/* thread support APIs */
#define dsched_event_use_threads() evthread_use_pthreads()
#define dsched_event_free(x)       event_free(x)
#define dsched_event_get_signal(x) event_get_signal(x)

/* Event priority APIs */
#define dsched_event_base_priority_init(b, n) event_base_priority_init((b), (n))

/* Basic event APIs */
#define dsched_event_enable_debug_mode() event_enable_debug_mode()

DSCHED_EXPORT int dsched_event_assign(struct event *ev, dsched_event_base_t *evbase, int fd, short arg,
                                      event_callback_fn cbfn, void *cbd);

#define dsched_event_set(b, x, fd, fg, cb, arg) \
    dsched_event_assign((x), (b), (fd), (fg), (event_callback_fn)(cb), (arg))

#define dsched_event_add(ev, tv)      event_add((ev), (tv))
#define dsched_event_del(ev)          event_del((ev))
#define dsched_event_active(x, y, z)  event_active((x), (y), (z))
#define dsched_event_base_loopexit(b) event_base_loopexit(b, NULL)


DSCHED_EXPORT dsched_event_t *dsched_event_new(dsched_event_base_t *b, int fd, short fg,
                                               event_callback_fn cbfn, void *cbd);

/* Timer APIs */
#define dsched_event_evtimer_new(b, cb, arg) dsched_event_new((b), -1, 0, (cb), (arg))

#define dsched_event_evtimer_add(x, tv) dsched_event_add((x), (tv))

#define dsched_event_evtimer_set(b, x, cb, arg) \
    dsched_event_assign((x), (b), -1, 0, (event_callback_fn)(cb), (arg))

#define dsched_event_evtimer_del(x) dsched_event_del((x))

#define dsched_event_evtimer_pending(x, tv) event_pending((x), EV_TIMEOUT, (tv))

#define dsched_event_evtimer_initialized(x) event_initialized((x))

/* Signal APIs */
#define dsched_event_signal_add(x, tv) event_add((x), (tv))

#define dsched_event_signal_set(b, x, fd, cb, arg) \
    dsched_event_assign((x), (b), (fd), EV_SIGNAL | EV_PERSIST, (event_callback_fn)(cb), (arg))

#define dsched_event_signal_del(x) event_del((x))

#define dsched_event_signal_pending(x, tv) event_pending((x), EV_SIGNAL, (tv))

#define dsched_event_signal_initalized(x) event_initialized((x))

#define dsched_event_loop(b, fg) event_base_loop((b), (fg))

typedef struct {
    pmix_list_item_t super;
    dsched_event_t ev;
} dsched_event_list_item_t;
DSCHED_EXPORT PMIX_CLASS_DECLARATION(dsched_event_list_item_t);

/* define a threadshift macro */
#define DSCHED_THREADSHIFT(x, eb, f)                                \
    do {                                                               \
        dsched_event_set((eb), &((x)->ev), -1, DSCHED_EV_WRITE, (f), (x)); \
        PMIX_POST_OBJECT((x));                                         \
        dsched_event_active(&((x)->ev), DSCHED_EV_WRITE, 1);               \
    } while (0)

END_C_DECLS

#endif /* DSCHED_EVENT_H_ */
