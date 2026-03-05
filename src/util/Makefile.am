# -*- makefile -*-
#
# Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2005 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2007-2016 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2013      NVIDIA Corporation.  All rights reserved.
# Copyright (c) 2013-2019 Intel, Inc.  All rights reserved.
# Copyright (c) 2016      Research Organization for Information Science
#                         and Technology (RIST). All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# Copyright (c) 2022      Triad National Security, LLC. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# Source code files

AM_CPPFLAGS = \
    $(LTDLINCL)

include $(top_srcdir)/Makefile.dsched-rules

noinst_LTLIBRARIES = libdschedutil.la

EXTRA_DIST =

headers = \
        dsched_attr.h \
        dsched_error.h \
        dsched_cmd_line.h

sources = \
        dsched_attr.c \
        dsched_error.c

dsched_show_help_content.c: $(top_srcdir)/contrib/dsched-convert-help.py
if DSCHED_PICKY_COMPILERS
	$(PMIX_V_GEN) $(PYTHON) $(top_srcdir)/contrib/dsched-convert-help.py \
		--root $(top_srcdir)/src \
		--out $(top_srcdir)/src/util/dsched_show_help_content.c \
		--purge
else
	$(PMIX_V_GEN) $(PYTHON) $(top_srcdir)/contrib/dsched-convert-help.py \
		--root $(top_srcdir)/src \
		--out $(top_srcdir)/src/util/dsched_show_help_content.c
endif

libdschedutil_la_SOURCES = $(headers) $(sources)


clean-local:
	rm -f dsched_show_help_content.c dsched_show_help_content.lo

MAINTAINERCLEANFILES = \
	dsched_show_help_content.c

distclean-local:
	rm -f dsched_show_help_content.c
