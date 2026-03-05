dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
dnl                         University Research and Technology
dnl                         Corporation.  All rights reserved.
dnl Copyright (c) 2004-2005 The University of Tennessee and The University
dnl                         of Tennessee Research Foundation.  All rights
dnl                         reserved.
dnl Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
dnl                         University of Stuttgart.  All rights reserved.
dnl Copyright (c) 2004-2005 The Regents of the University of California.
dnl                         All rights reserved.
dnl Copyright (c) 2006-2020 Cisco Systems, Inc.  All rights reserved
dnl Copyright (c) 2007      Sun Microsystems, Inc.  All rights reserved.
dnl Copyright (c) 2009      IBM Corporation.  All rights reserved.
dnl Copyright (c) 2009-2013 Los Alamos National Security, LLC.  All rights
dnl                         reserved.
dnl Copyright (c) 2009      Oak Ridge National Labs.  All rights reserved.
dnl
dnl Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
dnl Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
dnl Copyright (c) 2022      Amazon.com, Inc. or its affiliates.
dnl                         All Rights reserved.
dnl $COPYRIGHT$
dnl
dnl Additional copyrights may follow
dnl
dnl $HEADER$
dnl


AC_DEFUN([DSCHED_CONFIGURE_OPTIONS],[
dsched_show_subtitle "DYNASCHED Configuration options"

# A hint to tell us if we are working with a build from Git or a tarball.
# Helpful when preparing diagnostic output.
if test -e $DSCHED_TOP_SRCDIR/.git; then
    AC_DEFINE_UNQUOTED([DSCHED_GIT_REPO_BUILD], ["1"],
                       [If built from a git repo])
    dsched_git_repo_build=yes
else
    dsched_git_repo_build=no
fi


#
# Do we want dsched's --prefix behavior to be enabled by default?
#
AC_MSG_CHECKING([if want dsched "--prefix" behavior to be enabled by default])
AC_ARG_ENABLE([dsched-prefix-by-default],
    [AS_HELP_STRING([--enable-dsched-prefix-by-default],
        [Make "dsched ..." behave exactly the same as "dsched --prefix \$prefix" (where \$prefix is the value given to --prefix in configure)])])
if test "$enable_dsched_prefix_by_default" = "yes"; then
    AC_MSG_RESULT([yes])
    dsched_want_dsched_prefix_by_default=1
else
    AC_MSG_RESULT([no])
    dsched_want_dsched_prefix_by_default=0
fi
AC_DEFINE_UNQUOTED([DSCHED_WANT_DSCHED_PREFIX_BY_DEFAULT],
                   [$dsched_want_dsched_prefix_by_default],
                   [Whether we want dsched to effect "--prefix $prefix" by default])

#
# Developer debugging
#
AC_MSG_CHECKING([if want developer-level debugging code])
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],
                   [enable developer-level debugging code (not for general DSCHED users!) (default: disabled)]))
if test "$enable_debug" = "yes"; then
    AC_MSG_RESULT([yes])
    WANT_DEBUG=1
else
    AC_MSG_RESULT([no])
    WANT_DEBUG=0
fi


if test "$WANT_DEBUG" = "0"; then
    CFLAGS="-DNDEBUG $CFLAGS"
    CXXFLAGS="-DNDEBUG $CXXFLAGS"
fi
AC_DEFINE_UNQUOTED(DSCHED_ENABLE_DEBUG, $WANT_DEBUG,
    [Whether we want developer-level debugging code or not])

AC_ARG_ENABLE(debug-symbols,
    AS_HELP_STRING([--disable-debug-symbols],
        [Disable adding compiler flags to enable debugging symbols if --enable-debug is specified.  For non-debugging builds, this flag has no effect.]))

#
# Developer picky compiler options
#

AC_MSG_CHECKING([if want developer-level compiler pickyness])
AC_ARG_ENABLE(devel-check,
    AS_HELP_STRING([--enable-devel-check],
                   [enable developer-level compiler pickyness when building DSCHED (default: disabled)]))
if test "$enable_devel_check" = "yes"; then
    AC_MSG_RESULT([yes])
    WANT_PICKY_COMPILER=1
elif test "$enable_devel_check" = "no"; then
    AC_MSG_RESULT([no])
    WANT_PICKY_COMPILER=0
elif test "$dsched_git_repo_build" = "yes" && test "$WANT_DEBUG" = "1"; then
    AC_MSG_RESULT([yes])
    WANT_PICKY_COMPILER=1
else
    AC_MSG_RESULT([no])
    WANT_PICKY_COMPILER=0
fi

AC_DEFINE_UNQUOTED(DSCHED_PICKY_COMPILERS, $WANT_PICKY_COMPILER,
                   [Whether or not we are using picky compiler settings])
AM_CONDITIONAL([DSCHED_PICKY_COMPILERS], [test "$WANT_PICKY_COMPILER" = "1"])

AC_MSG_CHECKING([if want memory sanitizers])
AC_ARG_ENABLE(memory-sanitizers,
    AS_HELP_STRING([--memory-sanitizers],
                   [enable developer-level memory sanitizers when building DSCHED (default: disabled)]))
if test "$enable_memory_sanitizers" = "yes"; then
    AC_MSG_RESULT([yes])
    WANT_MEMORY_SANITIZERS=1
    AC_MSG_WARN([******************************************************])
    AC_MSG_WARN([**** Memory sanitizers may require that you LD_PRELOAD])
    AC_MSG_WARN([**** libasan in order to run an executable.])
    AC_MSG_WARN([******************************************************])
else
    AC_MSG_RESULT([no])
    WANT_MEMORY_SANITIZERS=0
fi

AC_DEFINE_UNQUOTED(DSCHED_MEMORY_SANITIZERS, $WANT_MEMORY_SANITIZERS,
                   [Whether or not we are using memory sanitizers])

#
# Do we want the pretty-print stack trace feature?
#

AC_MSG_CHECKING([if want pretty-print stacktrace])
AC_ARG_ENABLE([pretty-print-stacktrace],
    [AS_HELP_STRING([--enable-pretty-print-stacktrace],
                    [Pretty print stacktrace on process signal (default: enabled)])])
if test "$enable_pretty_print_stacktrace" = "no" ; then
    AC_MSG_RESULT([no])
    WANT_PRETTY_PRINT_STACKTRACE=0
else
    AC_MSG_RESULT([yes])
    WANT_PRETTY_PRINT_STACKTRACE=1
fi
AC_DEFINE_UNQUOTED([DSCHED_WANT_PRETTY_PRINT_STACKTRACE],
                   [$WANT_PRETTY_PRINT_STACKTRACE],
                   [if want pretty-print stack trace feature])

#
# Do we want to allow DLOPEN?
#

AC_MSG_CHECKING([if want dlopen support])
AC_ARG_ENABLE([dlopen],
    [AS_HELP_STRING([--enable-dlopen],
                    [Whether build should attempt to use dlopen (or
                     similar) to dynamically load components.
                     Disabling dlopen implies --disable-mca-dso.
                     (default: enabled)])])
if test "$enable_dlopen" = "no" ; then
    DSCHED_ENABLE_DLOPEN_SUPPORT=0
    AC_MSG_RESULT([no])
else
    DSCHED_ENABLE_DLOPEN_SUPPORT=1
    AC_MSG_RESULT([yes])
fi
AC_DEFINE_UNQUOTED(DSCHED_ENABLE_DLOPEN_SUPPORT, $DSCHED_ENABLE_DLOPEN_SUPPORT,
    [Whether we want to enable dlopen support])


#
# Do we want to show component load error messages by default?
#

AC_MSG_CHECKING([for default value of mca_base_component_show_load_errors])
AC_ARG_ENABLE([show-load-errors-by-default],
    [AS_HELP_STRING([--enable-show-load-errors-by-default],
                    [Set the default value for the MCA parameter
                     mca_base_component_show_load_errors (but can be
                     overridden at run time by the usual
                     MCA-variable-setting mechansism).  This MCA variable
                     controls whether warnings are displayed when an MCA
                     component fails to load at run time due to an error.
                     (default: enabled in --enable-debug builds, meaning that
                      mca_base_component_show_load_errors is enabled
                      by default when configured with --enable-debug])])
if test "$enable_show_load_errors_by_default" = "no" ; then
    DSCHED_SHOW_LOAD_ERRORS_DEFAULT=0
    AC_MSG_RESULT([disabled by default])
else
    DSCHED_SHOW_LOAD_ERRORS_DEFAULT=$WANT_DEBUG
    if test "$WANT_DEBUG" = "1"; then
        AC_MSG_RESULT([enabled by default])
    else
        AC_MSG_RESULT([disabled by default])
    fi
fi
AC_DEFINE_UNQUOTED(DSCHED_SHOW_LOAD_ERRORS_DEFAULT, $DSCHED_SHOW_LOAD_ERRORS_DEFAULT,
                   [Default value for mca_base_component_show_load_errors MCA variable])

#
# Handle embedded version strings
#
AC_MSG_CHECKING([if a proxy version string for dsched is required])
AC_ARG_WITH(proxy-version-string,
    AS_HELP_STRING([--with-proxy-version-string],
                   [Return the provided string when dsched is used in proxy mode and the version is requested]))
if test -n "$with_proxy_version_string"; then
    AC_MSG_RESULT([yes])
    DSCHED_PROXY_VERSION_STRING=$with_proxy_version_string
else
    AC_MSG_RESULT([no])
    DSCHED_PROXY_VERSION_STRING=$DSCHED_VERSION
fi
AC_DEFINE_UNQUOTED(DSCHED_PROXY_VERSION_STRING, "$DSCHED_PROXY_VERSION_STRING",
                   [Version string to be returned by dsched when in proxy mode])

#
# Save the actual version in an external header file so that
# packages that use us can know what version we are
#
dschedmajor=${DSCHED_MAJOR_VERSION}L
dschedminor=${DSCHED_MINOR_VERSION}L
dschedrelease=${DSCHED_RELEASE_VERSION}L
dschednumeric=$(printf 0x%4.4x%2.2x%2.2x $DSCHED_MAJOR_VERSION $DSCHED_MINOR_VERSION $DSCHED_RELEASE_VERSION)
AC_SUBST(dschedmajor)
AC_SUBST(dschedminor)
AC_SUBST(dschedrelease)
AC_SUBST(dschednumeric)

# Package/brand string
#
AC_MSG_CHECKING([if want package/brand string])
AC_ARG_WITH([package-string],
     [AS_HELP_STRING([--with-package-string=STRING],
                     [Use a branding string throughout DynaSched])])
if test "$with_package_string" = "" || test "$with_package_string" = "no"; then
    with_package_string="DynaSched $DSCHED_CONFIGURE_USER@$DSCHED_CONFIGURE_HOST Distribution"
fi
AC_DEFINE_UNQUOTED([DSCHED_PACKAGE_STRING], ["$with_package_string"],
     [package/branding string for DynaSched])
AC_MSG_RESULT([$with_package_string])

#
# Ident string
#
AC_MSG_CHECKING([if want ident string])
AC_ARG_WITH([ident-string],
     [AS_HELP_STRING([--with-ident-string=STRING],
                     [Embed an ident string into DynaSched object files])])
if test "$with_ident_string" = "" || test "$with_ident_string" = "no"; then
    with_ident_string="%VERSION%"
fi
# This is complicated, because $DSCHED_VERSION may have spaces in it.
# So put the whole sed expr in single quotes -- i.e., directly
# substitute %VERSION% for (not expanded) $DSCHED_VERSION.
with_ident_string="`echo $with_ident_string | sed -e 's/%VERSION%/$DSCHED_VERSION/'`"

# Now eval an echo of that so that the "$DSHED_VERSION" token is
# replaced with its value.  Enclose the whole thing in "" so that it
# ends up as 1 token.
with_ident_string="`eval echo $with_ident_string`"

AC_DEFINE_UNQUOTED([DSCHED_IDENT_STRING], ["$with_ident_string"],
     [ident string for DynaSched])
AC_MSG_RESULT([$with_ident_string])


# some systems don't want/like getpwuid
AC_MSG_CHECKING([if want getpwuid support])
AC_ARG_ENABLE([getpwuid],
    [AS_HELP_STRING([--disable-getpwuid],
        [Disable getpwuid support (default: enabled)])])
if test "$enable_getpwuid" = "no"; then
    AC_MSG_RESULT([no])
    dsched_want_getpwuid=0
else
    AC_MSG_RESULT([yes])
    dsched_want_getpwuid=1
fi
AC_DEFINE_UNQUOTED([DSCHED_ENABLE_GETPWUID], [$dsched_want_getpwuid],
                   [Disable getpwuid support (default: enabled)])

])dnl
