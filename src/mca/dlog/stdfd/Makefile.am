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
# Copyright (c) 2010      Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2014-2019 Intel, Inc.  All rights reserved.
# Copyright (c) 2017      IBM Corporation.  All rights reserved.
# Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

sources = \
        log_stdfd.h \
        log_stdfd.c \
        log_stdfd_component.c

# Make the output library in this directory, and name it either
# mca_<type>_<name>.la (for DSO builds) or libmca_<type>_<name>.la
# (for static builds).

if MCA_BUILD_dsched_dlog_stdfd_DSO
component_noinst =
component_install = dsched_mca_dlog_stdfd.la
else
component_noinst = libdsched_mca_dlog_stdfd.la
component_install =
endif

mcacomponentdir = $(dschedlibdir)
mcacomponent_LTLIBRARIES = $(component_install)
dsched_mca_dlog_stdfd_la_SOURCES = $(sources)
dsched_mca_dlog_stdfd_la_LDFLAGS = -module -avoid-version
dsched_mca_dlog_stdfd_la_LIBADD = $(top_builddir)/src/libdsched.la

noinst_LTLIBRARIES = $(component_noinst)
libdsched_mca_dlog_stdfd_la_SOURCES =$(sources)
libdsched_mca_dlog_stdfd_la_LDFLAGS = -module -avoid-version
