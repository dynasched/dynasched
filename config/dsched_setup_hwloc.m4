# -*- shell-script -*-
#
# Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved
# Copyright (c) 2013      Los Alamos National Security, LLC.  All rights reserved.
# Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# Copyright (c) 2021-2022 Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# MCA_hwloc_CONFIG([action-if-found], [action-if-not-found])
# --------------------------------------------------------------------
AC_DEFUN([DSCHED_SETUP_HWLOC],[
    DSCHED_VAR_SCOPE_PUSH([dsched_hwloc_dir dsched_hwloc_libdir dsched_check_hwloc_save_CPPFLAGS dsched_check_hwloc_save_LDFLAGS dsched_check_hwloc_save_LIBS])

    AC_ARG_WITH([hwloc],
                [AS_HELP_STRING([--with-hwloc=DIR],
                                [Search for hwloc headers and libraries in DIR ])])
    AC_ARG_WITH([hwloc-libdir],
                [AS_HELP_STRING([--with-hwloc-libdir=DIR],
                                [Search for hwloc libraries in DIR ])])

    dsched_hwloc_support=1
    dsched_check_hwloc_save_CPPFLAGS="$CPPFLAGS"
    dsched_check_hwloc_save_LDFLAGS="$LDFLAGS"
    dsched_check_hwloc_save_LIBS="$LIBS"

    if test "$with_hwloc" = "no"; then
        AC_MSG_WARN([DynaSched requires HWLOC topology library support.])
        AC_MSG_WARN([Please reconfigure so we can find the library.])
        AC_MSG_ERROR([Cannot continue.])
    fi

    OAC_CHECK_PACKAGE([hwloc],
                      [dsched_hwloc],
                      [hwloc.h],
                      [hwloc],
                      [hwloc_topology_init],
                      [],
                      [dsched_hwloc_support=0])

    if test $dsched_hwloc_support -eq 0; then
        AC_MSG_WARN([DynaSched requires HWLOC topology library support, but])
        AC_MSG_WARN([an adequate version of that library was not found.])
        AC_MSG_WARN([Please reconfigure and point to a location where])
        AC_MSG_WARN([the HWLOC library can be found.])
        AC_MSG_ERROR([Cannot continue.])
    fi

    # update global flags to test for HWLOC version
    DSCHED_FLAGS_PREPEND_UNIQ([CPPFLAGS], [$dsched_hwloc_CPPFLAGS])

    # NOTE: We have already read DynaSched's VERSION file, so we can use
    # those values
    dsched_hwloc_min_num_version=DSCHED_HWLOC_NUMERIC_MIN_VERSION
    dsched_hwloc_min_version=DSCHED_HWLOC_MIN_VERSION
    AC_MSG_CHECKING([version at or above v$dsched_hwloc_min_version])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
                                        #include <hwloc.h>
                                        #if (HWLOC_API_VERSION < $dsched_hwloc_min_num_version)
                                        #error "not version $dsched_hwloc_min_num_version or above"
                                        #endif
                                       ], [])],
                      [AC_MSG_RESULT([yes])],
                      [AC_MSG_RESULT(no)
                       AC_MSG_WARN([DynaSched requires HWLOC v$dsched_hwloc_min_version or above.])
                       AC_MSG_ERROR([Please select a supported version and configure again])])

    AC_MSG_CHECKING([if hwloc version is greater than 2.x])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
                                        #include <hwloc.h>
                                        #if (HWLOC_VERSION_MAJOR > 2)
                                        #error "hwloc version is greater than 2.x"
                                        #endif
                                       ], [])],
                      [AC_MSG_RESULT([no])],
                      [AC_MSG_RESULT([yes])
                       AC_MSG_WARN([This DynaSched version does not support HWLOC])
                       AC_MSG_WARN([versions 3.x or higher. Please direct us])
                       AC_MSG_WARN([to an HWLOC version in the $dsched_hwloc_min_version-2.x range.])
                       AC_MSG_ERROR([Cannot continue])])

    # reset global flags
    CPPFLAGS=$dsched_check_hwloc_save_CPPFLAGS

    DSCHED_FLAGS_APPEND_UNIQ([DSCHED_FINAL_CPPFLAGS], [$dsched_hwloc_CPPFLAGS])
    DSCHED_FLAGS_APPEND_UNIQ([DSCHED_FINAL_LDFLAGS], [$dsched_hwloc_LDFLAGS])
    DSCHED_FLAGS_APPEND_UNIQ([DSCHED_FINAL_LIBS], [$dsched_hwloc_LIBS])

    DSCHED_SUMMARY_ADD([Required Packages], [HWLOC], [], [$dsched_hwloc_SUMMARY])

    DSCHED_VAR_SCOPE_POP
])
