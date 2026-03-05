dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
dnl                         University Research and Technology
dnl                         Corporation.  All rights reserved.
dnl Copyright (c) 2004-2005 The University of Tennessee and The University
dnl                         of Tennessee Research Foundation.  All rights
dnl                         reserved.
dnl Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
dnl                         University of Stuttgart.  All rights reserved.
dnl Copyright (c) 2004-2005 The Regents of the University of California.
dnl                         All rights reserved.
dnl Copyright (c) 2007      Sun Microsystems, Inc.  All rights reserved.
dnl Copyright (c) 2009      Oak Ridge National Labs.  All rights reserved.
dnl Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved.
dnl Copyright (c) 2013-2020 Intel, Inc.  All rights reserved.
dnl Copyright (c) 2017      Research Organization for Information Science
dnl                         and Technology (RIST). All rights reserved.
dnl
dnl Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
dnl Copyright (c) 2021-2022 Amazon.com, Inc. or its affiliates.  All Rights reserved.
dnl $COPYRIGHT$
dnl
dnl Additional copyrights may follow
dnl
dnl $HEADER$
dnl
dnl Portions of this file derived from GASNet v1.12 (see "GASNet"
dnl comments, below)
dnl Copyright 2004,  Dan Bonachea <bonachea@cs.berkeley.edu>
dnl
dnl IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
dnl DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
dnl OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
dnl CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl
dnl THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
dnl INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
dnl AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
dnl ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
dnl PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
dnl

AC_DEFUN([DSCHED_CONFIGURE_SETUP],[

# Some helper script functions.  Unfortunately, we cannot use $1 kinds
# of arguments here because of the m4 substitution.  So we have to set
# special variable names before invoking the function.  :-\

dsched_show_title() {
  cat <<EOF

============================================================================
== ${1}
============================================================================
EOF
  DSCHED_LOG_MSG([=== ${1}], 1)
}


dsched_show_subtitle() {
  cat <<EOF

*** ${1}
EOF
  DSCHED_LOG_MSG([*** ${1}], 1)
}


dsched_show_subsubtitle() {
  cat <<EOF

+++ ${1}
EOF
  DSCHED_LOG_MSG([+++ ${1}], 1)
}

dsched_show_subsubsubtitle() {
  cat <<EOF

--- ${1}
EOF
  DSCHED_LOG_MSG([--- ${1}], 1)
}

dsched_show_verbose() {
  if test "$V" = "1"; then
      cat <<EOF
+++ VERBOSE: ${1}
EOF
      DSCHED_LOG_MSG([--- ${1}], 1)
  fi
}

#
# Save some stats about this build
#

DSCHED_CONFIGURE_USER="${USER:-`whoami`}"
DSCHED_CONFIGURE_HOST="${HOSTNAME:-`(hostname || uname -n) 2> /dev/null | sed 1q`}"
DSCHED_CONFIGURE_DATE="`$abs_srcdir/config/getdate.sh`"


AC_SUBST([SOURCE_DATE_EPOCH])
AM_CONDITIONAL([SOURCE_DATE_EPOCH_SET], [test -n "$SOURCE_DATE_EPOCH"])

#
# Save these details so that they can be used in dsched_info later
#
AC_SUBST(DSCHED_CONFIGURE_USER)
AC_DEFINE_UNQUOTED([DSCHED_CONFIGURE_USER], "$DSCHED_CONFIGURE_USER",
                   [User who built DSChed])
AC_SUBST(DSCHED_CONFIGURE_HOST)
AC_DEFINE_UNQUOTED([DSCHED_CONFIGURE_HOST], "$DSCHED_CONFIGURE_HOST",
                   [Hostname where DSChed was built])
AC_SUBST(DSCHED_CONFIGURE_DATE)
AC_DEFINE_UNQUOTED([DSCHED_CONFIGURE_DATE], "$DSCHED_CONFIGURE_DATE",
                   [Date when DSChed was built])
])dnl

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

AC_DEFUN([DSCHED_BASIC_SETUP],[

#
# Make automake clean emacs ~ files for "make clean"
#

CLEANFILES="*~ .\#*"
AC_SUBST(CLEANFILES)

#
# See if we can find an old installation of DSCHED to overwrite
#

# Stupid autoconf 2.54 has a bug in AC_PREFIX_PROGRAM -- if dsched_clean
# is not found in the path and the user did not specify --prefix,
# we'll get a $prefix of "."

dsched_prefix_save="$prefix"
AC_PREFIX_PROGRAM(dsched_clean)
if test "$prefix" = "."; then
    prefix="$dsched_prefix_save"
fi
unset dsched_prefix_save

#
# Basic sanity checking; we can't install to a relative path
#

case "$prefix" in
  /*/bin)
    prefix="`dirname $prefix`"
    echo installing to directory \"$prefix\"
    ;;
  /*)
    echo installing to directory \"$prefix\"
    ;;
  NONE)
    echo installing to directory \"$ac_default_prefix\"
    ;;
  @<:@a-zA-Z@:>@:*)
    echo installing to directory \"$prefix\"
    ;;
  *)
    AC_MSG_ERROR(prefix "$prefix" must be an absolute directory path)
    ;;
esac

# BEGIN: Derived from GASNet

# Suggestion from Paul Hargrove to disable --program-prefix and
# friends.  Heavily influenced by GASNet 1.12 acinclude.m4
# functionality to do the same thing (copyright listed at top of this
# file).

# echo program_prefix=$program_prefix  program_suffix=$program_suffix program_transform_name=$program_transform_name
# undo prefix autoconf automatically adds during cross-compilation
if test "$cross_compiling" = yes && test "$program_prefix" = "${target_alias}-" ; then
    program_prefix=NONE
fi
# normalize empty prefix/suffix
if test -z "$program_prefix" ; then
    program_prefix=NONE
fi
if test -z "$program_suffix" ; then
    program_suffix=NONE
fi
# undo transforms caused by empty prefix/suffix
if expr "$program_transform_name" : 's.^..$' >/dev/null || \
   expr "$program_transform_name" : 's.$$..$' >/dev/null || \
   expr "$program_transform_name" : 's.$$..;s.^..$' >/dev/null ; then
    program_transform_name="s,x,x,"
fi
if test "$program_prefix$program_suffix$program_transform_name" != "NONENONEs,x,x," ; then
    AC_MSG_WARN([*** The DSChed configure script does not support --program-prefix, --program-suffix or --program-transform-name. Users are recommended to instead use --prefix with a unique directory and make symbolic links as desired for renaming.])
    AC_MSG_ERROR([*** Cannot continue])
fi

# END: Derived from GASNet
])dnl

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

# 1 is the message
# 2 is whether to put a prefix or not
AC_DEFUN([DSCHED_LOG_MSG],
[AS_IF([test -n "$2"], [OAC_LOG_MSG([$1])], [OAC_LOG_MSG_NOPREFIX([$1])])])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_LOG_FILE], [DSCHED_LOG_FILE])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_LOG_COMMAND], [DSCHED_LOG_COMMAND])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_UNIQ], [DSCHED_UNIQ])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_APPEND], [DSCHED_APPEND])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_APPEND_UNIQ], [DSCHED_APPEND_UNIQ])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_FLAGS_APPEND_UNIQ], [DSCHED_FLAGS_APPEND_UNIQ])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_FLAGS_PREPEND_UNIQ], [DSCHED_FLAGS_PREPEND_UNIQ])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_FLAGS_APPEND_MOVE], [DSCHED_FLAGS_APPEND_MOVE])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

# Macro that serves as an alternative to using `which <prog>`. It is
# preferable to simply using `which <prog>` because backticks (`) (aka
# backquotes) invoke a sub-shell which may source a "noisy"
# ~/.whatever file (and we do not want the error messages to be part
# of the assignment in foo=`which <prog>`). This macro ensures that we
# get a sane executable value.
AC_DEFUN([DSCHED_WHICH],[
# 1 is the variable name to do "which" on
# 2 is the variable name to assign the return value to

DSCHED_VAR_SCOPE_PUSH([dsched_prog dsched_file dsched_dir dsched_sentinel])

dsched_prog=$1

IFS_SAVE=$IFS
IFS="$PATH_SEPARATOR"
for dsched_dir in $PATH; do
    if test -x "$dsched_dir/$dsched_prog"; then
        $2="$dsched_dir/$dsched_prog"
        break
    fi
done
IFS=$IFS_SAVE

DSCHED_VAR_SCOPE_POP
])dnl

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

m4_copy([OAC_VAR_SCOPE_PUSH], [DSCHED_VAR_SCOPE_PUSH])
m4_copy([OAC_VAR_SCOPE_POP], [DSCHED_VAR_SCOPE_POP])

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

#
# DSCHED_WITH_OPTION_MIN_MAX_VALUE(NAME,DEFAULT_VALUE,LOWER_BOUND,UPPER_BOUND)
# Defines a variable DSCHED_MAX_xxx, with "xxx" being specified as parameter $1 as "variable_name".
# If not set at configure-time using --with-max-xxx, the default-value ($2) is assumed.
# If set, value is checked against lower (value >= $3) and upper bound (value <= $4)
#
AC_DEFUN([DSCHED_WITH_OPTION_MIN_MAX_VALUE], [
    max_value=[$2]
    AC_MSG_CHECKING([maximum length of ]m4_translit($1, [_], [ ]))
    AC_ARG_WITH([max-]m4_translit($1, [_], [-]),
        AS_HELP_STRING([--with-max-]m4_translit($1, [_], [-])[=VALUE],
                       [maximum length of ]m4_translit($1, [_], [ ])[s.  VALUE argument has to be specified (default: [$2]).]))
    if test ! -z "$with_max_[$1]" && test "$with_max_[$1]" != "no" ; then
        # Ensure it's a number (hopefully an integer!), and >0
        expr $with_max_[$1] + 1 > /dev/null 2> /dev/null
        AS_IF([test "$?" != "0"], [happy=0],
              [AS_IF([test $with_max_[$1] -ge $3 && test $with_max_[$1] -le $4],
                     [happy=1], [happy=0])])

        # If badness in the above tests, bail
        AS_IF([test "$happy" = "0"],
              [AC_MSG_RESULT([bad value ($with_max_[$1])])
               AC_MSG_WARN([--with-max-]m4_translit($1, [_], [-])[s value must be >= $3 and <= $4])
               AC_MSG_ERROR([Cannot continue])])
        max_value=$with_max_[$1]
    fi
    AC_MSG_RESULT([$max_value])
    AC_DEFINE_UNQUOTED([DSCHED_MAX_]m4_toupper($1), $max_value,
                       [Maximum length of ]m4_translit($1, [_], [ ])[s (default is $2)])
    [DSCHED_MAX_]m4_toupper($1)=$max_value
    AC_SUBST([DSCHED_MAX_]m4_toupper($1))
])dnl

dnl #######################################################################
dnl #######################################################################
dnl #######################################################################

# Usage: DSCHED_COMPUTE_MAX_VALUE(number_bytes, variable_to_set, action if overflow)
# Compute maximum value of datatype of
# number_bytes, setting the result in the second argument.  Assumes a
# signed datatype.
AC_DEFUN([DSCHED_COMPUTE_MAX_VALUE], [
    # This is more complicated than it really should be.  But some
    # expr implementations (OpenBSD) have an expr with a max value of
    # 2^31 - 1, and we sometimes want to compute the max value of a
    # type as big or bigger than that...
    dsched_num_bits=`expr $1 \* 8 - 1`
    newval=1
    value=1
    overflow=0

    while test $dsched_num_bits -ne 0 ; do
        newval=`expr $value \* 2`
        if test 0 -eq `expr $newval \< 0` ; then
            # if the new value is not negative, next iteration...
            value=$newval
            dsched_num_bits=`expr $dsched_num_bits - 1`
            # if this was the last iteration, subtract 1 (as signed
            # max positive is 2^num_bits - 1).  Do this here instead
            # of outside of the while loop because we might have
            # already subtracted 1 by then if we're trying to find the
            # max value of the same datatype expr uses as it's
            # internal representation (ie, if we hit the else
            # below...)
            if test 0 -eq $dsched_num_bits ; then
                value=`expr $value - 1`
            fi
        else
            # if the new value is negative, we've over flowed.  First,
            # try adding value - 1 instead of value (see if we can get
            # to positive max of expr)
            newval=`expr $value - 1 + $value`
            if test 0 -eq `expr $newval \< 0` ; then
                value=$newval
                # Still positive, this is as high as we can go.  If
                # dsched_num_bits is 1, we didn't actually overflow.
                # Otherwise, we overflowed.
                if test 1 -ne $dsched_num_bits ; then
                    overflow=1
                fi
            else
                # still negative.  Time to give up.
                overflow=1
            fi
            dsched_num_bits=0
        fi
    done

    AS_VAR_SET([$2], [$value])
    AS_IF([test $overflow -ne 0], [$3])
])dnl
