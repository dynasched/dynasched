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
# Copyright (c) 2009-2020 Cisco Systems, Inc.  All rights reserved
# Copyright (c) 2015      Los Alamos National Security, LLC. All rights
#                         reserved.
# Copyright (c) 2019      Research Organization for Information Science
#                         and Technology (RIST).  All rights reserved.
# Copyright (c) 2019-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# Copyright (c) 2021      Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# Copyright (c) 2023      Jeffrey M. Squyres.  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

SUBDIRS = \
	etc \
	util \
	$(MCA_dsched_FRAMEWORKS_SUBDIRS) \
	$(MCA_dsched_FRAMEWORK_COMPONENT_STATIC_SUBDIRS) \
	. \
	$(MCA_dsched_FRAMEWORK_COMPONENT_DSO_SUBDIRS)

DIST_SUBDIRS = \
	etc \
	util \
	$(MCA_dsched_FRAMEWORKS_SUBDIRS) \
	$(MCA_dsched_FRAMEWORK_COMPONENT_ALL_SUBDIRS)

# Build the main DSCHED library

lib_LTLIBRARIES = libdsched.la
libdsched_la_SOURCES =

libdsched_la_LIBADD = \
	util/libdschedutil.la \
	$(MCA_dsched_FRAMEWORK_LIBS)
libdsched_la_DEPENDENCIES = \
	util/libdschedutil.la \
	$(MCA_dsched_FRAMEWORK_LIBS)
libdsched_la_LDFLAGS = -version-info $(libdsched_so_version)
libdsched_la_CPPFLAGS =

# included subdirectory Makefile.am's and appended-to variables
headers =
nodist_headers =
noinst_LTLIBRARIES =
EXTRA_DIST =
libdsched_la_SOURCES += $(headers)

include include/Makefile.include
include runtime/Makefile.include
include tools/Makefile.include
include event/Makefile.include

