.. _building-dsched-vpath-label:

VPATH builds
============

VPATH build are fully supported. For example:

.. code-block:: sh

   shell$ tar xf dsched-VERSION.tar.bz2
   shell$ cd dsched-VERSION
   shell$ mkdir build
   shell$ cd build
   shell$ ../configure [...options...]
   <... lots of output ...>
   shell$ make -j 4
