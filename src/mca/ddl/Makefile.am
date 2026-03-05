#
# Copyright (c) 2004-2010 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2010-2015 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# main library setup
noinst_LTLIBRARIES = libdschedmca_ddl.la
libdschedmca_ddl_la_SOURCES =

# local files
headers = ddl.h
libdschedmca_ddl_la_SOURCES += $(headers)

# Ensure that the man pages are rebuilt if the dsched_config.h file
# changes; a "good enough" way to know if configure was run again (and
# therefore the release date or version may have changed)
$(nodist_man_MANS): $(top_builddir)/src/include/dsched_config.h

# install the header files
dscheddir = $(dschedincludedir)/$(subdir)
nobase_dsched_HEADERS = $(headers)


include base/Makefile.am

distclean-local:
	rm -f base/static-components.h
	rm -f $(nodist_man_MANS)
