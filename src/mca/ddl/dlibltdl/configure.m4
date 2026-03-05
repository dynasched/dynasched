# -*- shell-script -*-
#
# Copyright (c) 2009-2015 Cisco Systems, Inc.  All rights reserved.
#
# Copyright (c) 2017      Intel, Inc.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# Copyright (c) 2022      Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

AC_DEFUN([MCA_dsched_ddl_dlibltdl_PRIORITY], [50])

#
# Force this component to compile in static-only mode
#
AC_DEFUN([MCA_dsched_ddl_dlibltdl_COMPILE_MODE], [
    AC_MSG_CHECKING([for MCA component $1:$2 compile mode])
    $3="static"
    AC_MSG_RESULT([$$3])
])

# MCA_dsched_ddl_dlibltdl_POST_CONFIG()
# ---------------------------------
AC_DEFUN([MCA_dsched_ddl_dlibltdl_POST_CONFIG],[
    # If we won, then do all the rest of the setup
    AS_IF([test "$1" = "1"],
          [
           # Add some stuff to CPPFLAGS so that the rest of the source
           # tree can be built
           LDFLAGS="$LDFLAGS $dsched_ddl_dlibltdl_ADD_LDFLAGS"
           LIBS="$LIBS $dsched_ddl_dlibltdl_ADD_LIBS"
          ])
])dnl

# MCA_ddl_dlibltdl_CONFIG([action-if-can-compile],
#                         [action-if-cant-compile])
# ------------------------------------------------
AC_DEFUN([MCA_dsched_ddl_dlibltdl_CONFIG],[
    DSCHED_VAR_SCOPE_PUSH([CPPFLAGS_save LDFLAGS_save LIBS_save])
    AC_CONFIG_FILES([src/mca/ddl/dlibltdl/Makefile])

    # Add --with options
    AC_ARG_WITH([libltdl],
        [AS_HELP_STRING([--with-libltdl(=DIR)],
             [Build libltdl support, optionally adding DIR/include, DIR/lib, and DIR/lib64 to the search path for headers and libraries])])
    AC_ARG_WITH([libltdl-libdir],
       [AS_HELP_STRING([--with-libltdl-libdir=DIR],
             [Search for libltdl libraries in DIR])])

    dsched_ddl_dlibltdl_happy=no
    AS_IF([test "$with_libltdl" != "no"],
        [OAC_CHECK_PACKAGE([libltdl],
                      [dsched_ddl_dlibltdl],
                      [ltdl.h],
                      [ltdl],
                      [lt_dlopen],
                      [dsched_ddl_dlibltdl_happy=yes],
                      [dsched_ddl_dlibltdl_happy=no])
        ])

    # If we have dlibltdl, do we have lt_dladvise?
    dsched_ddl_dlibltdl_have_lt_dladvise=0
    AS_IF([test "$dsched_ddl_dlibltdl_happy" = "yes"],
          [CPPFLAGS_save=$CPPFLAGS
           LDFLAGS_save=$LDFLAGS
           LIBS_save=$LIBS

           CPPFLAGS="$dsched_ddl_dlibltdl_CPPFLAGS $CPPFLAGS"
           LDFLAGS="$dsched_ddl_dlibltdl_LDFLAGS $LDFLAGS"
           LIBS="$dsched_ddl_dlibltdl_LIBS $LIBS"
           AC_CHECK_FUNC([lt_dladvise_init],
                         [dsched_ddl_dlibltdl_have_lt_dladvise=1])
           CPPFLAGS=$CPPFLAGS_save
           LDFLAGS=$LDFLAGS_save
           LIBS=$LIBS_save
          ])
    AC_DEFINE_UNQUOTED(DSCHED_DDL_DLIBLTDL_HAVE_LT_DLADVISE,
        [$dsched_ddl_dlibltdl_have_lt_dladvise],
        [Whether we have lt_dladvise or not])

    AS_IF([test "$dsched_ddl_dlibltdl_happy" = "yes"],
          [dsched_ddl_dlibltdl_ADD_CPPFLAGS=$dsched_ddl_dlibltdl_CPPFLAGS
           dsched_ddl_dlibltdl_ADD_LDFLAGS=$dsched_ddl_dlibltdl_LDFLAGS
           dsched_ddl_dlibltdl_ADD_LIBS=$dsched_ddl_dlibltdl_LIBS
           $1],
          [AS_IF([test ! -z "$with_libltdl" && \
                  test "$with_libltdl" != "no"],
                 [AC_MSG_WARN([libltdl support requested (via --with-libltdl) but not found.])
                  AC_MSG_ERROR([Cannot continue.])])
           $2])

    AC_SUBST(dsched_ddl_dlibltdl_CPPFLAGS)
    AC_SUBST(dsched_ddl_dlibltdl_LDFLAGS)
    AC_SUBST(dsched_ddl_dlibltdl_LIBS)

    DSCHED_VAR_SCOPE_POP
])
