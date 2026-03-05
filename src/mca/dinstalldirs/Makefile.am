#
# Copyright (c) 2006      Los Alamos National Security, LLC.  All rights
#                         reserved.
# Copyright (c) 2010      Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2016      Intel, Inc. All rights reserved
# Copyright (c) 2022-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# main library setup
noinst_LTLIBRARIES = libdschedmca_dinstalldirs.la
libdschedmca_dinstalldirs_la_SOURCES =

# local files
headers = dinstalldirs.h
libdschedmca_dinstalldirs_la_SOURCES += $(headers)

include base/Makefile.am

distclean-local:
	rm -f base/static-components.h
