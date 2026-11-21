.. _man1-dsterm:

dsterm
======

.. include_body

dsterm |mdash| Stop a running DynaSched scheduler

SYNOPSIS
--------

``dsterm [options]``


DESCRIPTION
-----------

``dsterm`` stops a running ``dsched``. It connects to the scheduler as
a PMIx tool, asks it to terminate, and then waits until its connection
to the scheduler drops, which is how it knows the scheduler has
actually gone rather than merely heard the request.

On the request, ``dsched`` finalizes its PMIx server, removes its
session directory and any file it wrote for ``--report-pid``, and
exits with status 0. This is the orderly way to stop ``dsched``.

Unless ``--uri`` says otherwise, ``dsterm`` connects to the scheduler
that PMIx advertises on the local host.

Only root and the user ``dsched`` runs as may stop it. A request from
anyone else is refused, and ``dsched`` keeps running.


OPTIONS
-------

``dsterm`` accepts the following options:

* ``-h`` | ``--help <arg0>``: Show help message. If the optional
  argument is not provided, then a generalized help message similar
  to the information provided here is returned. If an argument is
  provided, then a more detailed help message for that specific
  command line option is returned.

* ``-V`` | ``--version``: Print version and exit.

* ``-v`` | ``--verbose``: Report each step as it happens.

* ``--pmixmca <key> <value>``: Set the PMIx MCA parameter ``key`` to
  ``value`` for this run.

* ``--uri <arg>``: Connect to the ``dsched`` at the given PMIx URI
  rather than to the scheduler advertised on this host. The argument
  is the URI itself, or ``file:<path>`` for a file that holds it, as
  written by ``dsched --report-uri <path>``.


EXIT STATUS
-----------

Returns 0 once ``dsched`` has terminated. Returns 1 if ``dsterm``
could not connect to ``dsched``, if ``dsched`` refused the request, or
if the command line was not valid.

Interrupting ``dsterm`` (``SIGINT``, ``SIGTERM`` or ``SIGHUP``) before
``dsched`` has gone also returns 1. The first signal drops the
connection to ``dsched`` and exits; should that hang, a second warns
that another will force termination, and a third forces it. Whether
``dsched`` stops then depends on whether the request had already
reached it.


EXAMPLES
--------

Stop the scheduler running on this host:

.. code-block::

   dsterm

Stop the scheduler whose URI was written to a file when it started:

.. code-block::

   dsched --report-uri /path/to/dsched.uri &
   dsterm --uri file:/path/to/dsched.uri

.. seealso::
   :ref:`dsched-info(1) <man1-dsched-info>`,
   :ref:`dynasched(7) <man7-dynasched>`
