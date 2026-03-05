# -*- shell-script -*-
#
# Copyright (c) 2004-2009 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2015 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2007 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2006-2023 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2006-2008 Sun Microsystems, Inc.  All rights reserved.
# Copyright (c) 2006-2017 Los Alamos National Security, LLC.  All rights
#                         reserved.
# Copyright (c) 2009      Oak Ridge National Labs.  All rights reserved.
# Copyright (c) 2011-2013 NVIDIA Corporation.  All rights reserved.
# Copyright (c) 2012      Oracle and/or its affiliates.  All rights reserved.
# Copyright (c) 2013      Mellanox Technologies, Inc.
#                         All rights reserved.
# Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2014-2022 Research Organization for Information Science
#                         and Technology (RIST).  All rights reserved.
# Copyright (c) 2016-2017 IBM Corporation.  All rights reserved.
# Copyright (c) 2018-2022 Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# Copyright (c) 2021      FUJITSU LIMITED.  All rights reserved.
# Copyright (c) 2023-2024 Jeffrey M. Squyres.  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

############################################################################
# Initialization, version number, and other random setup/init stuff
############################################################################

# Load in everything found by autogen.pl
m4_include([config/autogen_found_items.m4])

# Init autoconf
# We don't have the version number to put in here yet, and we can't
# call DSCHED_GET_VERSION (etc.) before AC_INIT.  So use the shell
# version.  project_name_* comes from config/project_list.m4, which
# was set during autogen.pl.

AC_INIT([dsched],
        [m4_normalize(esyscmd([config/dsched_get_version.sh VERSION --tarball]))],
        [https://github.com/dynasched/dynasched/], [dsched])

AC_PREREQ(2.69)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)

OAC_PUSH_PREFIX([DSCHED])

# Get the absolute version of the srcdir.  We don't use "readlink -f",
# because that unfortunately isn't portable (cough cough macOS cough
# cough).
save=$(pwd)
cd $srcdir
abs_srcdir=$(pwd)
cd $save

# autotools expects to perform tests without interference
# from user-provided CFLAGS, particularly -Werror flags.
# Search for them here and cache any we find
DSCHED_CFLAGS_cache=
DSCHED_CFLAGS_pass=
for val in $CFLAGS; do
    if echo "$val" | grep -q -e "-W"; then
        DSCHED_CFLAGS_cache="$DSCHED_CFLAGS_cache $val";
    else
        DSCHED_CFLAGS_pass="$DSCHED_CFLAGS_pass $val";
    fi
done
CFLAGS=$DSCHED_CFLAGS_pass
# Load the version number code
m4_include([config/dsched_get_version.m4])

DSCHED_CAPTURE_CONFIGURE_CLI([DSCHED_CONFIGURE_CLI])

# Get our platform support file.  This has to be done very, very early
# because it twiddles random bits of autoconf
DSCHED_LOAD_PLATFORM

#
# Start it up
#
. $srcdir/VERSION
DSCHED_CONFIGURE_SETUP

dsched_show_title "Configuring DynaSched"

#
# Setup some things that must be done before AM-INIT-AUTOMAKE
#

dsched_show_subtitle "Startup tests"

AC_PROG_SED

AC_CHECK_PROG([PERL],[perl],[perl],[no])
AS_IF([test "X$PERL" = "Xno"],
      [AC_MSG_ERROR(["DynaSched requires perl. Aborting"])])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_DEFINE_UNQUOTED(DSCHED_ARCH, "$target", [DSCHED architecture string])

AS_IF([test "$host" != "$target"],
      [AC_MSG_WARN([Cross-compile detected])
       AC_MSG_WARN([Cross-compiling is only partially supported])
       AC_MSG_WARN([Proceed at your own risk!])])

# AC_USE_SYSTEM_EXTENSIONS alters CFLAGS (e.g., adds -g -O2)

DSCHED_VAR_SCOPE_PUSH([CFLAGS_save])
CFLAGS_save=$CFLAGS
AC_USE_SYSTEM_EXTENSIONS

# AC_USE_SYSTEM_EXTENSIONS will modify CFLAGS if nothing was in there
# beforehand.  We don't want that.  So if there was nothing in
# CFLAGS, put nothing back in there.

AS_IF([test -z "$CFLAGS_save"], [CFLAGS=])

DSCHED_VAR_SCOPE_POP

#
# Init automake
#

AM_INIT_AUTOMAKE([foreign dist-bzip2 subdir-objects no-define 1.13.4 tar-pax])

# SILENT_RULES is new in AM 1.11, but we require 1.11 or higher via
# autogen.  Limited testing shows that calling SILENT_RULES directly
# works in more cases than adding "silent-rules" to INIT_AUTOMAKE
# (even though they're supposed to be identical).  Shrug.

AM_SILENT_RULES([yes])

# set the language
AC_LANG([C])
# find NM
AC_PROG_GREP
LT_PATH_NM

# Make configure depend on the VERSION file, since it's used in AC_INIT
AC_SUBST([CONFIGURE_DEPENDENCIES], ['$(top_srcdir)/VERSION'])

# Sanity checks
AC_DEFUN([DSCHED_CHECK_DIR_FOR_SPACES],[
    dir="$1"
    article="$2"
    label="$3"

    AC_MSG_CHECKING([directory of $label])
    AC_MSG_RESULT([$dir])
    AS_IF([test -n "`echo $dir | grep ' '`"],
          [AC_MSG_WARN([This version of DynaSched does not support $article $label])
           AC_MSG_WARN([with a path that contains spaces])
           AC_MSG_ERROR([Cannot continue.])])
])

DSCHED_VAR_SCOPE_PUSH(dsched_checkdir)
dsched_checkdir=`pwd`
DSCHED_CHECK_DIR_FOR_SPACES([$dsched_checkdir], [a], [build tree])
DSCHED_CHECK_DIR_FOR_SPACES([$prefix], [a], [prefix])
DSCHED_CHECK_DIR_FOR_SPACES([$srcdir], [a], [source tree])
DSCHED_VAR_SCOPE_POP

dsched_show_subtitle "Checking versions"

# Get the version of DSCHED that we are installing
DSCHED_SAVE_VERSION([DSCHED], [DynaSched],
                          [$srcdir/VERSION],
                          [src/include/dsched_internal_version.h])

# Get shared library version numbers
. $srcdir/VERSION

AC_SUBST(libdsched_so_version)
AC_SUBST(libdsched_common_alps_so_version)

#
# Get the versions of the autotools that were used to bootstrap us
# (helpful for debugging reports)
#

AC_MSG_CHECKING([for bootstrap Autoconf version])
acversion=`grep "Generated by GNU Autoconf" $0 | head -n 1 | awk '{ print $6 }'`
AC_MSG_RESULT([$acversion])

AC_MSG_CHECKING([for bootstrap Automake version])
AC_MSG_RESULT([$am__api_version])

AC_MSG_CHECKING([for boostrap Libtool version])
ltversion=`grep VERSION= $srcdir/config/ltmain.sh | head -n 1 | cut -d= -f2`
AC_MSG_RESULT([$ltversion])

# List header files to generate

AC_CONFIG_HEADERS([src/include/dsched_config.h])

dsched_show_subtitle "Initialization, setup"

DSCHED_TOP_BUILDDIR="`pwd`"
AC_SUBST(DSCHED_TOP_BUILDDIR)
cd "$srcdir"
DSCHED_TOP_SRCDIR="`pwd`"
AC_SUBST(DSCHED_TOP_SRCDIR)
cd "$DSCHED_TOP_BUILDDIR"
AC_MSG_NOTICE([builddir: $DSCHED_TOP_BUILDDIR])
AC_MSG_NOTICE([srcdir: $DSCHED_TOP_SRCDIR])

if test "$DSCHED_TOP_BUILDDIR" != "$DSCHED_TOP_SRCDIR"; then
    AC_MSG_NOTICE([Detected VPATH build])
fi

# Setup the top of the src/include/dsched_config.h file

AH_TOP([/* -*- c -*-
 *
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2014      Intel, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * Function: - OS, CPU and compiler dependent configuration
 */

#ifndef DSCHED_CONFIG_H
#define DSCHED_CONFIG_H

#include "dsched_config_top.h"
])

AH_BOTTOM([
#include "dsched_config_bottom.h"
#endif /* DSCHED_CONFIG_H */
])

# Other basic setup stuff (shared with components)

DSCHED_BASIC_SETUP

DSCHED_TOP_SRCDIR="$DSCHED_TOP_SRCDIR"
DSCHED_TOP_BUILDDIR="$DSCHED_TOP_BUILDDIR"
AC_SUBST(DSCHED_TOP_SRCDIR)
AC_SUBST(DSCHED_TOP_BUILDDIR)

############################################################################
# Configuration options
############################################################################

OAC_CHECK_OS_FLAVORS
DSCHED_CONFIGURE_OPTIONS

if test "$enable_binaries" = "no" && test "$enable_dist" = "yes"; then
    AC_MSG_WARN([--disable-binaries is incompatible with --enable dist])
    AC_MSG_ERROR([Cannot continue])
fi


############################################################################
# Libtool: part one
# (before C compiler setup)
############################################################################

#
# Part one of libtool magic.  Default to: enable shared, disable static.
#

AC_ENABLE_SHARED([])
AC_DISABLE_STATIC([])

##################################
# Check for known incompatibility
##################################

# Do *not* print a message that we're checking the OS because this
# test is *not* meant to be an all-inclusive "if it passes this test,
# then configure must succeed" test.  This test is *only* mean to
# screen out the versions of OS X where we know OMPI will cause kernel
# panics because of bad implementations of pty's.  See
# https://svn.open-mpi.org/trac/ompi/ticket/1637 for details.
#   OS X name         OS X Version    $host_os value
# OS X Tiger             10.4.x           darwin8.x
# OS X Leopard           10.5.x           darwin9.x
# OS X Snow Leopard      10.6.x           darwin10.x
# OS X Lion              10.7.x           darwin11.x
# We do not support OS X before version 10.5 (Leopard)
case $host_os in
# Corresponds to OS X 10.0 - 10.4 (additional [] quoting for m4)
darwin[[45678]]*)
    AC_MSG_WARN([DSCHED does not support OS X prior to version 10.5 (Leopard)])
    AC_MSG_ERROR([Cannot continue])
esac

############################################################################
# Check for compilers and preprocessors
############################################################################

dsched_show_title "Compiler and preprocessor tests"

##################################
# C compiler characteristics
##################################

DSCHED_SETUP_CC

# Does the compiler support "ident"-like constructs?

DSCHED_CHECK_IDENT([CC], [CFLAGS], [c], [C])

#
# Check for type sizes
#

AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(void *)
AS_IF([test "$ac_cv_sizeof_void_p" -eq 4],
      [AC_MSG_WARN([DSCHED does not support 32 bit builds.])
       AC_MSG_ERROR([Cannot continue])])
AC_CHECK_SIZEOF(size_t)

#
# Check for type alignments
#

AC_CHECK_ALIGNOF(bool, [AC_INCLUDES_DEFAULT
                        #include <stdbool.h>])
AC_CHECK_ALIGNOF(int32_t)
AC_CHECK_ALIGNOF(int64_t)
AC_CHECK_ALIGNOF(int)
AC_CHECK_ALIGNOF(long)
AC_CHECK_ALIGNOF(long long)
AC_CHECK_ALIGNOF(double)
AC_CHECK_ALIGNOF(size_t)

# Check if we support the offsetof compiler directive
DSCHED_CHECK_OFFSETOF

##################################
# Only after setting up C check compiler attributes.
##################################

dsched_show_subtitle "Compiler characteristics"

DSCHED_CHECK_ATTRIBUTES
DSCHED_CHECK_COMPILER_VERSION_ID

# DSCHED only supports GCC >=v4.8.1.  Notes:
#
# 1. The default compiler that comes with RHEL 7 is v4.8.5
#    (version ID 264197).
# 2. We regularly test with GCC v4.8.1 (version ID 264193).
# 3. GCC 4.8.0 probably also works; we just haven't tested it.
#
# Since we regularly test with 4.8.1, that's what we check for.
AS_IF([test "$dsched_cv_compiler_FAMILYNAME" = "GNU" && \
           test "$dsched_cv_compiler_VERSION" -lt 264193],
      [AC_MSG_WARN([DSCHED no longer supports versions of the GNU compiler suite])
       AC_MSG_WARN([less than v4.8.1.])
       AC_MSG_WARN([Please upgrade your GNU compiler suite, or use])
       AC_MSG_WARN([a different compiler to build DSCHED.])
       AC_MSG_ERROR([Cannot continue])
      ])

##################################
# Assembler Configuration
##################################

dsched_show_subtitle "Atomics"

DSCHED_CONFIG_ASM

##################################
# Header files
##################################

dsched_show_title "Header file tests"

DSCHED_VAR_SCOPE_PUSH(DSCHED_CFLAGS_save_for_headers)
DSCHED_CFLAGS_save_for_headers=$CFLAGS
_DSCHED_CHECK_SPECIFIC_CFLAGS(-Werror, Werror)

AC_CHECK_HEADERS([arpa/inet.h dirent.h \
                  dlfcn.h endian.h execinfo.h fcntl.h \
                  libutil.h netdb.h netinet/in.h netinet/tcp.h \
                  poll.h  pty.h pwd.h sched.h \
                  strings.h linux/ethtool.h linux/sockios.h \
                  sys/fcntl.h \
                  sys/ioctl.h sys/param.h sys/queue.h \
                  sys/resource.h sys/select.h sys/socket.h \
                  sys/stat.h sys/time.h \
                  sys/types.h sys/uio.h sys/un.h net/uio.h sys/utsname.h sys/wait.h syslog.h \
                  termios.h unistd.h util.h malloc.h \
                  paths.h \
                  ioLib.h sockLib.h hostLib.h stdatomic.h])

# Needed to work around Darwin requiring sys/socket.h for
# net/if.h

AC_CHECK_HEADERS([net/if.h], [], [],
                 [#include <stdio.h>
                  #if STDC_HEADERS
                  # include <stdlib.h>
                  # include <stddef.h>
                  #else
                  # if HAVE_STDLIB_H
                  #  include <stdlib.h>
                  # endif
                  #endif
                  #if HAVE_SYS_SOCKET_H
                  # include <sys/socket.h>
                  #endif
                 ])

CFLAGS=$DSCHED_CFLAGS_save_for_headers
DSCHED_VAR_SCOPE_POP

# checkpoint results
AC_CACHE_SAVE

##################################
# Types
##################################

dsched_show_title "Type tests"

AC_CHECK_TYPES([socklen_t, struct sockaddr_in, struct sockaddr_in6,
                struct sockaddr_storage, struct ifreq, struct ethtool_cmd],
               [], [], [AC_INCLUDES_DEFAULT
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_LINUX_ETHTOOL_H
#include <linux/ethtool.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif])

AC_CHECK_DECLS([ethtool_cmd_speed, SIOCETHTOOL],
               [], [], [AC_INCLUDES_DEFAULT
#ifdef HAVE_LINUX_ETHTOOL_H
#include <linux/ethtool.h>
#endif
#ifdef HAVE_LINUX_SOCKIOS_H
#include <linux/sockios.h>
#endif])

AC_CHECK_MEMBERS([struct ethtool_cmd.speed_hi],
               [], [], [AC_INCLUDES_DEFAULT
#ifdef HAVE_LINUX_ETHTOOL_H
#include <linux/ethtool.h>
#endif
#ifdef HAVE_LINUX_SOCKIOS_H
#include <linux/sockios.h>
#endif])

AC_CHECK_MEMBERS([struct ethtool_cmd.speed_hi],
               [], [], [AC_INCLUDES_DEFAULT
#ifdef HAVE_LINUX_ETHTOOL_H
#include <linux/ethtool.h>
#endif
#ifdef HAVE_LINUX_SOCKIOS_H
#include <linux/sockios.h>
#endif])

AC_CHECK_MEMBERS([struct ethtool_cmd.speed_hi],
               [], [], [AC_INCLUDES_DEFAULT
#ifdef HAVE_LINUX_ETHTOOL_H
#include <linux/ethtool.h>
#endif
#ifdef HAVE_LINUX_SOCKIOS_H
#include <linux/sockios.h>
#endif])

AC_CHECK_MEMBERS([struct ethtool_cmd.speed_hi],
               [], [], [AC_INCLUDES_DEFAULT
#ifdef HAVE_LINUX_ETHTOOL_H
#include <linux/ethtool.h>
#endif
#ifdef HAVE_LINUX_SOCKIOS_H
#include <linux/sockios.h>
#endif])

AC_CHECK_DECLS([AF_UNSPEC, PF_UNSPEC, AF_INET6, PF_INET6],
               [], [], [AC_INCLUDES_DEFAULT
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif])

AC_CHECK_MEMBERS([siginfo_t.si_fd],,,[#include <signal.h>])

AC_CHECK_MEMBERS([siginfo_t.si_band],,,[#include <signal.h>])

# checkpoint results
AC_CACHE_SAVE

##################################
# Linker characteristics
##################################

AC_MSG_CHECKING([the linker for support for the -fini option])
DSCHED_VAR_SCOPE_PUSH([LDFLAGS_save])
LDFLAGS_save=$LDFLAGS
LDFLAGS="$LDFLAGS_save -Wl,-fini -Wl,finalize"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
        void finalize (void) {}
        ]])],
        [AC_MSG_RESULT([yes])
         dsched_ld_have_fini=1],
        [AC_MSG_RESULT([no])
         dsched_ld_have_fini=0])
LDFLAGS=$LDFLAGS_save
DSCHED_VAR_SCOPE_POP

##################################
# Libraries
##################################

dsched_show_title "Library and Function tests"

DSCHED_SEARCH_LIBS_CORE([gethostbyname], [nsl])

DSCHED_SEARCH_LIBS_CORE([socket], [socket])

# Darwin doesn't need -lm, as it's a symlink to libSystem.dylib
DSCHED_SEARCH_LIBS_CORE([ceil], [m])

AC_CHECK_FUNCS([asprintf snprintf vasprintf vsnprintf fork setsid strsignal syslog setpgid fileno_unlocked isatty])

# On some hosts, htonl is a define, so the AC_CHECK_FUNC will get
# confused.  On others, it's in the standard library, but stubbed with
# the magic glibc foo as not implemented.  and on other systems, it's
# just not there.  This covers all cases.

AC_CACHE_CHECK([for htonl define],
  [ompi_cv_htonl_define],
  [AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif],[
#ifndef ntohl
#error "ntohl not defined"
#endif
])], [dsched_cv_htonl_define=yes], [dsched_cv_htonl_define=no])])

AC_CHECK_FUNC([htonl], [dsched_have_htonl=yes], [dsched_have_htonl=no])

AS_IF([test "$dsched_cv_htonl_define" = "yes" || test "$dsched_have_htonl" = "yes"],
    [AC_DEFINE_UNQUOTED([HAVE_UNIX_BYTESWAP], [1],
        [whether unix byteswap routines -- htonl, htons, nothl, ntohs -- are available])])

AC_CHECK_DECLS(__func__)

# checkpoint results
AC_CACHE_SAVE

##################################
# System-specific tests
##################################

dsched_show_title "System-specific tests"

# all: endian
AC_C_BIGENDIAN

#
# Check out what thread support we have
#

DSCHED_CONFIG_THREADS

#
# What is the local equivalent of "ln -s"
#

AC_PROG_LN_S
AC_PROG_GREP
AC_PROG_EGREP

#
# We need as and lex
#

AM_PROG_AS
AC_PROG_LEX(noyywrap)

# If we don't have Flex and we don't have a generated .c file
# (distribution tarballs will have the .c file included, but git
# clones will not), then error.  Must have Flex -- other versions of
# Lex are not workable (all things being equal, since this is *only*
# required for developers, we decided that it really was not worth it
# to be portable between different versions of lex ;-).

if test "x$LEX" != xflex && test ! -e $DSCHED_TOP_SRCDIR/src/util/hostfile/hostfile_lex.c; then
    AC_MSG_WARN([*** Could not find Flex on your system.])
    AC_MSG_WARN([*** Flex is required for developer builds of DSCHED.])
    AC_MSG_WARN([*** Other versions of Lex are not supported.])
    AC_MSG_WARN([*** NOTE: If you are building from an official tarball])
    AC_MSG_WARN([*** (not the ones made by GitHub!) downloaded from the])
    AC_MSG_WARN([*** DSCHED web site, you do not need Flex.])
    AC_MSG_ERROR([Cannot continue])
fi

#
# Do we have RLIMIT_NPROC in <sys/resources.h>?
#

AC_CHECK_DECLS([RLIMIT_NPROC], [], [], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif])

#
# Do we have RLIMIT_MEMLOCK in <sys/resources.h>?
#

AC_CHECK_DECLS([RLIMIT_MEMLOCK], [], [], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif])

#
# Do we have RLIMIT_NOFILE in <sys/resources.h>?
#

AC_CHECK_DECLS([RLIMIT_NOFILE], [], [], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif])

#
# Do we have RLIMIT_MEMLOCK in <sys/resources.h>?
#

AC_CHECK_DECLS([RLIMIT_FSIZE], [], [], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif])

#
# Do we have RLIMIT_CORE in <sys/resources.h>?
#

AC_CHECK_DECLS([RLIMIT_CORE], [], [], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif])

#
# Do we have RLIMIT_STACK in <sys/resources.h>?
#

AC_CHECK_DECLS([RLIMIT_STACK], [], [], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif])

#
# Do we have RLIMIT_AS in <sys/resources.h>?
#

AC_CHECK_DECLS([RLIMIT_AS], [], [], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif])

# We need Python if we are building in a git clone
# (not a distribution tarball)
AC_MSG_CHECKING([if we need Python])
dsched_need_python=no
AS_IF([test -d "${DSCHED_TOP_SRCDIR}/.git"],
      [dsched_need_python=yes],
      [dsched_need_python=no])
AC_MSG_RESULT([$dsched_need_python])
# if we need it, then check the min version
AS_IF([test "$dsched_need_python" = "yes"],
      [AM_PATH_PYTHON([$python_min_version],
                      [dsched_have_good_python=1],
                      [AC_MSG_ERROR([DSCHED requires Python >= $python_min_version to build. Aborting.])])],
      [dsched_have_good_python=0])

AS_IF([test $dsched_have_good_python -eq 1],
      [DSCHED_SUMMARY_ADD([Required Packages], [Python], [], [yes ($PYTHON_VERSION)])])

#
# Setup Sphinx processing
#
# Note that we have to double escape the URL below
# so that the # it contains doesn't confuse the Autotools

OAC_SETUP_SPHINX([$srcdir/docs/_build/html/index.html],
                 [[https://docs.dsched.org/en/main/developers/prerequisites.html#sphinx-and-therefore-python]],
                 [$srcdir/docs/requirements.txt])

AS_IF([test -n "$OAC_MAKEDIST_DISABLE"],
      [AS_IF([test -n "$DSCHED_MAKEDIST_DISABLE"],
             [DSCHED_MAKEDIST_DISABLE="$DSCHED_MAKEDIST_DISABLE $OAC_MAKEDIST_DISABLE"],
             [DSCHED_MAKEDIST_DISABLE=$OAC_MAKEDIST_DISABLE])])
AS_IF([test -n "$DSCHED_MAKEDIST_DISABLE"],
      [AC_MSG_WARN(["make dist" will be disabled due to: $DSCHED_MAKEDIST_DISABLE])])
AC_SUBST([DSCHED_MAKEDIST_DISABLE])

# checkpoint results
AC_CACHE_SAVE

##################################
# Libevent
##################################
dsched_show_title "Event library"

DSCHED_LIBEVENT_CONFIG

##################################
# PMIx
##################################

dsched_show_title "Configure PMIx"

DSCHED_CHECK_PMIX

# Check for LTO flags - adapt as necessary. Need to delay
# this check until after we check PMIx as we need to
# determine its compatibility first
_DSCHED_CHECK_LTO_FLAG($CPPFLAGS, CPPFLAGS)
_DSCHED_CHECK_LTO_FLAG($CFLAGS, CFLAGS)
_DSCHED_CHECK_LTO_FLAG($LDFLAGS, LDFLAGS)
_DSCHED_CHECK_LTO_FLAG($LIBS, LIBS)

##################################
# HWLOC
##################################

dsched_show_title "Configure HWLOC"

DSCHED_SETUP_HWLOC

##################################
# MCA
##################################

dsched_show_title "Modular Component Architecture (MCA) setup"

AC_MSG_CHECKING([for subdir args])
DSCHED_CONFIG_SUBDIR_ARGS([dsched_subdir_args])
AC_MSG_RESULT([$dsched_subdir_args])

DSCHED_MCA

# checkpoint results
AC_CACHE_SAVE

##################################
# Visibility
##################################
# Check the visibility declspec at the end to avoid problem with
# the previous tests that are not necessarily prepared for
# the visibility feature.

dsched_show_title "Symbol visibility feature"

DSCHED_CHECK_VISIBILITY

##################################
# STOP-ON-EXEC
##################################
# Check ptrace support for stop-on-exec

dsched_show_title "Ptrace stop-on-exec support"

DSCHED_CHECK_PTRACE

############################################################################
# Final top-level DSCHED configuration
############################################################################

dsched_show_title "Final top-level DSCHED configuration"

############################################################################
# Libtool: part two
# (after C compiler setup = no compiler/linker tests after this)
############################################################################
dsched_show_subtitle "Libtool configuration"

LT_INIT()
LT_LANG([C])


# What's the suffix of shared libraries?  Inspired by generated
# Libtool code (even though we don't support several of these
# platforms, there didn't seem to be any harm in leaving in some of
# them, alhtough I did remove some that we have never/will never
# support, like OS/2).
DSCHED_DYN_LIB_PREFIX=lib
case $host_os in
cygwin*)
    DSCHED_DYN_LIB_PREFIX=cyg
    DSCHED_DYN_LIB_SUFFIX=dll
    ;;
mingw* | pw32* | cegcc*)
    DSCHED_DYN_LIB_SUFFIX=dll
    ;;
darwin* | rhapsody*)
    DSCHED_DYN_LIB_SUFFIX=dylib
    ;;
hpux9* | hpux10* | hpux11*)
    case $host_cpu in
        ia64*)
        DSCHED_DYN_LIB_SUFFIX=so
        ;;
    *)
        DSCHED_DYN_LIB_SUFFIX=sl
        ;;
    esac
    ;;
*)
   DSCHED_DYN_LIB_SUFFIX=so
   ;;
esac

AC_SUBST(DSCHED_DYN_LIB_PREFIX)
AC_SUBST(DSCHED_DYN_LIB_SUFFIX)

# Need the libtool binary before the rpathify stuff
LT_OUTPUT

############################################################################
# final compiler config
############################################################################
dsched_show_subtitle "Compiler flags"

#
# This is needed for VPATH builds, so that it will -I the appropriate
# include directory.  We delayed doing it until now just so that
# '-I$(top_srcdir)' doesn't show up in any of the configure output --
# purely aesthetic.
#

# Because dsched_config.h is created by AC_CONFIG_HEADERS, we
# don't need to -I the builddir for dsched/include. However, we do
# need to add it for dsched as it doesn't have an AC_CONFIG_HEADERS that
# will install it for us. If we VPATH building, we do need to include the
# source directories, however.
#

if test "$DSCHED_TOP_BUILDDIR" != "$DSCHED_TOP_SRCDIR"; then
    # Note the embedded m4 directives here -- we must embed them
    # rather than have successive assignments to these shell
    # variables, lest the $(foo) names try to get evaluated here.
    # Yuck!
    cpp_includes="$DSCHED_TOP_SRCDIR $DSCHED_TOP_BUILDDIR $DSCHED_TOP_SRCDIR/src/include $DSCHED_TOP_BUILDDIR/src/include $DSCHED_TOP_BUILDDIR/include"
else
    cpp_includes="$DSCHED_TOP_SRCDIR $DSCHED_TOP_SRCDIR/src/include"
fi
CPP_INCLUDES="$(echo $cpp_includes | $SED 's/[[^ \]]* */'"$dsched_cc_iquote"'&/g')"
CPPFLAGS="$CPP_INCLUDES $CPPFLAGS $DSCHED_FINAL_CPPFLAGS"
LDFLAGS="$LDFLAGS $DSCHED_FINAL_LDFLAGS"
LIBS="$LIBS $DSCHED_FINAL_LIBS"


# restore any user-provided Werror flags
AS_IF([test ! -z "$DSCHED_CFLAGS_cache"], [CFLAGS="$CFLAGS $DSCHED_CFLAGS_cache"])

# setup "picky" compiler options if enabled
DSCHED_SETUP_PICKY_COMPILERS

#
# Delayed the substitution of CFLAGS and CXXFLAGS until now because
# they may have been modified throughout the course of this script.
#

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)

AC_MSG_CHECKING([final CPPFLAGS])
AC_MSG_RESULT([$CPPFLAGS])

AC_MSG_CHECKING([final CFLAGS])
AC_MSG_RESULT([$CFLAGS])

AC_MSG_CHECKING([final LDFLAGS])
AC_MSG_RESULT([$LDFLAGS])

AC_MSG_CHECKING([final LIBS])
AC_MSG_RESULT([$LIBS])


#
# Aggregate MCA parameters directory
#
AC_SUBST([AMCA_PARAM_SETS_DIR], ['$(dscheddatadir)/amca-param-sets'])

# If there is a local hook, call it.
m4_ifdef([dsched_CONFIG_LOCAL], [dsched_CONFIG_LOCAL])

############################################################################
# Party on
############################################################################

AC_MSG_CHECKING([if libtool needs -no-undefined flag to build shared libraries])

case "`uname`" in
  CYGWIN*|MINGW*|AIX*)
    ## Add in the -no-undefined flag to LDFLAGS for libtool.
    AC_MSG_RESULT([yes])
    LDFLAGS="$LDFLAGS -no-undefined"
    ;;
  *)
    ## Don't add in anything.
    AC_MSG_RESULT([no])
    ;;
esac

# dscheddatadir, dschedlibdir, and dschedinclude are essentially the same as
# pkg*dir, but will always be */dsched.

dscheddatadir='${datadir}/dsched'
dschedlibdir='${libdir}/dsched'
dschedincludedir='${includedir}/dsched'
AC_SUBST(dscheddatadir)
AC_SUBST(dschedlibdir)
AC_SUBST(dschedincludedir)

dsched_want_prd=0

dsched_show_subtitle "Final output"

AC_CONFIG_FILES([
    Makefile
    config/Makefile
    include/Makefile
    include/dsched_version.h
    contrib/Makefile
    docs/Makefile
    src/Makefile
    src/etc/Makefile
    src/util/Makefile
    src/tools/dsched/Makefile
    src/tools/dsched_info/Makefile
])


AC_OUTPUT

DSCHED_SUMMARY_PRINT
