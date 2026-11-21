#
# Copyright (c) 2026      Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

AM_LDFLAGS = $(dsched_hwloc_LDFLAGS) $(dsched_libevent_LDFLAGS) $(dsched_pmix_LDFLAGS)

bin_PROGRAMS = dsterm

dsterm_SOURCES = \
        dsterm.c

dsterm_LDADD = \
    $(dsched_libevent_LIBS) \
    $(dsched_hwloc_LIBS) \
    $(dsched_pmix_LIBS) \
    $(top_builddir)/src/libdsched.la

EXTRA_DIST = \
    help-dsterm.txt
