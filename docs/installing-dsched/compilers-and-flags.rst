.. _install-configure-compilers-and-flags-label:

Specifying compilers and flags
==============================

Changing the compilers that DynaSched uses to build itself uses the
standard GNU Autoconf mechanism of setting special environment variables
either before invoking ``configure`` or on the ``configure`` command
line itself.

The following environment variables are recognized by ``configure``:

* ``CC``: C compiler to use
* ``CFLAGS``: Compile flags to pass to the C compiler
* ``CPPFLAGS``: Preprocessor flags to pass to the C compiler
* ``LDFLAGS``: Linker flags to pass to all compilers
* ``LIBS``: Libraries to pass to all compilers (it is rarely
  necessary for users to need to specify additional ``LIBS``)
* ``PKG_CONFIG``: Path to the ``pkg-config`` utility

.. note:: DynaSched |dsched_ver| itself does not contain any C++ code. However,
    you can use a C++ compiler to build DynaSched if you so choose. Note that
    individual plugins *may* choose to use C++ - in that case, they will not
    build unless a C++ compiler is available.

For example, to build with a specific instance of ``gcc``:

.. code-block:: sh

   shell$ ./configure \
       CC=/opt/gcc-a.b.c/bin/gcc CFLAGS=-m64 ...

.. note:: The DynaSched community generally suggests using the above
   command line form for setting different compilers (vs. setting
   environment variables and then invoking ``./configure``).  The
   above form will save all variables and values in the ``config.log``
   file, which makes post-mortem analysis easier if problems occur.

Note that if you intend to compile DynaSched with a ``make`` other than
the default one in your ``PATH``, then you must either set the ``$MAKE``
environment variable before invoking DynaSched's ``configure`` script, or
pass ``MAKE=your_make_prog`` to configure.  For example:

.. code-block:: sh

   shell$ ./configure MAKE=/path/to/my/make ...

This could be the case, for instance, if you have a shell alias for
``make``, or you always type ``gmake`` out of habit.  Failure to tell
``configure`` which non-default ``make`` you will use to compile DynaSched
can result in undefined behavior (meaning: don't do that).

Note that you may also want to ensure that the value of
``LD_LIBRARY_PATH`` is set appropriately (or not at all) for your build
(or whatever environment variable is relevant for your operating
system).  For example, some users have been tripped up by setting to
use a non-default C compiler via the ``CC`` environment variable,
but then failing to set ``LD_LIBRARY_PATH`` to include the directory
containing that non-default C compiler's support libraries.
This causes DynaSched's ``configure`` script to fail when it tries to
compile / link / run simple C programs.
