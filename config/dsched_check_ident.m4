dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2007      Sun Microsystems, Inc.  All rights reserved.
dnl Copyright (c) 2015-2019 Intel, Inc.  All rights reserved.
dnl Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
dnl Copyright (c) 2026      Nanook Consulting  All rights reserved.
dnl $COPYRIGHT$
dnl
dnl Additional copyrights may follow
dnl
dnl $HEADER$
dnl
dnl defines:
dnl   DSCHED_$1_USE_PRAGMA_IDENT
dnl   DSCHED_$1_USE_IDENT
dnl   DSCHED_$1_USE_CONST_CHAR_IDENT
dnl

# DSCHED_CHECK_IDENT(compiler-env, compiler-flags,
# file-suffix, lang) Try to compile a source file containing
# a #pragma ident, and determine whether the ident was
# inserted into the resulting object file
# -----------------------------------------------------------
AC_DEFUN([DSCHED_CHECK_IDENT], [
    AC_MSG_CHECKING([for $4 ident string support])

    dsched_pragma_ident_happy=0
    dsched_ident_happy=0
    dsched_static_const_char_happy=0
    _DSCHED_CHECK_IDENT(
        [$1], [$2], [$3],
        [[#]pragma ident], [],
        [dsched_pragma_ident_happy=1
         dsched_message="[#]pragma ident"],
        _DSCHED_CHECK_IDENT(
            [$1], [$2], [$3],
            [[#]ident], [],
            [dsched_ident_happy=1
             dsched_message="[#]ident"],
            _DSCHED_CHECK_IDENT(
                [$1], [$2], [$3],
                [[#]pragma comment(exestr, ], [)],
                [dsched_pragma_comment_happy=1
                 dsched_message="[#]pragma comment"],
                [dsched_static_const_char_happy=1
                 dsched_message="static const char[[]]"])))

    AC_DEFINE_UNQUOTED([DSCHED_$1_USE_PRAGMA_IDENT],
        [$dsched_pragma_ident_happy], [Use #pragma ident strings for $4 files])
    AC_DEFINE_UNQUOTED([DSCHED_$1_USE_IDENT],
        [$dsched_ident_happy], [Use #ident strings for $4 files])
    AC_DEFINE_UNQUOTED([DSCHED_$1_USE_PRAGMA_COMMENT],
        [$dsched_pragma_comment_happy], [Use #pragma comment for $4 files])
    AC_DEFINE_UNQUOTED([DSCHED_$1_USE_CONST_CHAR_IDENT],
        [$dsched_static_const_char_happy], [Use static const char[] strings for $4 files])

    AC_MSG_RESULT([$dsched_message])

    unset dsched_pragma_ident_happy dsched_ident_happy dsched_static_const_char_happy dsched_message
])

# _DSCHED_CHECK_IDENT(compiler-env, compiler-flags,
# file-suffix, header_prefix, header_suffix, action-if-success, action-if-fail)
# Try to compile a source file containing a #-style ident,
# and determine whether the ident was inserted into the
# resulting object file
# -----------------------------------------------------------
AC_DEFUN([_DSCHED_CHECK_IDENT], [
    eval dsched_compiler="\$$1"
    eval dsched_flags="\$$2"

    dsched_ident="string_not_coincidentally_inserted_by_the_compiler"
    cat > conftest.$3 <<EOF
$4 "$dsched_ident" $5
int main(int argc, char** argv);
int main(int argc, char** argv) { return 0; }
EOF

    # "strings" won't always return the ident string.  objdump isn't
    # universal (e.g., OS X doesn't have it), and ...other
    # complications.  So just try to "grep" for the string in the
    # resulting object file.  If the ident is found in "strings" or
    # the grep succeeds, rule that we have this flavor of ident.

    echo "configure:__oline__: $1" >&5
    dsched_output=`$dsched_compiler $dsched_flags -c conftest.$3 -o conftest.${OBJEXT} 2>&1 1>/dev/null`
    dsched_status=$?
    AS_IF([test $dsched_status = 0],
          [test -z "$dsched_output"
           dsched_status=$?])
    DSCHED_LOG_MSG([\$? = $dsched_status], 1)
    AS_IF([test $dsched_status = 0 && test -f conftest.${OBJEXT}],
          [dsched_output="`strings -a conftest.${OBJEXT} | grep $dsched_ident`"
           grep $dsched_ident conftest.${OBJEXT} 2>&1 1>/dev/null
           dsched_status=$?
           AS_IF([test "$dsched_output" != "" || test "$dsched_status" = "0"],
                 [$6],
                 [$7])],
          [DSCHED_LOG_MSG([the failed program was:])
           DSCHED_LOG_FILE([conftest.$3])
           $7])

    unset dsched_compiler dsched_flags dsched_output dsched_status
    rm -rf conftest.* conftest${EXEEXT}
])dnl
