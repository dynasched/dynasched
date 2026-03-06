GitHub, Git, and related topics
===============================

GitHub
------

DynaSched's Git repositories are `hosted at GitHub
<https://github.com/dynasched>`_.

#. First, you will need a Git client. We recommend getting the latest
   version available. If you do not have the command ``git`` in your
   path, you will likely need to download and install Git.
#. `dynasched <https://github.com/dynasched/dynasched/>`_ is the main DynaSched
   repository where most active development is done.  Git clone this
   repository.  Note that the use of the ``--recursive`` CLI option is
   necessary because DynaSched uses Git submodules::

      shell$ git clone --recursive https://github.com/dynasched/dynasched.git

Note that Git is natively capable of using many forms of web
proxies. If your network setup requires the user of a web proxy,
`consult the Git documentation for more details
<https://git-scm.com/>`_.

Git commits: open source / contributor's declaration
----------------------------------------------------

In order to remain open source, all new commits to the DynaSched
repository must include a ``Signed-off-by:`` line, indicating the
submitter's agreement to the :ref:`DynaSched Contributor's Declaration
<contributing-contributors-declaration-label>`.

.. tip:: You can use the ``-s`` option to ``git commit`` to
         automatically add the ``Signed-off-by:`` line to your commit
         message.

.. _git-github-branch-scheme-label:

Git branch scheme
-----------------

Generally, DynaSched has two types of branches in its Git repository:

#. ``master``:

   * All active development occurs on the ``master`` branch (new features,
     bug fixes, etc.).

#. Release branches of the form ``vMAJOR.MINOR.x`` (e.g., ``v4.0.x``,
   ``v4.1.x``, ``v5.0.x``).

   * The ``.x`` suffix indicates that this branch is used to create
     all releases in the DynaSched vMAJOR.MINOR series.
   * Periodically, the DynaSched community will make a new release
     branch, typically from ``master``.
   * A Git tag of the form ``vMAJOR.MINOR.RELEASE`` is used to
     indicate the specific commit on a release branch from where an
     official DynaSched release tarball was created (e.g., ``v4.1.0``,
     ``v4.1.1``, ``v4.1.2``, etc.).
