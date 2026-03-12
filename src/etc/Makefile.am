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
# Copyright (c) 2008-2023 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2019-2020 Intel, Inc.  All rights reserved.
# Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

dsched_file_from_platform = @DSCHED_PARAM_FROM_PLATFORM@
dsched_mca_param_file = @DSCHED_DEFAULT_MCA_PARAM_CONF@

EXTRA_DIST = dsched-mca-params.conf

# Steal a little trickery from a generated Makefile to only install
# files if they do not already exist at the target.

install-data-local:
	@ $(MKDIR_P) $(DESTDIR)$(sysconfdir); \
	if test "$(dsched_file_from_platform)" = "yes"; then \
	    $(INSTALL_DATA) $(dsched_mca_param_file) $(DESTDIR)$(sysconfdir)/dsched-mca-params.conf; \
	else \
	    if test -f $(DESTDIR)$(sysconfdir)/dsched-mca-params.conf; then \
		echo "******************************* WARNING ************************************"; \
		echo "*** Not installing new dsched-mca-params.conf over existing file in:"; \
		echo "***   $(DESTDIR)$(sysconfdir)/dsched-mca-params.conf"; \
		echo "******************************* WARNING ************************************"; \
	    else \
		echo installing dsched-mca-params.conf to $(DESTDIR)$(sysconfdir)/; \
		$(INSTALL_DATA) $(srcdir)/dsched-mca-params.conf $(DESTDIR)$(sysconfdir)/; \
	    fi; \
	fi

# Only remove if exactly the same as what in our tree
# NOTE TO READER: Bourne shell if ... fi evaluates the body if
#    the return of the evaluated command is 0 (as opposed to non-zero
#    as used by everyone else)
uninstall-local:
	if test "$(dsched_file_from_platform)" = "yes"; then \
	    if test -f $(DESTDIR)$(sysconfdir)/dsched-mca-params.conf; then \
		    if diff "$(DESTDIR)$(sysconfdir)/dsched-mca-params.conf" "$(dsched_mca_param_file)" > /dev/null 2>&1 ; then \
		      echo "rm -f $(DESTDIR)$(sysconfdir)/dsched-mca-params.conf" ; \
		      rm -f "$(DESTDIR)$(sysconfdir)/dsched-mca-params.conf" ; \
		    fi ; \
		fi ; \
	else \
	    if test -f $(DESTDIR)$(sysconfdir)/dsched-mca-params.conf; then \
		    if diff "$(DESTDIR)$(sysconfdir)/dsched-mca-params.conf" "$(srcdir)/dsched-mca-params.conf"> /dev/null 2>&1 ; then \
		      echo "rm -f $(DESTDIR)$(sysconfdir)/dsched-mca-params.conf" ; \
		      rm -f "$(DESTDIR)$(sysconfdir)/dsched-mca-params.conf" ; \
		    fi ; \
		fi ; \
	fi ;
