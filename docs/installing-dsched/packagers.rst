Advice for packagers
====================

.. _label-install-packagers-dso-or-not:

Components ("plugins"): DSO or no?
----------------------------------

DynaSched contains a number of components (sometimes called
"plugins") to effect different types of functionality in DynaSched.  For
example, some components provide support for networks and
may link against specialized libraries to do so.

DynaSched |dsched_ver| has two ``configure``-time defaults regarding the
treatment of components that may be of interest to packagers:

#. DynaSched's libraries default to building as shared libraries
   (vs. static libraries).  For example, on Linux, DynaSched will
   default to building ``libdsched.so`` (vs. ``libdsched.a``).

   .. note:: See the descriptions of ``--disable-shared`` and
             ``--enable-static`` :ref:`in this section
             <label-building-installation-cli-options>` for more
             details about how to change this default.

#. DynaSched will default to including its components in its libraries
   (as opposed to being compiled as dynamic shared objects, or DSOs).
   For example, ``libdsched.so`` on Linux systems will contain the FIFO
   SCHED component, instead of the FIFO SCHED being compiled into
   ``mca_sched_fifo.so`` and dynamically opened at run time via
   ``dlopen(3)``.

   .. note:: See the descriptions of ``--enable-mca-dso`` and
             ``--enable-mca-static`` :ref:`in this section
             <label-building-installation-cli-options>` for more
             details about how to change this defaults.

A side effect of these two defaults is that all the components
included in the DynaSched libraries will bring their dependencies with
them.  For example (on Linux), if the XYZ SCHED component
requires ``libXYZ.so``, then these defaults mean that
``libdsched.so`` will depend on ``libXYZ.so``.  This dependency will
likely be telegraphed into the DynaSched binary package that includes
``libdsched.so``.

Conversely, if the XYZ SCHED component was built as a DSO, then |mdash|
assuming no other parts of DynaSched require ``libXYZ.so`` |mdash|
``libdsched.so`` would *not* be dependent on ``libXYZ.so``.  Instead, the
``mca_sched_xyz.so`` DSO would have the dependency upon ``libXYZ.so``.

Packagers can use these facts to potentially create multiple binary
DynaSched packages, each with different dependencies by, for example,
using ``--enable-mca-dso`` to selectively build some components as
DSOs and leave the others included in their respective DynaSched
libraries.

.. code:: sh

   # Build all the "sched" components as DSOs (all other
   # components will default to being built in their respective
   # libraries)
   shell$ ./configure --enable-mca-dso=sched ...

This allows packaging ``$libdir`` as part of the "main" DynaSched
binary package, but then packaging
``$libdir/dsched/mca_sched_*.so`` as sub-packages.  These
sub-packages may inherit dependencies on their own.
Users can always install the "main" DynaSched
binary package, and can install the additional "sched" DynaSched
binary sub-package if they so choose.
