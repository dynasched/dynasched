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

EXTRA_DIST = \
	dsched-convert-help.py

headers = \
        dsched_attr.h \
        dsched_error.h \
        dsched_cmd_line.h \
        dsched_daemon_init.h \
        dsched_session_dir.h

sources = \
        dsched_attr.c \
        dsched_error.c \
        dsched_show_help_content.c \
        dsched_daemon_init.c \
        dsched_session_dir.c

# In a git clone, regenerate the show_help content on every make so that
# an edit to any help-*.txt file is picked up without having to remove
# the generated file by hand.  The converter rewrites its output only
# when the content changes, so an untouched tree does not recompile.  A
# tarball ships the generated file and does not regenerate it.
if DSCHED_DEVEL_BUILD
dsched_show_help_deps = FORCE
else
dsched_show_help_deps = dsched-convert-help.py
endif

dsched_show_help_content.c: $(dsched_show_help_deps)
if DSCHED_PICKY_COMPILERS
	$(DSCHED_V_GEN) $(PYTHON) $(abs_srcdir)/dsched-convert-help.py \
		--root $(abs_top_srcdir) \
		--out dsched_show_help_content.c \
		--purge
else
	$(DSCHED_V_GEN) $(PYTHON) $(abs_srcdir)/dsched-convert-help.py \
		--root $(abs_top_srcdir) \
		--out dsched_show_help_content.c
endif

FORCE:

libdschedutil_la_SOURCES = $(headers) $(sources)

# Cross-check the help files against the show_help calls.  The generation
# rule above performs the same check in a picky build, but outside a
# developer build it depends only on the converter script, so editing a
# help file or a show_help call does not re-run it - hook the check into
# "make check" as well, where it runs every time.
check-local:
	$(PYTHON) $(abs_srcdir)/dsched-convert-help.py \
		--root $(abs_top_srcdir) \
		--check-only

MAINTAINERCLEANFILES = \
	dsched_show_help_content.c
if DSCHED_DEVEL_BUILD
CLEANFILES += dsched_show_help_content.c
endif
