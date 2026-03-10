/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2014-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2015      Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2015-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2021-2026 Nanook Consulting  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */

#include "dsched_config.h"
#include "dsched_constants.h"

#include <stdio.h>
#ifdef HAVE_PWD_H
#    include <pwd.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_PARAM_H
#    include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */
#ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>
#ifdef HAVE_DIRENT_H
#    include <dirent.h>
#endif /* HAVE_DIRENT_H */
#ifdef HAVE_PWD_H
#    include <pwd.h>
#endif /* HAVE_PWD_H */

#include "src/util/pmix_argv.h"
#include "src/util/pmix_environ.h"
#include "src/util/pmix_os_dirpath.h"
#include "src/util/pmix_os_path.h"
#include "src/util/pmix_show_help.h"

#include "src/include/dsched_globals.h"
#include "src/util/dsched_session_dir.h"

/*******************************
 * Local function Declarations
 *******************************/
static bool tmpdir_created = false;
static bool setup_base_complete = false;

/*
 * Construct the session directory and create it if necessary
 */
pmix_status_t dsched_session_dir_init(void)
{
    pmix_status_t rc = PMIX_SUCCESS;
    mode_t my_mode = S_IRWXU; /* I'm looking for full rights */

    // only do this once
    if (setup_base_complete) {
        return PMIX_SUCCESS;
    }
    setup_base_complete = true;

    /* make sure that we have tmpdir_base set
     * if we need it
     */
    if (NULL == dsched_globals.tmpdir) {
        dsched_globals.tmpdir = strdup(pmix_tmp_directory());
        if (NULL == dsched_globals.tmpdir) {
            rc = PMIX_ERR_OUT_OF_RESOURCE;
        }
    }

    /* BEFORE doing anything else, check to see if this prefix is
     * allowed by the system
     */
    if (NULL != dsched_globals.prohibited_session_dirs &&
        NULL != dsched_globals.tmpdir) {
        char **list;
        int i, len;
        /* break the string into tokens - it should be
         * separated by ','
         */
        list = PMIx_Argv_split(dsched_globals.prohibited_session_dirs, ',');
        len = PMIx_Argv_count(list);
        /* cycle through the list */
        for (i = 0; i < len; i++) {
            /* check if prefix matches */
            if (0 == strncmp(dsched_globals.tmpdir, list[i], strlen(list[i]))) {
                /* this is a prohibited location */
                pmix_show_help("help-dsched-runtime.txt", "dsched:session:dir:prohibited", true,
                               dsched_globals.tmpdir, dsched_globals.prohibited_session_dirs);
                PMIx_Argv_free(list);
                return PMIX_ERR_FATAL;
            }
        }
        PMIx_Argv_free(list); /* done with this */
    }

    /* attempt to create it */
    rc = pmix_os_dirpath_create(dsched_globals.tmpdir, my_mode);
    if (PMIX_SUCCESS != rc && PMIX_ERR_EXISTS != rc) {
        PMIX_ERROR_LOG(rc);
        return rc;
    }

    if (PMIX_ERR_EXISTS != rc) {
        tmpdir_created = true;
    }
    return PMIX_SUCCESS;
}

void dsched_session_dir_finalize(void)
{
    // if we didn't create it, don't destruct it
    if (!tmpdir_created || NULL == dsched_globals.tmpdir) {
        return;
    }

    pmix_os_dirpath_destroy(dsched_globals.tmpdir, true, NULL);
    rmdir(dsched_globals.tmpdir);
    free(dsched_globals.tmpdir);
    dsched_globals.tmpdir = NULL;
}
