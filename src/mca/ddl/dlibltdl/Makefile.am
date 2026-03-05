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

noinst_LTLIBRARIES = libpmix_mca_ddl_plibltdl.la

libpmix_mca_ddl_plibltdl_la_SOURCES = $(sources)
libpmix_mca_ddl_plibltdl_la_CPPFLAGS = $(pmix_ddl_plibltdl_CPPFLAGS)
libpmix_mca_ddl_plibltdl_la_LDFLAGS = \
        $(pmix_ddl_plibltdl_LDFLAGS) \
        -module -avoid-version
libpmix_mca_ddl_plibltdl_la_LIBADD = $(pmix_ddl_plibltdl_LIBS)
