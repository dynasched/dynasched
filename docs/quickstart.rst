.. _label-quickstart:

Quick start
===========

In many cases, DynaSched can be built and installed by simply
indicating the installation directory on the command line:

.. code-block:: sh

   $ $(PYTHON) setup.py install --quiet --user --prefix="<path>"

Note that there are many configuration options to ``setup.py``.
Some of them may be needed for your particular
environment; see ``setup.py --help`` for guidance.

If your installation prefix path is not writable by a regular user,
you may need to use ``sudo`` or ``su`` to run ``setup.py``. However,
this is typically discouraged as it can interfere with system-level
Python packages. A far better alternative in such situations is to
use Python's "virtualenv" to create an isolated environment for the
package that is unique to the user.

Establishing a virtual environment for DynaSched is easy to do and
documentation on the required steps is readily available on the
Internet. Very briefly, the steps consist of:

* using ``pip`` to install the ``virtualenv`` package:

.. code-block:: sh

   $ pip install virtualenv

Now that ``virtualenv`` is installed, you create a virtual environment
by simply typing:

.. code-block:: sh

   $ virtualenv -p python3 myenv
   ...lots of output

The ``-p python3`` is optional and can be used to load whichever Python
version you like into your new environment.

The new environment is "activated" by typing:

.. code-block:: sh

   $ source ./myenv/bin/activate

You can now safely install your own private copy of DynaSched from a
published version of the package using ``pip``:

.. code-block:: sh

   $ python -m pip install dynasched

Alternatively, if no published DynaSched package is available or you
want to work with the latest and greatest version (e.g., for development),
you can clone the GitHub repository:

.. code-block:: sh

   $ git clone https://github.com/dynasched/dynasched

to obtain the code. Prior to installing
the DynaSched modules, you should ensure that your virtual environment
includes all the required Python modules by running:

.. code-block:: sh

   $ python -m pip install < requirements.txt
   ...lots of output

DynaSched can then be locally installed
using the command line shown at the beginning of this article.

Be sure to add your ``<prefix>/lib/pythonXXX/site-packages/DynaSched-pyXXX.egg`` (where XXX is your Python version, e.g., "3.10") to your ``PYTHONPATH`` before
trying to execute DynaSched!

Exiting the virtual environment is accomplished by typing:

.. code-block:: sh

   $ deactivate
