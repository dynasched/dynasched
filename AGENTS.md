# AGENTS.md — Orientation for AI Coding Agents and Human Contributors

This document is an orientation guide for AI coding agents and human contributors working in the DynaSched codebase. It is a map, not comprehensive documentation; authoritative detail lives in `docs/` and in the header files under `src/`.

AI-assisted contributions are welcome. DynaSched targets HPC environments running on diverse hardware and operating systems. Write portable, standards-conforming C11 code and avoid shortcuts that work on one platform but silently break on another.

---

## What is DynaSched?

DynaSched (Dynamic Scheduler Platform) is an open-source, PMIx-native resource scheduler designed for HPC environments. It operates as a daemon (`dsched`) that accepts allocation requests from PMIx clients, computes schedules using pluggable algorithms, and returns allocation decisions — all built on top of the PMIx infrastructure.

Repository: https://github.com/dynasched/dynasched

---

## Terminology

- **MCA**: Modular Component Architecture — the plugin system inherited from the PMIx/PRRTE ecosystem
- **framework**: An MCA abstraction layer defining a functional area's interface (e.g., `dsched`, `dmetric`, `dmeta`)
- **component**: A plugin implementing a framework interface
- **module**: The active component instance after MCA selection
- **session**: A resource allocation session tracking nodes assigned to a user request (`dsched_session_t`)
- **meta**: A scheduling meta-request object (`dsched_meta_t`) carrying a request through the scheduling pipeline
- **caddy**: A heap-allocated callback object used to carry context across thread-shift boundaries (`dsched_shift_caddy_t`)
- **evbase**: An event base (`dsched_event_base_t`) wrapping a libevent `event_base` used for async dispatch
- **PMIX_NEW / PMIX_RELEASE**: PMIx object lifecycle macros (reference-counted allocation and release)

---

## User-Facing Tools

| Tool | Source | Purpose |
|------|--------|---------|
| `dsched` | `src/tools/dsched/` | The scheduler daemon — accepts and services allocation requests |
| `dsched_info` | `src/tools/dsched_info/` | Query configuration and list available components |

---

## Source Layout

```
src/
  tools/          # Executable entry points (dsched daemon, dsched_info)
  mca/            # MCA frameworks and their components
  runtime/        # Global state initialization, progress threads
  util/           # Internal utilities (error handling, attrs, session dirs)
  include/        # Internal headers (globals, constants, types, config)
  event/          # Libevent integration
```

Central data structures (`dsched_session_t`, `dsched_node_t`, `dsched_meta_t`, `dsched_shift_caddy_t`, `dsched_globals_t`) are defined in `src/include/dsched_globals.h`.

---

## MCA Frameworks

| Framework | Location | Responsibility |
|-----------|----------|----------------|
| `dsched` | `src/mca/dsched/` | Core scheduling algorithm (compute an allocation from a request) |
| `dmetric` | `src/mca/dmetric/` | Resource metric collection (e.g., energy) |
| `dmeta` | `src/mca/dmeta/` | Meta-scheduling — prioritize and order pending requests |
| `dlog` | `src/mca/dlog/` | Logging subsystem |
| `ddl` | `src/mca/ddl/` | Dynamic linker abstraction |
| `dinstalldirs` | `src/mca/dinstalldirs/` | Installation directory queries |

### Currently Available Components

| Framework | Component | Location | Notes |
|-----------|-----------|----------|-------|
| `dsched` | `fifo` | `src/mca/dsched/fifo/` | First-in, first-out scheduler |
| `dmetric` | `energy` | `src/mca/dmetric/energy/` | Energy-aware metric collection |
| `dmeta` | `pri` | `src/mca/dmeta/pri/` | Priority-based meta-scheduler |

### Component Structure

Each component requires:
- `<framework>.h` — the framework-level header defining the module struct
- `<component>.h` — component-local header exporting the component and module structs
- `<component>.c` — module implementation
- `<component>_component.c` — MCA registration, open, query, close
- `Makefile.am`

Selection follows standard MCA priority rules: the highest-priority component that opens successfully becomes the active module.

---

## DynaSched's Relationship with PMIx

DynaSched depends on PMIx (minimum version 6.1.0) and uses PMIx internals directly — this is distinct from calling the PMIx public API.

**From PMIx, DynaSched uses:**
- MCA infrastructure (`src/mca/base/pmix_base.h`, `src/mca/mca.h`)
- Data structures (`pmix_list_t`, `pmix_pointer_array_t`, `pmix_object_t`)
- Threading primitives (`pmix_mutex_t`, `pmix_lock_t`, `PMIX_WAKEUP_THREAD`)
- Utility functions (`pmix_argv_*`, `pmix_output_*`, `pmix_show_help`)
- Event loop integration (libevent via PMIx's event abstraction)
- Memory management (`PMIX_NEW`, `PMIX_RELEASE`)

---

## Coding Rules

### Mandatory Header Order

`dsched_config.h` **must be the first `#include`** in every `.c` file:

```c
#include "dsched_config.h"

#include <sys/types.h>
...
#include "src/include/dsched_globals.h"
```

### Symbol Prefixes

| Scope | Prefix |
|-------|--------|
| Exported macros | `DSCHED_` |
| Exported functions and types | `dsched_` |
| MCA component symbols | `dsched_<framework>_<component>_` |
| Internal (not exported) | `dsched_` without `DSCHED_EXPORT` |

Do not use `PMIX_` or `pmix_` prefixes for new DynaSched symbols.

### Copyright Header

New files require the standard multi-institution BSD copyright block with `$COPYRIGHT$` and `$HEADER$` tokens, matching the pattern in existing files.

### Macro Definitions

Define logical macros to `0` or `1`; never `#undef` them. Test with `#if FOO`, not `#ifdef FOO`.

### Constant-on-Left Comparisons

```c
if (NULL == ptr)              /* correct */
if (DSCHED_SUCCESS == rc)     /* correct */
```

### Bracing

Always use `{ }` around every conditional or loop body, including single-statement ones.

### Indentation

4 spaces, never tab characters.

### Compiler Warnings

Aim for zero compiler warnings. Use `DSCHED_HIDE_UNUSED_PARAMS(...)` for intentionally unused parameters rather than casting to void or suppressing warnings with flags.

### Generated Files

Do not hand-edit files produced by autotools or files vendored from PMIx. Edit the upstream source (`.m4`, `.am`, `.ac`) instead.

### Error Handling

Functions that can fail return `int`. Check every return value. Use `DSCHED_SUCCESS` / `DSCHED_ERROR` return codes and the appropriate output/logging calls for unexpected errors.

### Thread Model and Event Loop

DynaSched is event-driven. The main progress thread runs a libevent loop. Do not block on the progress thread. Schedule deferred work using `DSCHED_THREADSHIFT`.

The `fifo` scheduler component demonstrates the pattern: work is dispatched to a dedicated per-component event base (`dsched_progress_thread_init`) and results are shifted back to the main event base (`dsched_globals.evbase`) via `DSCHED_THREADSHIFT`.

### Thread-Shifting with Caddies

A **caddy** (`dsched_shift_caddy_t`) carries request context across thread-shift boundaries. Pattern:

1. Allocate with `PMIX_NEW(dsched_shift_caddy_t)`
2. Assign the relevant fields (do not copy — just point)
3. Call `DSCHED_THREADSHIFT(cd, evbase, handler_fn)`
4. The target event base fires `handler_fn` on the correct thread

Required caddy fields:

| Field | Type | Purpose |
|-------|------|---------|
| `ev` | `dsched_event_t` | Libevent handle — must be named `ev` |
| `mt` | `dsched_meta_t *` | The meta-request being scheduled |
| `alloc` | `dsched_alloc_t *` | Allocation result being built |
| `evcbfunc` | `dsched_event_cbfunc_fn_t` | Callback when scheduling completes |

Never allocate caddies on the stack; they must outlive the creating stack frame.

### Memory Management

Use `PMIX_NEW` / `PMIX_RELEASE` for objects that embed `pmix_object_t`. Use `malloc`/`free` for plain C structs that do not need reference counting.

### C Standard

DynaSched targets C11. Fix compiler warnings at their source; do not add `-Wno-*` flags.

---

## Build System

DynaSched uses GNU Autotools. The `configure` script is not checked in; generate it with:

```bash
./autogen.pl
./configure [options]
make -j$(nproc)
make install
```

### Common Configure Options

| Option | Purpose |
|--------|---------|
| `--with-pmix=<path>` | Path to PMIx installation |
| `--with-hwloc=<path>` | Path to hwloc installation |
| `--with-libevent=<path>` | Path to libevent installation |
| `--enable-debug` | Build with debug symbols and assertions |
| `--enable-devel-check` | Treat compiler warnings as errors |

**Minimum dependency versions** (from `VERSION`):

| Dependency | Minimum |
|------------|---------|
| PMIx | 6.1.0 |
| hwloc | 2.1.0 |
| libevent | 2.0.21 |
| autoconf | 2.69.0 |
| automake | 1.13.4 |
| libtool | 2.4.2 |

---

## Contributing

### Commit Messages

Write prose commit messages, not bullet lists. The subject line should complete "If applied, this commit will …". The body must explain **why** the change is needed, not just what it does. Keep subject lines under 72 characters.

All commits require `Signed-off-by:` (DCO):
```bash
git commit -s
```

### Pull Requests

- Open PRs against the `master` branch
- One logical change per PR
- Describe the problem in the PR description, not just the solution
- Update `docs/` for any user-visible change

### Testing

Run the daemon directly to smoke-test changes:

```bash
./dsched &        # start the scheduler daemon
# submit a test allocation request via a PMIx client
kill %1           # shut down
```

### Reporting Bugs

File issues at https://github.com/dynasched/dynasched/issues. Include the output of `dsched_info --all` and a minimal reproduction case.
