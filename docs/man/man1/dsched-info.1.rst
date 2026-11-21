.. _man1-dsched-info:

dsched-info
===========

.. include_body

dsched-info |mdash| Display information about the DynaSched installation

SYNOPSIS
--------

``dsched-info [options]``


DESCRIPTION
-----------

``dsched-info`` provides detailed information about the DynaSched
installation. It can be useful for at least three common scenarios:

#. Checking local configuration and seeing how DynaSched was installed.

#. Submitting bug reports / help requests to the DynaSched community
   (see :doc:`Getting help </getting-help>`).

#. Seeing a list of installed DynaSched plugins and querying what MCA
   parameters they support.

OPTIONS
-------

``dsched-info`` accepts the following options:

* ``-h`` | ``--help <arg0>``: Show help message. If the optional
  argument is not provided, then a generalized help message similar
  to the information provided here is returned. If an argument is
  provided, then a more detailed help message for that specific
  command line option is returned.

* ``-v`` | ``--verbose``: Enable debug output.

* ``-V`` | ``--version``: Print version and exit.

* ``--dmca <key> <value>`` | ``--mca <key> <value>``: Set the DynaSched
  MCA parameter ``key`` to ``value`` for this run.

* ``--pmixmca <key> <value>``: Set the PMIx MCA parameter ``key`` to
  ``value`` for this run.

* ``-a`` | ``--all``: Show all configuration options and MCA
  parameters.

* ``--arch``: Show architecture on which DynaSched was compiled.

* ``-c`` | ``--config``: Show configuration options

* ``--hostname``: Show the hostname on which DynaSched was configured
  and built.

* ``--internal``: Show internal MCA parameters (not meant to be
  modified by users).

* ``--param <framework>[:<component>,<component>...]`` |
  ``--params ...``: Show MCA parameters. The framework may be the
  keyword ``all``. Components are named as a comma-delimited list
  after a colon; if none are given, the parameters of every component
  in the framework are reported.

* ``--path <type>``: Show paths that DynaSched was configured
  with. Accepts the following parameters: ``all``, ``prefix``, ``bindir``,
  ``libdir``, ``incdir``, ``mandir``, ``pkglibdir``, ``sysconfdir``.

* ``--show-version [<what> [<part>]]``: Show the version of DynaSched or
  of its components. ``what`` is ``dsched`` or ``all`` for DynaSched
  itself and every component, a framework name for every component in
  that framework, or ``framework:component`` for one component.
  ``part`` is one of ``full`` (the default), ``major``, ``minor``,
  ``release``, ``greek`` or ``repo``. With no argument, show
  DynaSched's version and every component's.

* ``-t`` | ``--type <type>``: Show the MCA parameters of the given
  value type (for example ``string`` or ``int``).

* ``--include-pmix``: Also show the PMIx build information and MCA
  parameters.

* ``--pretty-print``: When used in conjunction with other parameters, the output is
  displayed in "prettyprint" format (default)

* ``--parsable``: When used in conjunction with other parameters, the output is
  displayed in a machine-parsable format

* ``--parseable``: Synonym for ``--parsable``

* ``--color <hue>``: Control color coding: auto (default), never, always

* ``--show-failed``: Show the components that failed to load along with the reason why they failed

* ``--selected-only``: Show only variables from selected components.


EXIT STATUS
-----------

Returns 0 if successful, non-zero if an error is encountered

EXAMPLES
--------

Examples of using this command.

Show the default output of options and listing of installed
components in a human-readable / prettyprint format:

.. code-block::

   dsched-info

Show the default output of options and listing of installed components
in a machine-parsable format:

.. code-block::

   dsched-info --parsable

Show the MCA parameters of the ``fifo`` component of the ``dsched``
framework in a human-readable / prettyprint format:

.. code-block::

   dsched-info --param dsched:fifo

Show the "bindir" that DynaSched was configured with:

.. code-block::

   dsched-info --path bindir

Show the version of DynaSched version numbers in a prettyprint format:

.. code-block::

   dsched-info --version

Show *all* information about the DynaSched installation, including all
components that can be found, all the MCA parameters that they support,
versions of DynaSched and the components, etc.:

.. code-block::

   dsched-info --all

.. seealso::
   :ref:`dynasched(7) <man7-dynasched>`
