/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2010 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2009      Sun Microsystems, Inc.  All rights reserved.
 * Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2013      Mellanox Technologies, Inc.
 *                         All rights reserved.
 * Copyright (c) 2015-2017 Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2015-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * This file is included at the bottom of dsched_config.h, and is
 * therefore a) after all the #define's that were output from
 * configure, and b) included in most/all files in DSCHED.
 *
 * Since this file is *only* ever included by dsched_config.h, and
 * dsched_config.h already has #ifndef/#endif protection, there is no
 * need to #ifndef/#endif protection here.
 */

#ifndef DSCHED_CONFIG_H
#    error "dsched_config_bottom.h should only be included from dsched_config.h"
#endif

/*
 * If we build a static library, Visual C define the _LIB symbol. In the
 * case of a shared library _USERDLL get defined.
 *
 * DSCHED_BUILDING and _LIB define how dsched_config.h
 * handles configuring all of DSCHED's "compatibility" code.  Both
 * constants will always be defined by the end of dsched_config.h.
 *
 * DSCHED_BUILDING affects how much compatibility code is included by
 * dsched_config.h.  It will always be 1 or 0.  The user can set the
 * value before including dsched_config.h and it will be
 * respected.
 */
#ifndef DSCHED_BUILDING
#    define DSCHED_BUILDING 1
#endif

/*
 * Flex is trying to include the unistd.h file. As there is no configure
 * option or this, the flex generated files will try to include the file
 * even on platforms without unistd.h. Therefore, if we
 * know this file is not available, we can prevent flex from including it.
 */
#ifndef HAVE_UNISTD_H
#    define YY_NO_UNISTD_H
#endif

/***********************************************************************
 *
 * code that should be in dsched_config_bottom.h regardless of build
 * status
 *
 **********************************************************************/

/*
 * BEGIN_C_DECLS should be used at the beginning of your declarations,
 * so that C++ cdschedlers don't mangle their names.  Use END_C_DECLS at
 * the end of C declarations.
 */
#undef BEGIN_C_DECLS
#undef END_C_DECLS
#if defined(c_plusplus) || defined(__cplusplus)
#    define BEGIN_C_DECLS extern "C" {
#    define END_C_DECLS   }
#else
#    define BEGIN_C_DECLS /* empty */
#    define END_C_DECLS   /* empty */
#endif

/**
 * The attribute definition should be included before any potential
 * usage.
 */
#if DSCHED_HAVE_ATTRIBUTE_ALIGNED
#    define __dsched_attribute_aligned__(a)  __attribute__((__aligned__(a)))
#    define __dsched_attribute_aligned_max__ __attribute__((__aligned__))
#else
#    define __dsched_attribute_aligned__(a)
#    define __dsched_attribute_aligned_max__
#endif

#if DSCHED_HAVE_ATTRIBUTE_ALWAYS_INLINE
#    define __dsched_attribute_always_inline__ __attribute__((__always_inline__))
#else
#    define __dsched_attribute_always_inline__
#endif

#if DSCHED_HAVE_ATTRIBUTE_COLD
#    define __dsched_attribute_cold__ __attribute__((__cold__))
#else
#    define __dsched_attribute_cold__
#endif

#if DSCHED_HAVE_ATTRIBUTE_CONST
#    define __dsched_attribute_const__ __attribute__((__const__))
#else
#    define __dsched_attribute_const__
#endif

#if DSCHED_HAVE_ATTRIBUTE_DEPRECATED
#    define __dsched_attribute_deprecated__ __attribute__((__deprecated__))
#else
#    define __dsched_attribute_deprecated__
#endif

#if DSCHED_HAVE_ATTRIBUTE_FORMAT
#    define __dsched_attribute_format__(a, b, c) __attribute__((__format__(a, b, c)))
#else
#    define __dsched_attribute_format__(a, b, c)
#endif

/* Use this __atribute__ on function-ptr declarations, only */
#if DSCHED_HAVE_ATTRIBUTE_FORMAT_FUNCPTR
#    define __dsched_attribute_format_funcptr__(a, b, c) __attribute__((__format__(a, b, c)))
#else
#    define __dsched_attribute_format_funcptr__(a, b, c)
#endif

#if DSCHED_HAVE_ATTRIBUTE_HOT
#    define __dsched_attribute_hot__ __attribute__((__hot__))
#else
#    define __dsched_attribute_hot__
#endif

#if DSCHED_HAVE_ATTRIBUTE_MALLOC
#    define __dsched_attribute_malloc__ __attribute__((__malloc__))
#else
#    define __dsched_attribute_malloc__
#endif

#if DSCHED_HAVE_ATTRIBUTE_MAY_ALIAS
#    define __dsched_attribute_may_alias__ __attribute__((__may_alias__))
#else
#    define __dsched_attribute_may_alias__
#endif

#if DSCHED_HAVE_ATTRIBUTE_NO_INSTRUMENT_FUNCTION
#    define __dsched_attribute_no_instrument_function__ __attribute__((__no_instrument_function__))
#else
#    define __dsched_attribute_no_instrument_function__
#endif

#if DSCHED_HAVE_ATTRIBUTE_NOINLINE
#    define __dsched_attribute_noinline__ __attribute__((__noinline__))
#else
#    define __dsched_attribute_noinline__
#endif

#if DSCHED_HAVE_ATTRIBUTE_NONNULL
#    define __dsched_attribute_nonnull__(a)  __attribute__((__nonnull__(a)))
#    define __dsched_attribute_nonnull_all__ __attribute__((__nonnull__))
#else
#    define __dsched_attribute_nonnull__(a)
#    define __dsched_attribute_nonnull_all__
#endif

#if DSCHED_HAVE_ATTRIBUTE_NORETURN
#    define __dsched_attribute_noreturn__ __attribute__((__noreturn__))
#else
#    define __dsched_attribute_noreturn__
#endif

/* Use this __atribute__ on function-ptr declarations, only */
#if DSCHED_HAVE_ATTRIBUTE_NORETURN_FUNCPTR
#    define __dsched_attribute_noreturn_funcptr__ __attribute__((__noreturn__))
#else
#    define __dsched_attribute_noreturn_funcptr__
#endif

#if DSCHED_HAVE_ATTRIBUTE_PACKED
#    define __dsched_attribute_packed__ __attribute__((__packed__))
#else
#    define __dsched_attribute_packed__
#endif

#if DSCHED_HAVE_ATTRIBUTE_PURE
#    define __dsched_attribute_pure__ __attribute__((__pure__))
#else
#    define __dsched_attribute_pure__
#endif

#if DSCHED_HAVE_ATTRIBUTE_SENTINEL
#    define __dsched_attribute_sentinel__ __attribute__((__sentinel__))
#else
#    define __dsched_attribute_sentinel__
#endif

#if DSCHED_HAVE_ATTRIBUTE_UNUSED
#    define __dsched_attribute_unused__ __attribute__((__unused__))
#else
#    define __dsched_attribute_unused__
#endif

#if DSCHED_HAVE_ATTRIBUTE_VISIBILITY
#    define __dsched_attribute_visibility__(a) __attribute__((__visibility__(a)))
#else
#    define __dsched_attribute_visibility__(a)
#endif

#if DSCHED_HAVE_ATTRIBUTE_WARN_UNUSED_RESULT
#    define __dsched_attribute_warn_unused_result__ __attribute__((__warn_unused_result__))
#else
#    define __dsched_attribute_warn_unused_result__
#endif

#if DSCHED_HAVE_ATTRIBUTE_WEAK_ALIAS
#    define __dsched_attribute_weak_alias__(a) __attribute__((__weak__, __alias__(a)))
#else
#    define __dsched_attribute_weak_alias__(a)
#endif

#if DSCHED_HAVE_ATTRIBUTE_DESTRUCTOR
#    define __dsched_attribute_destructor__ __attribute__((__destructor__))
#else
#    define __dsched_attribute_destructor__
#endif

#if DSCHED_HAVE_ATTRIBUTE_OPTNONE
#    define __dsched_attribute_optnone__ __attribute__((__optnone__))
#else
#    define __dsched_attribute_optnone__
#endif

#if DSCHED_HAVE_ATTRIBUTE_EXTENSION
#    define __dsched_attribute_extension__ __extension__
#else
#    define __dsched_attribute_extension__
#endif

#if DSCHED_C_HAVE_VISIBILITY
#    define DSCHED_EXPORT        __dsched_attribute_visibility__("default")
#    define DSCHED_MODULE_EXPORT __dsched_attribute_visibility__("default")
#else
#    define DSCHED_EXPORT
#    define DSCHED_MODULE_EXPORT
#endif

#if !defined(__STDC_LIMIT_MACROS) && (defined(c_plusplus) || defined(__cplusplus))
/* When using a C++ cdschedler, the max / min value #defines for std
   types are only included if __STDC_LIMIT_MACROS is set before
   including stdint.h */
#    define __STDC_LIMIT_MACROS
#endif
#include "dsched_config.h"
#include "dsched_stdint.h"

/***********************************************************************
 *
 * Code that is only for when building DSCHED or utilities that are
 * using the internals of DSCHED.  It should not be included when
 * building MPI applications
 *
 **********************************************************************/
#if DSCHED_BUILDING

/*
 * Maximum size of a filename path.
 */
#    include <limits.h>
#    ifdef HAVE_SYS_PARAM_H
#        include <sys/param.h>
#    endif
#    if defined(PATH_MAX)
#        define DSCHED_PATH_MAX (PATH_MAX + 1)
#    elif defined(_POSIX_PATH_MAX)
#        define DSCHED_PATH_MAX (_POSIX_PATH_MAX + 1)
#    else
#        define DSCHED_PATH_MAX 256
#    endif

/*
 * Set the cdschedle-time path-separator on this system and variable separator
 */
#    define DSCHED_PATH_SEP "/"
#    define DSCHED_ENV_SEP  ':'

#    if defined(MAXHOSTNAMELEN)
#        define DSCHED_MAXHOSTNAMELEN (MAXHOSTNAMELEN + 1)
#    elif defined(HOST_NAME_MAX)
#        define DSCHED_MAXHOSTNAMELEN (HOST_NAME_MAX + 1)
#    else
/* SUSv2 guarantees that "Host names are limited to 255 bytes". */
#        define DSCHED_MAXHOSTNAMELEN (255 + 1)
#    endif

#    define DSCHED_DEBUG_ZERO(obj)

/*
 * printf functions for portability (only when building DSCHED)
 */
#    if !defined(HAVE_VASPRINTF) || !defined(HAVE_VSNPRINTF)
#        include <stdarg.h>
#        include <stdlib.h>
#    endif

#    if !defined(HAVE_ASPRINTF) || !defined(HAVE_SNPRINTF) || !defined(HAVE_VASPRINTF) \
        || !defined(HAVE_VSNPRINTF)
#        include "src/util/pmix_printf.h"
#    endif

#    ifndef HAVE_ASPRINTF
#        define asprintf pmix_asprintf
#    endif

#    ifndef HAVE_SNPRINTF
#        define snprintf pmix_snprintf
#    endif

#    ifndef HAVE_VASPRINTF
#        define vasprintf pmix_vasprintf
#    endif

#    ifndef HAVE_VSNPRINTF
#        define vsnprintf dsched_vsnprintf
#    endif

/*
 * On some homogenous big-iron machines (Sandia's Red Storm), there
 * are no htonl and friends.  If that's the case, provide stubs.  I
 * would hope we never find a platform that doesn't have these macros
 * and would want to talk to the outside world... On other platforms
 * we fail to detect them correctly.
 */
#    if !defined(HAVE_UNIX_BYTESWAP)
static inline uint32_t htonl(uint32_t hostvar)
{
    return hostvar;
}
static inline uint32_t ntohl(uint32_t netvar)
{
    return netvar;
}
static inline uint16_t htons(uint16_t hostvar)
{
    return hostvar;
}
static inline uint16_t ntohs(uint16_t netvar)
{
    return netvar;
}
#    endif

/*
 * Define __func__-preprocessor directive if the cdschedler does not
 * already define it.  Define it to __FILE__ so that we at least have
 * a clue where the developer is trying to indicate where the error is
 * coming from (assuming that __func__ is typically used for
 * printf-style debugging).
 */
#    if defined(HAVE_DECL___FUNC__) && !HAVE_DECL___FUNC__
#        define __func__ __FILE__
#    endif

#    define IOVBASE_TYPE void

/* ensure the bool type is defined as it is used everywhere */
#    include <stdbool.h>

/**
 * If we generate our own bool type, we need a special way to cast the result
 * in such a way to keep the cdschedlers silent.
 */
#    define DSCHED_INT_TO_BOOL(VALUE) (bool) (VALUE)


#    if !HAVE_DECL_AF_UNSPEC
#        define AF_UNSPEC 0
#    endif
#    if !HAVE_DECL_PF_UNSPEC
#        define PF_UNSPEC 0
#    endif
#    if !HAVE_DECL_AF_INET6
#        define AF_INET6 AF_UNSPEC
#    endif
#    if !HAVE_DECL_PF_INET6
#        define PF_INET6 PF_UNSPEC
#    endif

#    if defined(__APPLE__) && defined(HAVE_INTTYPES_H)
/* Prior to Mac OS X 10.3, the length modifier "ll" wasn't
   supported, but "q" was for long long.  This isn't ANSI
   C and causes a warning when using PRI?64 macros.  We
   don't support versions prior to OS X 10.3, so we dont'
   need such backward compatibility.  Instead, redefine
   the macros to be "ll", which is ANSI C and doesn't
   cause a cdschedler warning. */
#        include <inttypes.h>
#        if defined(__PRI_64_LENGTH_MODIFIER__)
#            undef __PRI_64_LENGTH_MODIFIER__
#            define __PRI_64_LENGTH_MODIFIER__ "ll"
#        endif
#        if defined(__SCN_64_LENGTH_MODIFIER__)
#            undef __SCN_64_LENGTH_MODIFIER__
#            define __SCN_64_LENGTH_MODIFIER__ "ll"
#        endif
#    endif

/* If we're in C++, then just undefine restrict and then define it to
   nothing.  "restrict" is not part of the C++ language, and we don't
   have a corresponding AC_CXX_RESTRICT to figure out what the C++
   cdschedler supports. */
#    if defined(c_plusplus) || defined(__cplusplus)
#        undef restrict
#        define restrict
#    endif

#else

/* For a similar reason to what is listed in dsched_config_top.h, we
   want to protect others from the autoconf/automake-generated
   PACKAGE_<foo> macros in dsched_config.h.  We can't put these undef's
   directly in dsched_config.h because they'll be turned into #defines'
   via autoconf.

   So put them here in case any only else includes DSCHED/DSCHED's
   config.h files. */

#    undef PACKAGE_BUGREPORT
#    undef PACKAGE_NAME
#    undef PACKAGE_STRING
#    undef PACKAGE_TARNAME
#    undef PACKAGE_VERSION
#    undef PACKAGE_URL
#    undef HAVE_CONFIG_H

#endif /* DSCHED_BUILDING */
