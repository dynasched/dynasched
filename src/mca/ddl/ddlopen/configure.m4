# -*- shell-script -*-
#
# Copyright (c) 2009-2015 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2016      Research Organization for Information Science
#                         and Technology (RIST). All rights reserved.
#
# Copyright (c) 2017      Intel, Inc.  All rights reserved.
# Copyright (c) 2022      Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# Copyright (c) 2023-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

AC_DEFUN([MCA_dsched_ddl_ddlopen_PRIORITY], [80])

#
# Force this component to compile in static-only mode
#
AC_DEFUN([MCA_dsched_ddl_ddlopen_COMPILE_MODE], [
    AC_MSG_CHECKING([for MCA component $1:$2 compile mode])
    $3="static"
    AC_MSG_RESULT([$$3])
])

# MCA_dsched_ddl_ddlopen_POST_CONFIG()
# ---------------------------------
AC_DEFUN([MCA_dsched_ddl_ddlopen_POST_CONFIG],[
    # If we won, then do all the rest of the setup
    AS_IF([test "$1" = "1"],
          [
           # Add some stuff to CPPFLAGS so that the rest of the source
           # tree can be built
           LDFLAGS="$LDFLAGS $dsched_ddl_ddlopen_ADD_LDFLAGS"
           LIBS="$LIBS $dsched_ddl_ddlopen_ADD_LIBS"
          ])
])dnl

# MCA_ddl_ddlopen_CONFIG([action-if-can-compile],
#                      [action-if-cant-compile])
# ------------------------------------------------
AC_DEFUN([MCA_dsched_ddl_ddlopen_CONFIG],[
    AC_CONFIG_FILES([src/mca/ddl/ddlopen/Makefile])

    dnl This is effectively a back-door for DynaSched developers to
    dnl force the use of the libltdl ddl component.
    AC_ARG_ENABLE([dsched-dlopen],
        [AS_HELP_STRING([--disable-dsched-dlopen],
                        [Disable the DynaSched "dlopen" DDL component (and probably force the use of the "libltdl" DDL component).])
        ])

    dsched_ddl_ddlopen_happy=no
    AS_IF([test "$enable_dsched_dlopen" != "no"],
        [OAC_CHECK_PACKAGE([dlopen],
                  [dsched_ddl_ddlopen],
                  [dlfcn.h],
                  [dl],
                  [dlopen],
                  [dsched_ddl_ddlopen_happy=yes],
                  [dsched_ddl_ddlopen_happy=no])
        ])

    AS_IF([test "$dsched_ddl_ddlopen_happy" = "yes"],
          [dsched_ddl_ddlopen_ADD_LIBS=$dsched_ddl_ddlopen_LIBS
           $1],
          [$2])

    AC_SUBST(dsched_ddl_ddlopen_LIBS)
])
