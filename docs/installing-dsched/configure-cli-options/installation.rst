.. _label-building-installation-cli-options:

Installation options
^^^^^^^^^^^^^^^^^^^^

The following are general installation command line options that can
be used with ``configure``:

* ``--prefix=DIR``:
  Install DynaSched into the base directory named ``DIR``.  Hence, DynaSched
  will place its executables in ``DIR/bin``, its header files in
  ``DIR/include``, its libraries in ``DIR/lib``, etc.

  .. note:: Also see the section on :ref:`installation location
            <building-dsched-installation-location-label>` for more
            information on the installation prefix.

* ``--disable-shared``: By default, DynaSched builds a
  shared library, and all components are included as part of those
  shared library. This switch disables this default; it is really
  only useful when used with ``--enable-static``.  Specifically, this
  option does *not* imply ``--enable-static``; enabling static
  libraries and disabling shared libraries are two independent
  options.

  .. tip::

     :ref:`See this section <label-install-packagers-dso-or-not>` for
     advice to packagers about this CLI option.

* ``--enable-static``:
  Build DynaSched as a static library, and statically link in
  all components.  Note that this option does *not* imply
  ``--disable-shared``; enabling static libraries and disabling shared
  libraries are two independent options.

  .. tip::

     :ref:`See this section <label-install-packagers-dso-or-not>` for
     advice to packagers about this CLI option.

* ``--enable-dlopen``: Enable DynaSched to load components as
  standalone Dynamic Shared Objects (DSOs) at run-time.  This option
  is enabled by default.

  The opposite of this option, ``--disable-dlopen``, causes the following:

  #. DynaSched will not attempt to open any DSOs at run-time.
  #. configure behaves as if the ``--enable-mca-static`` argument was set.
  #. configure will ignore the ``--enable-mca-dso`` argument.

  See the description of ``--enable-mca-static`` / ``--enable-mca-dso`` for
  more information.

  .. note:: This option does *not* change how DynaSched's libraries
            (``libdsched``, for example) will be built.  You can change
            whether DynaSched builds static or dynamic libraries via
            the ``--enable|disable-static`` and
            ``--enable|disable-shared`` arguments.

.. _building-dsched-cli-options-mca-dso-label:

* ``--enable-mca-dso[=LIST]`` and ``--enable-mca-static[=LIST]``
  These two options, along with ``--enable-mca-no-build``, govern the
  behavior of how DynaSched's frameworks and components are built.

  The ``--enable-mca-dso`` option specifies which frameworks and/or
  components are built as Dynamic Shared Objects (DSOs).
  Specifically, DSOs are built as "plugins" outside of the core DynaSched
  library, and are loaded by DynaSched at run time.

  The ``--enable-mca-static`` option specifies which frameworks and/or
  components are built as part of the core DynaSched library (i.e.,
  they are not built as DSOs, and therefore do not need to be
  separately discovered and opened at run time).

  Both options can be used one of two ways:

  #. ``--enable-mca-OPTION`` (with no value)
  #. ``--enable-mca-OPTION=LIST``

  ``--enable-mca-OPTION=no`` or ``--disable-mca-OPTION`` are both legal
  options, but have no impact on the selection logic described below.
  Only affirmative options change the selection process.

  ``LIST`` is a comma-delimited list of DynaSched frameworks and/or
  framework+component tuples.  Examples:

  * ``sched`` specifies the entire sched framework
  * ``sched-fifo`` specifies just the FIFO component in the sched framework
  * ``sched,metrics-energy`` specifies the entire sched framework and the energy
     component in the metrics framework

  DynaSched's ``configure`` script uses the values of these two options
  when evaluating each component to determine how it should be built
  by evaluating these conditions in order:

  #. If an individual component's build behavior has been specified
     via these two options, ``configure`` uses that behavior.
  #. Otherwise, if the component is in a framework whose build
     behavior has been specified via these two options, ``configure``
     uses that behavior.
  #. Otherwise, ``configure`` uses the global default build behavior.

  At each level of the selection process, if the component is
  specified to be built as both a static and dso component, the static
  option will win.

  .. note:: ``configure``'s global default
            is to build all components as DSOs.

  .. important:: If the ``--disable-dlopen`` option is specified, then
                 DynaSched will not be able to search for DSOs at run
                 time, and the value of the ``--enable-mca-dso``
                 option will be silently ignored.

  Some examples:

  #. Default to building all components as DSOs::

        shell$ ./configure

  #. Build all components as static, except the FIFO SCHED, which will be
     built as a DSO::

        shell$ ./configure --enable-mca-static --enable-mca-dso=sched-fifo

  #. Build all components as static, except all SCHED components, which
     will be built as DSOs::

        shell$ ./configure --enable-mca-static --enable-mca-dso=sched

  #. Build all components as static, except all SCHED components and the
     ENERGY METRICS component, which will be built as DSOs::

        shell$ ./configure --enable-mca-static --enable-mca-dso=sched,metrics-energy

  #. Build all METRICs as static, except the ENERGY METRICS, as the
     ``<framework-component>`` option is more specific than the
     ``<framework>`` option::

        shell$ ./configure --enable-mca-static=metrics --enable-mca-dso=metrics-energy

  #. Build the ENERGY METRICS as static, because the static option at the
     same level always wins::

        shell$ ./configure --enable-mca-dso=metrics-energy --enable-mca-static=metrics-energy

  .. tip::

     :ref:`See this section <label-install-packagers-dso-or-not>` for
     advice to packagers about this CLI option.

* ``--enable-mca-no-build=LIST``: Comma-separated list of
  ``<framework>-<component>`` pairs that will not be built. For
  example, ``--enable-mca-no-build=metrics-energy,sched-fifo`` will
  disable building both the ``energy`` METRICS component and the
  ``fifo`` SCHED component.

  .. note:: This option is typically only useful for components that
            would otherwise be built - it is not necessary to
            specify to not build a component that the ``configure``
            script will naturally reject due to lack of support
            (e.g., lack of a dependent library)

* ``--disable-show-load-errors-by-default``:
  Set the default value of the ``mca_base_component_show_load_errors``
  MCA variable: the ``--enable`` form of this option sets the MCA
  variable to true, the ``--disable`` form sets the MCA variable to
  false.  The MCA ``mca_base_component_show_load_errors`` variable can
  still be overridden at run time via the usual MCA-variable-setting
  mechanisms; this configure option simply sets the default value.

  The ``--disable`` form of this option is intended for DynaSched
  packagers who tend to enable support for many different types of
  systems in their packages.  For example, consider a
  packager who includes support for both the FOO and BAR networks in
  their DynaSched package, both of which require support libraries
  (``libFOO.so`` and ``libBAR.so``).  If an end user only has BAR
  hardware, they likely only have ``libBAR.so`` available on their
  systems -- not ``libFOO.so``.  Disabling load errors by default will
  prevent the user from seeing potentially confusing warnings about
  the FOO components failing to load because ``libFOO.so`` is not
  available on their systems.

  Conversely, system administrators tend to build an DynaSched that is
  targeted at their specific environment, and contains few (if any)
  components that are not needed.  In such cases, they might want
  their users to be warned that the FOO components failed to
  load (e.g., if ``libFOO.so`` was mistakenly unavailable), because DynaSched
  may otherwise silently fail to provide support for that capability.

* ``--with-platform=FILE``:
  Load configure options for the build from ``FILE``.  Options on the
  command line that are not in ``FILE`` are also used.  Options on the
  command line and in ``FILE`` are replaced by what is in ``FILE``.
