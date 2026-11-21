# DynaSched Security Policy

DynaSched runs as a scheduler daemon (`dsched`) that is also a PMIx server: it
accepts connections from PMIx tools and from the system controller, and acts on
the allocation and session-control requests they send. A scheduler is a shared
service by nature, so `dsched` can find itself serving requests from entities
at different privilege levels, and the node it runs on may be reachable from
more than one network.

The DynaSched community takes the security of the scheduler seriously,
recognizing that our ability to respond to concerns is bound by our limited
access to volunteer resources. We deeply appreciate coordinated efforts done in
partnership with our reporters, as these have the highest probability of a
successful and satisfactory resolution. Reports that simply state something is
wrong while providing no assistance in triaging the problem or developing the
solution will be treated seriously, but with correspondingly longer response
times.

DynaSched does not have formal, contractual relationships with its users. Any
relationships with downstream packagers, resource managers and the libraries
DynaSched builds on are informal, and quite frequently take the form of
individual rather than organizational contacts. It is therefore not possible
for the DynaSched community to offer any guarantee as to the breadth or
immediacy of notification for security issues.

While we recognize that reporters may have a preferred process for dealing
with security-related issues, our limited available resources restrict us to
the somewhat informal process described below. We appreciate your
understanding.

## Reporting a security issue

**Report potential security issues privately, through GitHub's private
reporting form:**

> <https://github.com/dynasched/dynasched/security/advisories/new>

You can also reach the form from the repository's **Security** tab, under
*Advisories*; GitHub's help pages describe it under private reporting.

This opens a private conversation visible only to you and the DynaSched
maintainers. Please do **not** open a public GitHub issue, post to a mailing
list, or submit a public pull request for a suspected security issue before we
have had a chance to respond — a pull request is public the moment it exists,
including its diff and its commit messages.

A report is most useful when it contains:

* the output of `dsched-info --all` from the affected build, or, failing that,
  the DynaSched version, how it was configured, and the PMIx, hwloc, and
  libevent versions it was built against;
* the environment: operating system, the system controller or resource
  manager `dsched` was serving, and whether `dsched` was started by a user, by
  a system service, or as root;
* a minimal reproducer — the command lines, the requests sent to `dsched`, and
  any MCA parameters needed to demonstrate the problem;
* what access someone needs to reach the problem (an unprivileged local
  account, a different user on the same node, a peer on the network, and so
  on), and what it then allows them to do;
* any suggested fix, and how you would like to be credited — the commit
  that carries the fix is where that credit appears.

## What happens after you report

We handle security issues with the same small, volunteer-driven process we use
for every other defect, because that is what our resources support. In
particular, **DynaSched does not operate a formal notification, embargo, and
private-release procedure.** We are not able to maintain a vetted list of
embargoed consumers, coordinate a multi-party disclosure timetable, or produce
private pre-release tarballs, and we would rather say so plainly than promise
a process we cannot execute.

What we do instead:

1. **Triage in the private advisory thread.** We work with you to confirm the
   issue, understand its scope, and judge its severity. Expect questions; the
   conversation stays private throughout.
2. **Develop the fix on a private branch.** Work proceeds out of public view,
   so the defect is not advertised by its own repair. We may ask you to review
   or test a candidate patch.
3. **Ship the fix in the next release.** The fix merges to the supported
   branch and is included in the next release. Releases are infrequent and are
   driven primarily by accumulated bug fixes, so a fix may sit on the branch
   for some time before a release carries it. A sufficiently severe issue can
   prompt a release sooner than the normal rhythm, but we cannot commit in
   advance to a schedule.

If the reported behavior turns out not to be a security issue, we will say so
and explain why, and — where there is a real defect underneath it — move the
discussion to a public issue and fix it there like any other bug.

## Supported versions

DynaSched has not yet made a release. Until it does, only the development head
is supported.

| Branch | Status |
|--------|--------|
| `master` | Pre-release development — fixes land here |

Once releases begin, support will be limited to the newest release series, and
**fixes will not be backported to earlier series**: the remedy for a security
issue in an older series will be to upgrade.

## Scope

This policy covers DynaSched itself — the `dsched` and `dsched-info`
executables and the code in this repository.

Issues in closely related components belong to their own projects, and we will
happily help you find the right contacts and coordinate:

* **PMIx** (OpenPMIx): report it privately through that repository's own
  form, <https://github.com/openpmix/openpmix/security/advisories/new>, under
  the policy in
  [`openpmix/SECURITY.md`](https://github.com/openpmix/openpmix/blob/master/SECURITY.md).
  DynaSched consumes a great deal of PMIx internal machinery — including the
  code that authenticates every connection to `dsched` — so if you are unsure
  which side of that boundary a problem lies on, report it to either and say
  so; we will route it.
* **PRRTE**, when it is the system controller `dsched` serves: report it
  through <https://github.com/openpmix/prrte/security/advisories/new>, under
  the policy in
  [`prrte/SECURITY.md`](https://github.com/openpmix/prrte/blob/master/SECURITY.md).
* **libevent, hwloc, other resource managers**, and third-party MCA plugins:
  these are separate upstreams. Raise the issue with us if you cannot tell
  where it belongs, and we will work out where the fix has to be written.

A few behaviors are intentional and are not, by themselves, security issues:

* **`dsched` trusts its own user.** A process that can connect to `dsched` as
  the user `dsched` runs as can make allocation and session requests as that
  user. What *is* in scope is a way for some *other* user, or a party that has
  not authenticated, to do that — for instance through session directory
  permissions, a predictable rendezvous path, or a connection that skips
  authentication.
* **Running DynaSched as root.** `dsched` refuses to run as root unless
  explicitly overridden with `--allow-run-as-root` (or the
  `DSCHED_ALLOW_RUN_AS_ROOT` and `DSCHED_ALLOW_RUN_AS_ROOT_CONFIRM`
  environment variables, both set to `1`), and that override accepts the risk of running a
  daemon that serves other processes' requests with full privileges. A way to gain privileges that
  works *without* the override is very much in scope.

If you are unsure whether what you have found is in scope, report it. We would
much rather read a report that turns out to be benign than miss one that is
not.

## Software authenticity and integrity

Once releases are published, confirm the authenticity and integrity of a
DynaSched archive by computing its checksum and comparing it against the value
listed on the [GitHub release
page](https://github.com/dynasched/dynasched/releases). For an archive named
`dynasched-<version>.tar.bz2` and a SHA-256 checksum, for example:

```sh
shell$ sha256sum dynasched-<version>.tar.bz2
```

To avoid having to compare the string by eye, give `sha256sum -c` the value
from the release page:

```sh
shell$ echo '<checksum from the release page>  dynasched-<version>.tar.bz2' | sha256sum -c
```

Note that a checksum confirms only that you received the archive the release
page describes. It says nothing about a snapshot tarball built from an
arbitrary Git commit, and nothing about a tree you assembled yourself — for
those, verify the Git history you built from.
