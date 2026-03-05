dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2010-2015 Cisco Systems, Inc.  All rights reserved.
dnl Copyright (c) 2016-2017 Intel, Inc. All rights reserved.
dnl Copyright (c) 2016-2019 Research Organization for Information Science
dnl                         and Technology (RIST). All rights reserved.
dnl Copyright (c) 2026      Nanook Consulting  All rights reserved.
dnl $COPYRIGHT$
dnl
dnl Additional copyrights may follow
dnl
dnl $HEADER$
dnl

dnl There will only be one component used in this framework, and it will
dnl be selected at configure time by priority.  Components must set
dnl their priorities in their configure.m4 file.

dnl We only want one winning component (vs. STOP_AT_FIRST_PRIORITY,
dnl which will allow all components of the same priority who succeed to
dnl win)
m4_define(MCA_dsched_ddl_CONFIGURE_MODE, STOP_AT_FIRST)

AC_DEFUN([MCA_dsched_ddl_CONFIG],[
    DSCHED_HAVE_DDL_SUPPORT=0

    # If --disable-dlopen was used, then have all the components fail
    # (we still need to configure them all so that things like "make
    # dist" work", but we just want the MCA system to (artificially)
    # conclude that it can't build any of the components.
    AS_IF([test $DSCHED_ENABLE_DLOPEN_SUPPORT -eq 0],
          [want_ddl=0], [want_ddl=1])

    MCA_CONFIGURE_FRAMEWORK([ddl], [$want_ddl])

    # If we found no suitable static ddl component and dlopen support
    # was not specifically disabled, this is an error.
    AS_IF([test "$MCA_dsched_ddl_STATIC_COMPONENTS" = "" && \
           test $DSCHED_ENABLE_DLOPEN_SUPPORT -eq 1],
          [AC_MSG_WARN([Did not find a suitable static dsched ddl component])
           AC_MSG_WARN([You might need to install libltld (and its headers) or])
           AC_MSG_WARN([specify --disable-dlopen to configure.])
           AC_MSG_ERROR([Cannot continue])])

    # If we have a winning component (which, per above, will only
    # happen if --disable-dlopen was *not* specified), do some more
    # logic.
    AS_IF([test "$MCA_dsched_ddl_STATIC_COMPONENTS" != ""],
       [ # We had a winner -- w00t!

        DSCHED_HAVE_DDL_SUPPORT=1
        # If we added any -L flags to ADD_LDFLAGS, then we (might)
        # need to add those directories to LD_LIBRARY_PATH.
        # Otherwise, if we try to AC RUN_IFELSE anything here in
        # configure, it might die because it can't find the libraries
        # we just linked against.
        DSCHED_VAR_SCOPE_PUSH([dsched_ddl_base_found_l dsched_ddl_base_token dsched_ddl_base_tmp dsched_ddl_base_dir])
        dsched_ddl_base_found_l=0
        eval "dsched_ddl_base_tmp=\$dsched_ddl_${dsched_ddl_winner}_ADD_LIBS"
        for dsched_ddl_base_token in $dsched_ddl_base_tmp; do
            case $dsched_ddl_base_token in
            -l*) dsched_ddl_base_found_l=1 ;;
            esac
        done
        AS_IF([test $dsched_ddl_base_found_l -eq 1],
              [eval "dsched_ddl_base_tmp=\$dsched_ddl_${dsched_ddl_winner}_ADD_LDFLAGS"
               for dsched_ddl_base_token in $dsched_ddl_base_tmp; do
                   case $dsched_ddl_base_token in
                   -L*)
                       dsched_ddl_base_dir=`echo $dsched_ddl_base_token | cut -c3-`
                       export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$dsched_ddl_base_dir
                       AC_MSG_WARN([Adding to LD_LIBRARY_PATH: $dsched_ddl_base_dir])
                       ;;
                   esac
               done])
        DSCHED_VAR_SCOPE_POP
    ])

    AC_DEFINE_UNQUOTED([DSCHED_HAVE_DDL_SUPPORT], [$DSCHED_HAVE_DDL_SUPPORT],
                       [Whether the DSCHED DDL framework is functional or not])
])
