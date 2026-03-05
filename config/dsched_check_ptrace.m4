dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2020      Intel, Inc.  All rights reserved.
dnl Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
dnl Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
dnl $COPYRIGHT$
dnl
dnl Additional copyrights may follow
dnl
dnl $HEADER$
dnl

# See if there is support for ptrace options required for
# "stop-on-exec" behavior.

AC_DEFUN([DSCHED_CHECK_PTRACE],[

    DSCHED_VAR_SCOPE_PUSH(dsched_have_ptrace_traceme dsched_have_ptrace_detach dsched_have_ptrace_header dsched_have_ptrace dsched_want_stop_on_exec dsched_traceme_cmd dsched_detach_cmd dsched_ptrace_linux_sig dsched_ptrace_CFLAGS_save)

    dsched_have_ptrace_traceme=no
    dsched_have_ptrace_detach=no
    dsched_traceme_cmd=
    dsched_detach_cmd=

    AC_CHECK_HEADER([sys/ptrace.h],
                    [dsched_have_ptrace_header=1
                     # must manually define the header protection since check_header doesn't do it
                     AC_DEFINE_UNQUOTED([HAVE_SYS_PTRACE_H], [1], [Whether or not we have the ptrace header])],
                    [dsched_have_ptrace_header=0])

    AC_CHECK_FUNC([ptrace],
                  [dsched_have_ptrace=yes],
                  [dsched_have_ptrace=no])

    if test "$dsched_have_ptrace_header" = "1" && test "$dsched_have_ptrace" = "yes"; then
        AC_MSG_CHECKING([PTRACE_TRACEME])
        AC_EGREP_CPP([yes],
                     [#include <sys/ptrace.h>
                      #ifdef PTRACE_TRACEME
                        yes
                      #endif
                     ],
                     [AC_MSG_RESULT(yes)
                      dsched_have_ptrace_traceme=yes
                      dsched_traceme_cmd=PTRACE_TRACEME],
                     [AC_MSG_RESULT(no)
                      AC_MSG_CHECKING([PT_TRACE_ME])
                      AC_EGREP_CPP([yes],
                                   [#include <sys/ptrace.h>
                                    #ifdef PT_TRACE_ME
                                      yes
                                    #endif
                                   ],
                                   [AC_MSG_RESULT(yes)
                                    dsched_have_ptrace_traceme=yes
                                    dsched_traceme_cmd=PT_TRACE_ME],
                                   [AC_MSG_RESULT(no)
                                    dsched_have_ptrace_traceme=no])
                     ])

        AC_MSG_CHECKING([PTRACE_DETACH])
        AC_EGREP_CPP([yes],
                     [#include <sys/ptrace.h>
                      #ifdef PTRACE_DETACH
                        yes
                      #endif
                     ],
                     [AC_MSG_RESULT(yes)
                      dsched_have_ptrace_detach=yes
                      dsched_detach_cmd=PTRACE_DETACH],
                     [AC_MSG_RESULT(no)
                      AC_MSG_CHECKING(PT_DETACH)
                      AC_EGREP_CPP([yes],
                                   [#include <sys/ptrace.h>
                                    #ifdef PT_DETACH
                                      yes
                                    #endif
                                   ],
                                   [AC_MSG_RESULT(yes)
                                    dsched_have_ptrace_detach=yes
                                    dsched_detach_cmd=PT_DETACH],
                                   [AC_MSG_RESULT(no)
                                    dsched_have_ptrace_detach=no])
                     ])

        AC_MSG_CHECKING([Linux ptrace function signature])
        AC_LANG_PUSH(C)
        # must have -Werror set here
        dsched_ptrace_CFLAGS_save=$CFLAGS
        CFLAGS="$CFLAGS -Werror"
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
                [[#include "sys/ptrace.h"]],
                [[long (*ptr)(enum __ptrace_request request, pid_t pid, void *addr, void *data);]
                 [ptr = ptrace;]])
            ],[
                AC_MSG_RESULT([yes])
                dsched_ptrace_linux_sig=1
            ],[
                AC_MSG_RESULT([no])
                dsched_ptrace_linux_sig=0
            ])
        AC_LANG_POP(C)
        CFLAGS=$dsched_ptrace_CFLAGS_save

    fi

    AC_MSG_CHECKING(ptrace stop-on-exec will be supported)
    AS_IF([test "$dsched_have_ptrace_traceme" = "yes" && test "$dsched_have_ptrace_detach" = "yes"],
          [AC_MSG_RESULT(yes)
           dsched_want_stop_on_exec=1],
          [AC_MSG_RESULT(no)
           dsched_want_stop_on_exec=0])

    AC_DEFINE_UNQUOTED([DSCHED_HAVE_LINUX_PTRACE], [$dsched_ptrace_linux_sig], [Does ptrace have the Linux signature])
    AC_DEFINE_UNQUOTED([DSCHED_HAVE_STOP_ON_EXEC], [$dsched_want_stop_on_exec], [Whether or not we have stop-on-exec support])
    AC_DEFINE_UNQUOTED([DSCHED_TRACEME], [$dsched_traceme_cmd], [Command for declaring that process expects to be traced by parent])
    AC_DEFINE_UNQUOTED([DSCHED_DETACH], [$dsched_detach_cmd], [Command to detach from process being traced])

    DSCHED_VAR_SCOPE_POP
])
