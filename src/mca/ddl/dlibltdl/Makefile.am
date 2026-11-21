#
# Copyright (c) 2004-2010 The Trustees of Indiana University.
#                         All rights reserved.
# Copyright (c) 2014-2015 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2017      Intel, Inc.  All rights reserved.
# Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

sources = \
        ddl_libltdl.h \
        ddl_libltdl_component.c \
        ddl_libltdl_module.c

# This component will only ever be built statically -- never as a DSO.

noinst_LTLIBRARIES = libdsched_mca_ddl_dlibltdl.la

libdsched_mca_ddl_dlibltdl_la_SOURCES = $(sources)
libdsched_mca_ddl_dlibltdl_la_CPPFLAGS = $(dsched_ddl_dlibltdl_CPPFLAGS)
libdsched_mca_ddl_dlibltdl_la_LDFLAGS = \
        $(dsched_ddl_dlibltdl_LDFLAGS) \
        -module -avoid-version
libdsched_mca_ddl_dlibltdl_la_LIBADD = $(dsched_ddl_dlibltdl_LIBS)
