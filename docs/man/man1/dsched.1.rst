.. _man1-dsched:

dsched
======

.. include_body

dsched |mdash| The DynaSched scheduler daemon

SYNOPSIS
--------

``dsched [options]``


DESCRIPTION
-----------

``dsched`` is the DynaSched scheduler. It runs as a daemon that is
also a PMIx server in the scheduler role. PMIx tools and the system
controller connect to it and send it allocation, query and
session-control requests, which it answers with its scheduling
components.

At startup ``dsched``:

#. refuses to run as root, unless told otherwise (see
   ``--allow-run-as-root``);
#. detaches into the background, if ``--daemonize`` is given;
#. starts its PMIx server. PMIx advertises the scheduler on the local
   host, so that tools such as :ref:`dsterm(1) <man1-dsterm>` can find
   it without being told where it is;
#. connects to the system controller: the one named by
   ``--controller``, or else one PMIx finds on the local host, if any
   is running;
#. prints a ready message on standard error, unless ``--no-ready-msg``
   is given or it has daemonized:

   .. code-block::

      Scheduler checking in as pid <pid> on host <host>

``dsched`` names itself ``dsched-<host>-<pid>`` as a PMIx namespace,
with rank 0, unless the ``PMIX_SERVER_NSPACE`` and
``PMIX_SERVER_RANK`` environment variables say otherwise.

Only tools that name themselves are accepted, and only tools whose
user and group PMIx has established. PMIx accepts a tool only from
the user ``dsched`` runs as, or from root.


OPTIONS
-------

``dsched`` accepts the following options:

* ``-h`` | ``--help <arg0>``: Show help message. If the optional
  argument is not provided, then a generalized help message similar
  to the information provided here is returned. If an argument is
  provided, then a more detailed help message for that specific
  command line option is returned.

* ``-V`` | ``--version``: Print version and exit.

* ``-v`` | ``--verbose``: Show verbose output (verbosity 5), unless a
  level is set by ``--debug`` or the ``dsched_verbose`` MCA
  parameter.

* ``--debug``: Show debug output (verbosity 10), unless a level is set
  by the ``dsched_verbose`` MCA parameter, and report the exit status
  as ``dsched`` exits.

* ``--dmca <key> <value>`` | ``--mca <key> <value>``: Set the DynaSched
  MCA parameter ``key`` to ``value``.

* ``--pmixmca <key> <value>``: Set the PMIx MCA parameter ``key`` to
  ``value``.

* ``--daemonize``: Detach into the background. The command that
  started ``dsched`` returns 0 once the scheduler is ready, or the
  scheduler's status if it fails to start. The daemon's standard input,
  output and error are connected to ``/dev/null``.

* ``--set-sid``: Separate from the current terminal session. (A
  daemonized ``dsched`` always does.)

* ``--no-ready-msg``: Do not print the ready message.

* ``--tmpdir <dir>``: Use ``<dir>`` as the root of ``dsched``'s
  session directory tree, in which the PMIx server also makes its
  socket. This is shorthand for the ``dsched_tmpdir`` MCA parameter.
  Note that a socket's path is limited to about 100 characters, so a
  deeply nested directory can keep the server from starting.

* ``--report-pid <arg>``: Report ``dsched``'s pid: on standard output
  (``-``), on standard error (``+``), to the pipe whose file descriptor
  is given (an integer), or to a file (anything else). A file is
  removed when ``dsched`` exits.

* ``--report-uri <arg>``: Report ``dsched``'s PMIx URI, in the same
  forms as ``--report-pid``. A tool can be pointed at that file, as in
  ``dsterm --uri file:<path>``.

* ``--keepalive <fd>``: Monitor the pipe whose file descriptor is
  given, and terminate when it closes. While the pipe is open,
  ``dsched`` ignores the first termination signal (see `STOPPING
  DSCHED`_).

* ``--controller <arg>``: Connect to the system controller named by
  ``<arg>`` instead of looking for one. A controller named this way is
  required: if it cannot be reached, ``dsched`` does not start.
  ``<arg>`` is one of:

  * ``pid:<pid>`` or ``pid:file:<path>``: the controller's pid, given
    directly or in a file;
  * ``file:<path>``: a PMIx rendezvous file the controller wrote;
  * ``uri:<uri>`` or ``uri:file:<path>``: the controller's PMIx URI,
    given directly or in a file;
  * ``ns:<namespace>``: the controller's PMIx namespace;
  * ``system``: only the system-level server;
  * ``system-first``: the system-level server if there is one, else any
    server ``dsched`` may use.

* ``--allow-run-as-root``: Allow ``dsched`` to run as root. This is
  **strongly discouraged**: a mistake, such as a wrong ``TMPDIR``, can
  damage the system it runs on.


STOPPING DSCHED
---------------

The orderly way to stop ``dsched`` is :ref:`dsterm(1) <man1-dsterm>`.
It asks ``dsched`` to terminate; ``dsched`` finalizes its PMIx server,
removes its session directory and any ``--report-pid`` file, and exits
with status 0. Only root and the user ``dsched`` runs as may ask.

``dsched`` also stops on ``SIGTERM``, ``SIGINT`` or ``SIGHUP``. That is
its abnormal way out, and it exits with status 1. The signals count up
in a fixed sequence:

#. **The first signal** starts an orderly abort: ``dsched`` finalizes
   its PMIx server, removes its session directory and any
   ``--report-pid`` file, and exits.

#. **A second signal**, if the abort has not finished, prints a warning
   that the abort is in progress and that another signal will force
   termination.

#. **A third signal** forces termination: ``dsched`` makes one last
   attempt to finalize its server and remove its files, and exits
   immediately.

A ``dsched`` started with ``--keepalive <fd>`` lives as long as that
pipe stays open, so it ignores the first signal. A second still prints
the warning, and a third still forces termination. Closing the pipe
makes ``dsched`` abort as for a first signal.


EXIT STATUS
-----------

Returns 0 when stopped by :ref:`dsterm(1) <man1-dsterm>`. Returns 1
when stopped by a signal or by the closing of its ``--keepalive``
pipe, and when it fails to start - including when the command line is
not valid, when the controller named by ``--controller`` cannot be
reached, and when it is run as root without ``--allow-run-as-root``.


ENVIRONMENT
-----------

``DSCHED_ALLOW_RUN_AS_ROOT``, ``DSCHED_ALLOW_RUN_AS_ROOT_CONFIRM``
   Both set to ``1``, allow ``dsched`` to run as root, as
   ``--allow-run-as-root`` does.

``PMIX_SERVER_NSPACE``, ``PMIX_SERVER_RANK``
   The PMIx namespace and rank ``dsched`` takes, in place of
   ``dsched-<host>-<pid>`` and 0.

``DSCHED_MCA_<param>``
   Set the DynaSched MCA parameter ``<param>``. A value given on the
   command line takes precedence.


EXAMPLES
--------

Start the scheduler in the foreground, and stop it from another
terminal:

.. code-block::

   dsched
   dsterm

Start the scheduler in the background, recording its pid and URI:

.. code-block::

   dsched --daemonize --report-pid /path/to/dsched.pid --report-uri /path/to/dsched.uri
   dsterm --uri file:/path/to/dsched.uri

Start the scheduler attached to the system controller whose URI is in
a file:

.. code-block::

   dsched --controller uri:file:/path/to/controller.uri

.. seealso::
   :ref:`dsterm(1) <man1-dsterm>`,
   :ref:`dsched-info(1) <man1-dsched-info>`,
   :ref:`dynasched(7) <man7-dynasched>`
