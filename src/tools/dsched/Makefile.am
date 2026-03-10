#
# Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2005 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2009 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2008-2020 Cisco Systems, Inc.  All rights reserved
# Copyright (c) 2008      Sun Microsystems, Inc.  All rights reserved.
# Copyright (c) 2015-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

AM_LDFLAGS = $(dsched_hwloc_LDFLAGS) $(dsched_libevent_LDFLAGS) $(dsched_pmix_LDFLAGS)
AM_CFLAGS = \
            -DDSCHED_CONFIGURE_USER="\"@DSCHED_CONFIGURE_USER@\"" \
            -DDSCHED_CONFIGURE_HOST="\"@DSCHED_CONFIGURE_HOST@\"" \
            -DDSCHED_CONFIGURE_DATE="\"@DSCHED_CONFIGURE_DATE@\"" \
            -DDSCHED_BUILD_USER="\"$$USER\"" \
            -DDSCHED_BUILD_HOST="\"$${HOSTNAME:-`(hostname || uname -n) | sed 1q`}\"" \
            -DDSCHED_BUILD_DATE="\"`$(top_srcdir)/config/getdate.sh`\"" \
            -DDSCHED_BUILD_CFLAGS="\"@CFLAGS@\"" \
            -DDSCHED_BUILD_CPPFLAGS="\"@CPPFLAGS@\"" \
            -DDSCHED_BUILD_LDFLAGS="\"@LDFLAGS@\"" \
            -DDSCHED_BUILD_LIBS="\"@LIBS@\"" \
            -DDSCHED_CC_ABSOLUTE="\"@DSCHED_CC_ABSOLUTE@\"" \
            -DDSCHED_GREEK_VERSION="\"@DSCHED_GREEK_VERSION@\"" \
            -DDSCHED_REPO_REV="\"@DSCHED_REPO_REV@\"" \
            -DDSCHED_RELEASE_DATE="\"@DSCHED_RELEASE_DATE@\""

bin_PROGRAMS = dsched

dsched_SOURCES =  \
        dsched.h \
        dsched.c \
        server.c \
        backend.c \
        session.c \
        queries.c

dsched_LDADD = \
    $(dsched_libevent_LIBS) \
    $(dsched_hwloc_LIBS) \
    $(dsched_pmix_LIBS) \
	$(top_builddir)/src/libdsched.la

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/dsched$(EXEEXT)

EXTRA_DIST = \
    help-dsched.txt
