#!/bin/sh
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
# Copyright (c) 2009-2015 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2015-2019 Research Organization for Information Science
#                         and Technology (RIST).  All rights reserved.
# Copyright (c) 2015      Los Alamos National Security, LLC. All rights
#                         reserved.
# Copyright (c) 2017      Intel, Inc. All rights reserved.
# Copyright (c) 2026      Nanook Consulting  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

srcdir=$1
builddir=$PWD
distdir=$builddir/$2
DSCHED_REPO_REV=$3

if test x"$2" = x ; then
    echo "*** ERROR: Must supply relative distdir as argv[2] -- aborting"
    exit 1
elif test ! -d "$distdir" ; then
    echo "*** ERROR: dist dir does not exist"
    echo "*** ERROR:   $distdir"
    exit 1
fi

# We can catch some hard (but possible) to do mistakes by looking at
# our repo's revision, but only if we are in the source tree.
# Otherwise, use what configure told us, at the cost of allowing one
# or two corner cases in (but otherwise VPATH builds won't work).
repo_rev=$DSCHED_REPO_REV
if test -e .git ; then
    repo_rev=$(config/dsched_get_version.sh VERSION --repo-rev)
fi

#
# Update VERSION:repo_rev with the best value we have.
#
perl -pi -e 's/^repo_rev=.*/repo_rev='$repo_rev'/' -- "${distdir}/VERSION"
# need to reset the timestamp to not annoy AM dependencies
touch -r "${srcdir}/VERSION" "${distdir}/VERSION"

echo "*** Updated VERSION file with repo rev: $repo_rev"
echo "*** (via dist-hook / config/distscript.sh)"

#
# Update dsched.spec:%{version} with the main version
#
DSCHED_SPEC=contrib/dsched.spec
perl -pi -e 's/^Version:.*/Version: '$DSCHED_REPO_REV'/' -- "${distdir}/$DSCHED_SPEC"
touch -r "${srcdir}/$DSCHED_VERSION" "${distdir}/$DSCHED_VERSION"

echo "*** Updated $DSCHED_SPEC file with repo rev: $DSCHED_REPO_REV"
echo "*** (via dist-hook / config/distscript.sh)"
