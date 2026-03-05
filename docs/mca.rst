.. _label-mca:

The Modular Component Architecture (MCA)
========================================

DynaSched is a highly-customizable system; it can be configured via
configuration files, command line parameters, and environment
variables.  The main functionality of DynaSched's configuration system
is through the Modular Component Architecture (MCA).

* This section describes the MCA itself and how to set MCA parameters at
  run time.
* Later sections in this documentation describe different parts of
  DynaSched's functionality, and the specific names and values of MCA
  parameters that can be used to affect DynaSched's behavior.

/////////////////////////////////////////////////////////////////////////

Terminology
-----------

The Modular Component Architecture (MCA) is the backbone for much of
DynaSched's functionality.  It is a series of *frameworks*,
*components*, and *modules* that are assembled at run-time to create
the DynaSched implementation.

MCA *parameters* (also known as MCA *variables*) are used to customize
DynaSched's behavior at run-time.

Each of these entities are described below.

Frameworks
^^^^^^^^^^

An MCA framework manages zero or more components at run-time and is
targeted at a specific task (e.g., providing a specific scheduler
algorithm).  Although each MCA framework supports only a single
type of component, it may support multiple components of that type.

Some of the more common frameworks that users may want or need to
customize include the following:

* ``sched``: DynaSched Scheduling Layer; users may want to select
  which schedulers are selected to be active
* ``metrics``: Metrics to be used by scheduler components during
  their computation, and for "scoring" the resulting proposed
  allocations

There are multiple frameworks within DynaSched; the exact set varies
between different versions of DynaSched.  You can use the
:ref:`dsched_info(1) <man1-dsched_info>` command to see the full list of
frameworks that are included in DynaSched |dsched_ver|.

Components
^^^^^^^^^^

An MCA component is an implementation of a framework's formal
interface.  It is a standalone collection of code that can be bundled
into a plugin that can be inserted into the DynaSched code base, either
at run-time and/or compile-time.

.. note:: Good synonyms for DynaSched's "component" concept are
          "plugin", or "add-on".

The exact set of components varies between different versions of DynaSched.
DynaSched's code base includes support for many components, but
not all of them may be present or available on your system.  You can
use the :ref:`dsched_info(1) <man1-dsched_info>` command to see what
components are included in DynaSched |dsched_ver| on your system.

Modules
^^^^^^^

An MCA module (frequently called a ``plugin``) is an instance of a
component.  While it is possible for a component to have multiple
active plugins, it would be a very rare component that did so.

Parameters (variables)
^^^^^^^^^^^^^^^^^^^^^^

MCA *parameters* (sometimes called MCA *variables*) are the basic unit
of run-time tuning for DynaSched.  They are simple "key = value" pairs
that are used extensively throughout DynaSched.  The general rules of
thumb that the developers use are:

#. Instead of using a constant for an important value, make it an MCA
   parameter so users can adjust it if necessary.
#. If a task can be implemented in multiple, user-discernible ways,
   implement as many as possible, and use an MCA parameter to
   choose between them at run-time.

For example, a scheduler plugin might be based on an objective function
that can take a variety of metrics into account. An MCA param could be
used to specify which metrics are to be used, thereby allowing a system
administrator or researcher to explore various combinations.

/////////////////////////////////////////////////////////////////////////

.. _label-running-setting-mca-param-values:

Setting MCA parameter values
----------------------------

MCA parameters may be set in several different ways.

.. admonition:: Rationale
   :class: tip

   Having multiple methods to set MCA parameters allows, for example,
   system administrators to fine-tune the DynaSched installation for
   their hardware / environment while allowing researchers to keep
   those settings while adding others to further customize their
   trial system.

   HPC environments tend to be unique.  Providing extensive run-time tuning
   capabilities through MCA parameters allows the customization of
   DynaSched to each system's particular needs.

The following are the different methods to set MCA parameters, listed
in priority order:

#. Command line
#. Environment variables
#. Configuration files


Command line
^^^^^^^^^^^^

Variables set on the command line when DynaSched is invoked are given the
highest priority - i.e., they will overwrite any value provided via the
environment or configuration file. Command line parameters are specified
using the ``--dmca`` option:

.. code-block:: sh

   shell$ dsched --dmca sched_fifo_metric energy

Note that DynaSched is based on the PMIx library, and therefore the PMIx
MCA parameters can also be set to configure that library's behavior. PMIx
parameters are specified with the ``--pmixmca`` option:

.. code-block:: sh

    shell$ dsched --pmixmca ptl_base_verbose 5


Environment variables
^^^^^^^^^^^^^^^^^^^^^

Environment variables are given the next highest priority.  Any environment variable
named ``DSCHED_MCA_<param_name>`` will be examined for use. Note that
misspelling of names will cause the value to be ignored - i.e.,
DynaSched does not report unrecognized MCA parameters, it simply ignores
them.

.. note:: Just like with command line values, setting environment
          variables to values with multiple words requires shell
          quoting, such as:

          .. code-block:: sh

             shell$ export DSCHED_MCA_param="value with multiple words"


Configuration files
^^^^^^^^^^^^^^^^^^^

Simple configuration text files can also be used to set MCA
parameter values.  Parameters are set one per line (comments are
permitted).  For example:

.. code-block:: ini

   # This is a comment
   # Set an MCA parameter
   mca_component_show_load_errors = 1

Note that quotes are *not* necessary for setting multi-word values
in MCA parameter files.  Indeed, if you use quotes in the MCA
parameter file, they will be used as part of the value itself.  For
example:

.. code-block:: ini

   # The following two values are different:
   param1 = value with multiple words
   param2 = "value with multiple words"

By default, two files are searched (in order):

#. ``$HOME/.dsched/mca-params.conf``: The user-supplied set of
   values has the higher precedence.
#. ``$prefix/etc/dsched-mca-params.conf``: The system-supplied set
   of values has a lower precedence.

More specifically, the MCA parameter ``mca_param_files`` specifies a
colon-delimited path of files to search for MCA parameters.  Files to
the left have lower precedence; files to the right are higher
precedence.

.. warning:: Setting DynaSched MCA parameters via configuration files
             entails editing (by default) the following files:

             ``$HOME/.dsched/mca-params.conf`` or
             ``$prefix/etc/dsched-mca-params.conf``

/////////////////////////////////////////////////////////////////////////

.. _label-running-selecting-framework-components:

Selecting which DynaSched components are used at run time
---------------------------------------------------------

Each MCA framework has a top-level MCA parameter that helps guide
which components are selected to be used at run-time.  Specifically,
every framework has an MCA parameter of the same name that can be used
to *include* or *exclude* components from a given run.

For example, the ``sched`` MCA parameter can used to control which scheduler
components are used.  It takes a comma-delimited list of component
names, and may be optionally prefixed with ``^``.  For example:

.. note:: The ``sched`` framework provides support
  for a range of scheduling algorithms. The various plugins can
  execute in parallel (e.g., to compare their results), or a single
  one can be selected for production purposes.

.. code-block:: sh

   # Tell DynaSched to include *only* the sched components listed here and
   # implicitly ignore all the rest:
   export SCHED_MCA_sched=fifo,lrhnn ...

   # Tell DynaSched to exclude the fifo and lrhnn sched components
   # and implicitly include all the rest
   export DSCHED_MCA_sched=^fifo,lrhnn ...

Note that ``^`` can *only* be the prefix of the *entire*
comma-delimited list because the inclusive and exclusive behavior are
mutually exclusive.  Specifically, since the exclusive behavior means
"use all components *except* these", it does not make sense to mix it
with the inclusive behavior of not specifying it (i.e., "use all of
these components").  Hence, something like this:

.. code-block:: sh

   export DSCHED_MCA_sched=fifo,^lrhnn ...

does not make sense |mdash| and will cause an error |mdash| because it
says "use only the ``fifo`` component" but
also "use all components except ``lrhnn``".  These two statements
clearly contradict each other.

/////////////////////////////////////////////////////////////////////////

Common MCA parameters
---------------------

DynaSched has a *large* number of MCA parameters available.  Users can
use the :ref:`dsched_info(1) <man1-dsched_info>` command to see *all*
available MCA parameters.

The vast majority of these MCA parameters, however, are not useful to
most users.  Although the full list of MCA parameters can be found in the output of
``dsched_info(1)``, the following list of commonly-used parameters is
presented here so that they can easily be found via internet searches:

* Individual framework names with the ``_base_verbose`` suffix
  appended (e.g., ``sched_base_verbose``, ``metrics_base_verbose``, etc.)
  can be used to set the general verbosity level of all the components
  in that framework.

  * This can be helpful when troubleshooting why certain components
    are or are not being selected at run time.

* ``mca_base_component_show_load_errors``: By default, DynaSched
  emits a warning message if it fails to open a DSO component at run
  time.  This typically happens when a shared library that the DSO
  requires is not available.

  .. admonition:: Rationale
     :class: tip

     In DynaSched, components default to building
     as DSOs (vs. being included in the parent library,
     ``libdsched.so``).  On misconfigured systems, sometimes libraries
     required by various components may not be present, thereby causing
     those components to fail to open at run time.

     Having DynaSched warn about such failures to load is useful
     because it alerts users to the misconfiguration. Note that
     load errors cannot be reported if DSOs have been disabled.

  This MCA parameter can take four general values:

  #. ``yes`` or a boolean "true" value (e.g., ``1``): DynaSched will
     emit a warning about every component DSO that fails to load.

  #. ``no`` or a boolean "false" value (e.g., ``0``): DynaSched will
     never emit warnings about component DSOs that fail to load.

  #. A comma-delimited list of frameworks and/or components: DynaSched
     will emit a warning about any dynamic component that fails to
     open and matches a token in the list. "Match" is defined as:

     * If a token in the list is only a framework name, then any
       component in that framework will match.
     * If a token in the list specifies both a framework name and a
       component name (in the form ``framework/component``), then
       only the specified component in the specified framework will
       match.

     For example, if the value of this MCA parameter is
     ``sched,metrics/energy``, then DynaSched will warn if any component in
     the sched framework or if the energy metrics component fails to load at run
     time.

  #. The value can also be a ``^`` character followed by a
     comma-delimited list of ``framework[/component]`` values: This
     is similar to the comma-delimited list of tokens, except it will
     only emit warnings about dynamic components that fail to load
     and do *not* match a token in the list.

     For example, if the value of this MCA parameter is
     ``^sched,metrics/energy``, then DynaSched will only warn about the
     failure to load DSOs that are neither in the sched
     framework nor are the energy metrics component.
