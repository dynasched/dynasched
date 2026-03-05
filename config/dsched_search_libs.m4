dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2013-2020 Cisco Systems, Inc.  All rights reserved
dnl Copyright (c) 2014-2019 Intel, Inc.  All rights reserved.
dnl Copyright (c) 2026      Nanook Consulting  All rights reserved.
dnl $COPYRIGHT$
dnl
dnl Additional copyrights may follow
dnl
dnl $HEADER$
dnl

# DSCHED SEARCH_LIBS_CORE(func, list-of-libraries,
#                       action-if-found, action-if-not-found,
#                       other-libraries)
#
# Wrapper around AC SEARCH_LIBS.  If a library ends up being added to
# $LIBS, then also add it to the wrapper LIBS list (so that it is
# added to the link command line for the static link case).
#
# NOTE: COMPONENTS SHOULD NOT USE THIS MACRO!  Components should use
# DSCHED_SEARCH_LIBS_COMPONENT.  The reason why is because this macro
# calls DSCHED_WRAPPER_FLAGS_ADD -- see big comment in
# DSCHED_setup_wrappers.m4 for an explanation of why this is bad).
# NOTE: DSCHED doesn't have wrapper compilers, so this is not an issue
# here - we leave the note just for downstream compatibility
AC_DEFUN([DSCHED_SEARCH_LIBS_CORE],[

    DSCHED_VAR_SCOPE_PUSH([LIBS_save add])
    LIBS_save=$LIBS

    AC_SEARCH_LIBS([$1], [$2],
        [DSCHED_have_$1=1
         $3],
        [DSCHED_have_$1=0
         $4], [$5])

    AC_DEFINE_UNQUOTED([DSCHED_HAVE_]m4_toupper($1), [$DSCHED_have_$1],
         [whether $1 is found and available])

    DSCHED_VAR_SCOPE_POP
])dnl

# DSCHED SEARCH_LIBS_COMPONENT(prefix, func, list-of-libraries,
#                            action-if-found, action-if-not-found,
#                            other-libraries)
#
# Same as DSCHED SEARCH_LIBS_CORE, above, except that we don't call DSCHED
# WRAPPER_FLAGS_ADD.  Instead, we add it to the ${prefix}_LIBS
# variable (i.e., $prefix is usually "framework_component", such as
# "fbtl_posix").
AC_DEFUN([DSCHED_SEARCH_LIBS_COMPONENT],[

    DSCHED_VAR_SCOPE_PUSH([LIBS_save add])
    LIBS_save=$LIBS

    AC_SEARCH_LIBS([$2], [$3],
        [ # Found it!  See if anything was added to LIBS
         add=`printf '%s\n' "$LIBS" | sed -e "s/$LIBS_save$//"`
         AS_IF([test -n "$add"],
             [DSCHED_FLAGS_APPEND_UNIQ($1_LIBS, [$add])])
         $1_have_$2=1
         $4],
        [$1_have_$2=0
         $5], [$6])

        AC_DEFINE_UNQUOTED([DSCHED_HAVE_]m4_toupper($1), [$$1_have_$2],
            [whether $1 is found and available])
    DSCHED_VAR_SCOPE_POP
])dnl
