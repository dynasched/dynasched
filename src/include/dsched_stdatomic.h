/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2018      Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2019      Intel, Inc.  All rights reserved.
 * Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * Copyright (c) 2021      Amazon.com, Inc. or its affiliates.  All Rights
 *                         reserved.
 * Copyright (c) 2023      Triad National Security, LLC. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#if !defined(DSCHED_STDATOMIC_H)
#    define DSCHED_STDATOMIC_H

#    include "dsched_stdint.h"
#    include <stdbool.h>

#    if DSCHED_ATOMIC_C11

#ifdef HAVE_STDATOMIC_H
#        include <stdatomic.h>
#endif

typedef atomic_int dsched_atomic_int_t;
typedef atomic_long dsched_atomic_long_t;

typedef _Atomic bool dsched_atomic_bool_t;
typedef _Atomic int32_t dsched_atomic_int32_t;
typedef _Atomic uint32_t dsched_atomic_uint32_t;
typedef _Atomic int64_t dsched_atomic_int64_t;
typedef _Atomic uint64_t dsched_atomic_uint64_t;

typedef _Atomic size_t dsched_atomic_size_t;
typedef _Atomic ssize_t dsched_atomic_ssize_t;
typedef _Atomic intptr_t dsched_atomic_intptr_t;
typedef _Atomic uintptr_t dsched_atomic_uintptr_t;

#    else

typedef volatile int dsched_atomic_int_t;
typedef volatile long dsched_atomic_long_t;

typedef volatile bool dsched_atomic_bool_t;
typedef volatile int32_t dsched_atomic_int32_t;
typedef volatile uint32_t dsched_atomic_uint32_t;
typedef volatile int64_t dsched_atomic_int64_t;
typedef volatile uint64_t dsched_atomic_uint64_t;

typedef volatile size_t dsched_atomic_size_t;
typedef volatile ssize_t dsched_atomic_ssize_t;
typedef volatile intptr_t dsched_atomic_intptr_t;
typedef volatile uintptr_t dsched_atomic_uintptr_t;

#    endif

#endif /* !defined(DSCHED_STDATOMIC_H) */
