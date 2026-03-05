# -*- shell-script ; indent-tabs-mode:nil -*-
#
# Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2005 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved
# Copyright (c) 2011-2014 Los Alamos National Security, LLC. All rights
#                         reserved.
# Copyright (c) 2014-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2014-2019 Research Organization for Information Science
#                         and Technology (RIST).  All rights reserved.
# Copyright (c) 2016      IBM Corporation.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# Copyright (c) 2021-2022 Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# Copyright (c) 2023      Jeffrey M. Squyres.  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

dnl $1 is the base cap name (i.e., what comes after "PMIX_CAP_")
dnl $2 is the action if happy
dnl $3 is the action if not happy
AC_DEFUN([DSCHED_CHECK_PMIX_CAP],[

    AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([#include <pmix_version.h>],
                         [#if !defined(PMIX_CAPABILITIES)
                          #error This PMIx does not have any capability flags
                          #endif
                          #if !defined(PMIX_CAP_$1)
                          #error This PMIx does not have the PMIX_CAP_$1 capability flag
                          #endif
                         ]
                        )
        ],
        [$2],
        [$3])

])

AC_DEFUN([DSCHED_CHECK_PMIX],[

    DSCHED_VAR_SCOPE_PUSH([dsched_external_pmix_save_CPPFLAGS dsched_pmix_support found_pmixcc])

    AC_ARG_WITH([pmix],
                [AS_HELP_STRING([--with-pmix(=DIR)],
                                [Where to find PMIx support, optionally adding DIR to the search path])])
    AC_ARG_WITH([pmix-libdir],
                [AS_HELP_STRING([--with-pmix-libdir=DIR],
                                [Look for libpmix in the given directory DIR, DIR/lib or DIR/lib64])])

    dsched_pmix_support=1

    if test "$with_pmix" = "no"; then
        AC_MSG_WARN([DynaSched requires PMIx support using])
        AC_MSG_WARN([an external copy that you supply.])
        AC_MSG_ERROR([Cannot continue])
    fi

    dnl Need to explicitly enable wrapper compiler to get the dependent libraries
    dnl when pkg-config is not available.
    PMIX_USE_WRAPPER_COMPILER=1
    OAC_CHECK_PACKAGE([pmix],
                      [dsched_pmix],
                      [pmix.h],
                      [pmix],
                      [PMIx_Init],
                      [],
                      [dsched_pmix_support=0])

    AS_IF([test $dsched_pmix_support -eq 0],
          [AC_MSG_WARN([DynaSched requires PMIx support using an external copy that you supply.])
           AC_MSG_ERROR([Cannot continue.])])

    dsched_external_pmix_save_CPPFLAGS=$CPPFLAGS
    DSCHED_FLAGS_PREPEND_UNIQ(CPPFLAGS, $dsched_pmix_CPPFLAGS)

    # if the version file exists, then we need to parse it to find
    # the actual release series
    # NOTE: We have already read DynaSched's VERSION file, so we can use
    # $pmix_min_version.
    dsched_pmix_min_num_version=DSCHED_PMIX_NUMERIC_MIN_VERSION
    dsched_pmix_min_version=DSCHED_PMIX_MIN_VERSION
    dsched_pmix_max_version=DSCHED_PMIX_MAX_VERSION
    AC_MSG_CHECKING([version at or above v$dsched_pmix_min_version])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
                                        #include <pmix_version.h>
                                        #if (PMIX_NUMERIC_VERSION < $dsched_pmix_min_num_version)
                                        #error "not version $dsched_pmix_min_num_version or above"
                                        #endif
                                       ], [])],
                      [AC_MSG_RESULT([yes])],
                      [AC_MSG_RESULT(no)
                       AC_MSG_WARN([DynaSched requires PMIx v$dsched_pmix_min_num_version or above.])
                       AC_MSG_ERROR([Please select a supported version and configure again])])

    # NOTE: We have already read DynaSched's VERSION file, so we can use
    # $pmix_max_version.
    dsched_pmix_max_num_version=DSCHED_PMIX_NUMERIC_MAX_VERSION
    dsched_pmix_max_version=DSCHED_PMIX_MAX_VERSION
    AC_MSG_CHECKING([version below v$dsched_pmix_max_version])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
                                        #include <pmix_version.h>
                                        #if !(PMIX_NUMERIC_VERSION < $dsched_pmix_max_num_version)
                                        #error "not below version $dsched_pmix_max_num_version"
                                        #endif
                                       ], [])],
                      [AC_MSG_RESULT([yes])],
                      [AC_MSG_RESULT(no)
                       AC_MSG_WARN([DynaSched requires PMIx be below v$dsched_pmix_max_num_version.])
                       AC_MSG_ERROR([Please select a supported version and configure again])])

    AC_CHECK_HEADER([src/util/pmix_argv.h], [],
                    [AC_MSG_ERROR([Could not find PMIx devel headers.  Can not continue.])])

    # restore the global flags
    CPPFLAGS=$dsched_external_pmix_save_CPPFLAGS

    DSCHED_FLAGS_APPEND_UNIQ(DSCHED_FINAL_CPPFLAGS, $dsched_pmix_CPPFLAGS)
    DSCHED_FLAGS_APPEND_UNIQ(DSCHED_FINAL_LDFLAGS, $dsched_pmix_LDFLAGS)
    DSCHED_FLAGS_APPEND_UNIQ(DSCHED_FINAL_LIBS, $dsched_pmix_LIBS)

    AC_DEFINE_UNQUOTED([DSCHED_PMIX_MINIMUM_VERSION],
                       [$dsched_pmix_min_num_version],
                       [Minimum supported PMIx version])

    AC_DEFINE_UNQUOTED([DSCHED_PMIX_MIN_VERSION_STRING],
                       ["$dsched_pmix_min_version"],
                       [Minimum supported PMIx version])

    AC_DEFINE_UNQUOTED([DSCHED_PMIX_MAX_VERSION_STRING],
                       ["$dsched_pmix_max_version"],
                       [Maximum supported PMIx version])

    found_pmixcc=0
    PMIXCC_PATH="pmixcc"
    AS_IF([test -n "${with_pmix}"],
          [PMIXCC_PATH="${with_pmix}/bin/$PMIXCC_PATH"])
    DSCHED_LOG_COMMAND([pmixcc_showme_results=`$PMIXCC_PATH --showme:version 2>&1`], [found_pmixcc=1])
    DSCHED_LOG_MSG([pmixcc version: $pmixcc_showme_results])
    AS_IF([test $found_pmixcc -eq 0],
          [AC_MSG_WARN([Could not find $PMIXCC_PATH])
           PMIXCC_PATH=])
    AM_CONDITIONAL([DSCHED_HAVE_PMIXCC], [test $found_pmixcc -eq 1])
    AC_SUBST([PMIXCC_PATH])

    # Check for any needed capabilities from the PMIx we found.
    #
    # Note: if the PMIx we found does not define capability flags,
    # then it definitely does not have the capability flags we're
    # looking for.

    dsched_external_pmix_save_CPPFLAGS=$CPPFLAGS
    dsched_external_pmix_save_LDFLAGS=$LDFLAGS
    dsched_external_pmix_save_LIBS=$LIBS

    DSCHED_FLAGS_APPEND_UNIQ(CPPFLAGS, $DSCHED_FINAL_CPPFLAGS)
    DSCHED_FLAGS_APPEND_UNIQ(LDFLAGS, $DSCHED_FINAL_LDFLAGS)
    DSCHED_FLAGS_APPEND_UNIQ(LIBS, $DSCHED_FINAL_LIBS)

    AC_MSG_CHECKING([for functional form of GET_NUMBER macro])
    DSCHED_CHECK_PMIX_CAP([GET_NUMBER_FN],
                        [AC_MSG_RESULT([yes])
                         dsched_get_number_macro=1],
                        [AC_MSG_RESULT([no])
                         dsched_get_number_macro=0])
    AC_DEFINE_UNQUOTED([DSCHED_PMIX_GET_NUMBER_FN],
                       [$dsched_get_number_macro],
                       [Whether or not PMIx has the GET_NUMBER FN])

    AC_MSG_CHECKING([for support of stop progress thread API])
    DSCHED_CHECK_PMIX_CAP([STOP_PRGTHRD],
                        [AC_MSG_RESULT([yes])
                         dsched_pmix_stop_progress_thread=1],
                        [AC_MSG_RESULT([no])
                         dsched_pmix_stop_progress_thread=0])
    AC_DEFINE_UNQUOTED([DSCHED_PMIX_STOP_PRGTHRD],
                       [$dsched_pmix_stop_progress_thread],
                       [Whether or not PMIx supports the stop progress thread API])

    AC_MSG_CHECKING([for support of version 2 server upcalls])
    DSCHED_CHECK_PMIX_CAP([UPCALLS2],
                        [AC_MSG_RESULT([yes])
                         dsched_server2_upcalls=1],
                        [AC_MSG_RESULT([no])
                         dsched_server2_upcalls=0])
    AC_DEFINE_UNQUOTED([DSCHED_PMIX_SERVER2_UPCALLS],
                       [$dsched_server2_upcalls],
                       [Whether or not PMIx supports server2 upcalls])

    AC_MSG_CHECKING([for in-memory show-help content compatibility])
    DSCHED_CHECK_PMIX_CAP([INMEMHELP],
                        [AC_MSG_RESULT([yes])],
                        [AC_MSG_RESULT([no])
                         AC_MSG_WARN([Your PMIx version either does not have])
                         AC_MSG_WARN([the capabilities feature or does not])
                         AC_MSG_WARN([include the PMIX_CAP_INMEMHELP capability flag.])
                         AC_MSG_WARN([This DynaSched version requires that support])
                         AC_MSG_WARN([in order to build. Please point us to a])
                         AC_MSG_WARN([PMIx installation with the required capability.])
                         AC_MSG_ERROR([Cannot continue.])])

    AC_MSG_CHECKING([for LTO compatibility])
    DSCHED_CHECK_PMIX_CAP([LTO],
                        [DSCHED_PMIX_LTO_CAPABILITY=1
                         AC_MSG_RESULT([yes])],
                        [AC_MSG_RESULT([no])
                         AC_MSG_WARN([Your PMIx version either does not have])
                         AC_MSG_WARN([the capabilities feature or does not])
                         AC_MSG_WARN([include the PMIX_CAP_LTO capability flag])
                         AC_MSG_WARN([This build will not be compatible with the])
                         AC_MSG_WARN([LTO optimizer. All LTO-related flags will])
                         AC_MSG_WARN([be removed from the build])
                         DSCHED_PMIX_LTO_CAPABILITY=0])

    AC_DEFINE_UNQUOTED([DSCHED_PMIX_LTO_CAPABILITY],
                       [$DSCHED_PMIX_LTO_CAPABILITY],
                       [Whether or not PMIx has the LTO capability flag set])

    # restore the global flags
    CPPFLAGS=$dsched_external_pmix_save_CPPFLAGS
    LDFLAGS=$dsched_external_pmix_save_LDFLAGS
    LIBS=$dsched_external_pmix_save_LIBS

    DSCHED_SUMMARY_ADD([Required Packages], [PMIx], [], [$dsched_pmix_SUMMARY])

    DSCHED_VAR_SCOPE_POP
])
