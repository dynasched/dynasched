# -*- shell-script -*-
#
# Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved
# Copyright (c) 2013      Los Alamos National Security, LLC.  All rights reserved.
# Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2017-2019 Research Organization for Information Science
#                         and Technology (RIST).  All rights reserved.
# Copyright (c) 2020      IBM Corporation.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# Copyright (c) 2021-2022 Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# DSCHED_LIBEVENT_CONFIG([action-if-found], [action-if-not-found])
# --------------------------------------------------------------------
# Attempt to find a libevent package.  If found, evaluate
# action-if-found.  Otherwise, evaluate action-if-not-found.
#
# Modifies the following in the environment:
#  * dsched_libevent_CPPFLAGS
#  * dsched_libevent_LDFLAGS
#  * dsched_libevent_LIBS
#
# Adds the following to the wrapper compilers:
#  * CPPFLAGS: none
#  * LDFLAGS: add dsched_libevent_LDFLAGS
#  * LIBS: add dsched_libevent_LIBS
AC_DEFUN([DSCHED_LIBEVENT_CONFIG],[
    DSCHED_VAR_SCOPE_PUSH([dsched_event_dir dsched_event_libdir dsched_check_libevent_save_CPPFLAGS dsched_check_libevent_save_LDFLAGS dsched_check_libevent_save_LIBS])

    AC_ARG_WITH([libevent],
                [AS_HELP_STRING([--with-libevent=DIR],
                                [Search for libevent headers and libraries in DIR ])])
    AC_ARG_WITH([libevent-libdir],
                [AS_HELP_STRING([--with-libevent-libdir=DIR],
                                [Search for libevent libraries in DIR ])])

    AS_IF([test "$with_libevent" = "no"],
          [AC_MSG_WARN([DynaSched requires libevent support])
           AC_MSG_ERROR([Cannot continue])])

    dsched_libevent_support=1
    dsched_check_libevent_save_CPPFLAGS="$CPPFLAGS"
    dsched_check_libevent_save_LDFLAGS="$LDFLAGS"
    dsched_check_libevent_save_LIBS="$LIBS"

    dnl Do not use pkg-config for libevent, because we need the pthreads interface
    dnl and the libevent_pthreads module will always pull in libevent instead of
    dnl libevent_core.
    libevent_USE_PKG_CONFIG=0
    OAC_CHECK_PACKAGE([libevent],
                      [dsched_libevent],
                      [event.h],
                      [event_core event_pthreads],
                      [event_config_new],
                      [],
                      [dsched_libevent_support=0])

    # Check to see if the above check failed because it conflicted with LSF's libevent.so
    # This can happen if LSF's library is in the LDFLAGS envar or default search
    # path. The 'event_getcode4name' function is only defined in LSF's libevent.so and not
    # in Libevent's libevent.so
    if test $dsched_libevent_support -eq 0 ; then
        AC_CHECK_LIB([event], [event_getcode4name],
                     [AC_MSG_WARN([===================================================================])
                      AC_MSG_WARN([Possible conflicting libevent.so libraries detected on the system.])
                      AC_MSG_WARN([])
                      AC_MSG_WARN([LSF provides a libevent.so that is not from Libevent in its])
                      AC_MSG_WARN([library path. It is possible that you have installed Libevent])
                      AC_MSG_WARN([on the system, but the linker is picking up the wrong version.])
                      AC_MSG_WARN([])
                      AC_MSG_WARN([You will need to address this linker path issue. One way to do so is])
                      AC_MSG_WARN([to make sure the libevent system library path occurs before the])
                      AC_MSG_WARN([LSF library path.])
                      AC_MSG_WARN([===================================================================])
                      AC_MSG_ERROR([Cannot continue])
                      ])
    fi

    if test $dsched_libevent_support -eq 0 ; then
        AC_MSG_WARN([===================================================================])
        AC_MSG_WARN([Libevent could not be found. DynaSched requires libevent support.])
        AC_MSG_WARN([Please locate or install libevent. If installing from packages,])
        AC_MSG_WARN([note that you may need to install both the libevent AND libevent-devel])
        AC_MSG_WARN([packages so we can find the required headers])
        AC_MSG_ERROR([Cannot continue])
    fi

    # need to add resulting flags to global ones so we can
    # test for thread support
    DSCHED_FLAGS_PREPEND_UNIQ([CPPFLAGS], [$dsched_libevent_CPPFLAGS])
    DSCHED_FLAGS_PREPEND_UNIQ([LDFLAGS], [$dsched_libevent_LDFLAGS])
    DSCHED_FLAGS_PREPEND_UNIQ([LIBS], [$dsched_libevent_LIBS])

    # Check for general threading support
    AC_MSG_CHECKING([if libevent threads enabled])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#include <event.h>
#include <event2/thread.h>
      ], [[
#if !(EVTHREAD_LOCK_API_VERSION >= 1)
#  error "No threads!"
#endif
      ]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])
       AC_MSG_WARN([DynaSched rquires libevent to be compiled with thread support enabled])
       AC_MSG_ERROR([Cannot continue])])

    AC_MSG_CHECKING([for libevent pthreads support])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#include <event.h>
#include <event2/thread.h>
      ], [[
#if !defined(EVTHREAD_USE_PTHREADS_IMPLEMENTED) || !EVTHREAD_USE_PTHREADS_IMPLEMENTED
#  error "No pthreads!"
#endif
      ]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])
       AC_MSG_WARN([DynaSched requires libevent to be compiled with pthread support enabled])
       AC_MSG_ERROR([Cannot continue])])

    dsched_event_min_num_version=DSCHED_EVENT_NUMERIC_MIN_VERSION
    dsched_event_min_version=DSCHED_EVENT_MIN_VERSION
    AC_MSG_CHECKING([version at or above v$dsched_event_min_version])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
                                        #include <event2/event.h>
#if defined(_EVENT_NUMERIC_VERSION) && _EVENT_NUMERIC_VERSION < $dsched_event_min_num_version
#error "libevent API version is less than $dsched_event_min_version"
#elif defined(EVENT__NUMERIC_VERSION) && EVENT__NUMERIC_VERSION < $dsched_event_min_num_version
#error "libevent API version is less than $dsched_event_min_version"
#endif
                                   ], [])],
                  [dsched_libevent_cv_version_check=yes
                   AC_MSG_RESULT([yes])],
                  [dsched_libevent_cv_version_check=no
               AC_MSG_RESULT([no])])
    AS_IF([test "${dsched_libevent_cv_version_check}" = "no"],
          [AC_MSG_WARN([libevent version is too old ($dsched_event_min_version or later required)])
           AC_MSG_ERROR([Cannot continue])])

    # restore global flags
    CPPFLAGS="$dsched_check_libevent_save_CPPFLAGS"
    LDFLAGS="$dsched_check_libevent_save_LDFLAGS"
    LIBS="$dsched_check_libevent_save_LIBS"

    AC_MSG_CHECKING([will libevent support be built])
    AC_MSG_RESULT([yes])
    DSCHED_FLAGS_APPEND_UNIQ([DSCHED_FINAL_CPPFLAGS], [$dsched_libevent_CPPFLAGS])

    DSCHED_FLAGS_APPEND_UNIQ([DSCHED_FINAL_LDFLAGS], [$dsched_libevent_LDFLAGS])

    DSCHED_FLAGS_APPEND_UNIQ([DSCHED_FINAL_LIBS], [$dsched_libevent_LIBS])

    # Set output variables
    DSCHED_SUMMARY_ADD([Required Packages], [Libevent], [], [$dsched_libevent_SUMMARY])

    DSCHED_VAR_SCOPE_POP
])
