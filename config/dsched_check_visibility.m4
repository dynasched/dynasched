# -*- shell-script -*-
#
# Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2005 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2007 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2006-2020 Cisco Systems, Inc.  All rights reserved
# Copyright (c) 2009-2011 Oracle and/or its affiliates.  All rights reserved.
# Copyright (c) 2019      Intel, Inc.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# DSCHED_CHECK_VISIBILITY
# --------------------------------------------------------
AC_DEFUN([DSCHED_CHECK_VISIBILITY],[
    AC_REQUIRE([AC_PROG_GREP])

    # Check if the compiler has support for visibility, like some
    # versions of gcc, icc Sun Studio cc.
    AC_ARG_ENABLE(visibility,
        AS_HELP_STRING([--enable-visibility],
            [enable visibility feature of certain compilers/linkers (default: enabled)]))

    dsched_visibility_define=0
    dsched_msg="whether to enable symbol visibility"

    if test "$enable_visibility" = "no"; then
        AC_MSG_CHECKING([$dsched_msg])
        AC_MSG_RESULT([no (disabled)])
    else
        CFLAGS_orig=$CFLAGS

        dsched_add=
        case "$dsched_c_vendor" in
        sun)
            # Check using Sun Studio -xldscope=hidden flag
            dsched_add=-xldscope=hidden
            CFLAGS="$DSCHED_CFLAGS_BEFORE_PICKY $dsched_add -errwarn=%all"
            ;;

        *)
            # Check using -fvisibility=hidden
            dsched_add=-fvisibility=hidden
            CFLAGS="$DSCHED_CFLAGS_BEFORE_PICKY $dsched_add -Werror"
            ;;
        esac

        AC_MSG_CHECKING([if $CC supports $dsched_add])
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[
            #include <stdio.h>
            __attribute__((visibility("default"))) int foo;
            ]],[[fprintf(stderr, "Hello, world\n");]])],
            [AS_IF([test -s conftest.err],
                   [$GREP -iq visibility conftest.err
                    # If we find "visibility" in the stderr, then
                    # assume it doesn't work
                    AS_IF([test "$?" = "0"], [dsched_add=])])
            ], [dsched_add=])
        AS_IF([test "$dsched_add" = ""],
              [AC_MSG_RESULT([no])],
              [AC_MSG_RESULT([yes])])

        CFLAGS=$CFLAGS_orig
        DSCHED_VISIBILITY_CFLAGS=$dsched_add

        if test "$dsched_add" != "" ; then
            dsched_visibility_define=1
            AC_MSG_CHECKING([$dsched_msg])
            AC_MSG_RESULT([yes (via $dsched_add)])
        elif test "$enable_visibility" = "yes"; then
            AC_MSG_ERROR([Symbol visibility support requested but compiler does not seem to support it.  Aborting])
        else
            AC_MSG_CHECKING([$dsched_msg])
            AC_MSG_RESULT([no (unsupported)])
        fi
        unset dsched_add
    fi

    AC_DEFINE_UNQUOTED([DSCHED_C_HAVE_VISIBILITY], [$dsched_visibility_define],
            [Whether C compiler supports symbol visibility or not])
])
