#include "dsched_config.h"

#include <stdarg.h>

#include "src/include/dsched_globals.h"

dsched_globals_t dsched_globals = {
    .evbase = NULL,
    .evpri = 5,
    .initialized = false,
    .version_string = NULL,
    .basename = NULL,
    .show_help_data = NULL,
    .nodes = PMIX_POINTER_ARRAY_STATIC_INIT,
    .sessions = PMIX_POINTER_ARRAY_STATIC_INIT,
    .topologies = PMIX_POINTER_ARRAY_STATIC_INIT,
    .cache = PMIX_POINTER_ARRAY_STATIC_INIT,
    .param_files = NULL,
    .override_param_file = NULL,
    .suppress_override_warning = false,
    .clean_output = -1,
    .tmpdir = NULL
};

#if DSCHED_PICKY_COMPILERS
void dsched_hide_unused_params(int x, ...)
{
    va_list ap;
    (void)x;
    va_start(ap, x);
    va_end(ap);
}
#endif
