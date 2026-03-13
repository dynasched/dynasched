#
# Copyright (c) 2010-2020 Cisco Systems, Inc.  All rights reserved
# Copyright (c) 2016-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2017      IBM Corporation.  All rights reserved.
# Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

sources = \
        sched_fifo.h \
        sched_fifo.c \
        sched_fifo_component.c

# Make the output library in this directory, and name it either
# mca_<type>_<name>.la (for DSO builds) or libmca_<type>_<name>.la
# (for static builds).

if MCA_BUILD_dsched_dsched_fifo_DSO
component_noinst =
component_install = dsched_mca_dsched_fifo.la
else
component_noinst = libdsched_mca_dsched_fifo.la
component_install =
endif

mcacomponentdir = $(dschedlibdir)
mcacomponent_LTLIBRARIES = $(component_install)
dsched_mca_dsched_fifo_la_SOURCES = $(sources)
dsched_mca_dsched_fifo_la_LDFLAGS = -module -avoid-version
dsched_mca_dsched_fifo_la_LIBADD = $(top_builddir)/src/libdsched.la

noinst_LTLIBRARIES = $(component_noinst)
libdsched_mca_dsched_fifo_la_SOURCES =$(sources)
libdsched_mca_dsched_fifo_la_LDFLAGS = -module -avoid-version
