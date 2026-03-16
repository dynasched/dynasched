#
# Copyright (c) 2026      Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# main library setup
noinst_LTLIBRARIES = libdschedmca_dsched.la
libdschedmca_dsched_la_SOURCES =

# local files
headers = dsched.h
libdschedmca_dsched_la_SOURCES += $(headers)

# pkgdata setup
EXTRA_DIST =

# Conditionally install the header files
dscheddir = $(dschedincludedir)/$(subdir)
nobase_dsched_HEADERS = $(headers)

include base/Makefile.include

distclean-local:
	rm -f base/static-components.h
