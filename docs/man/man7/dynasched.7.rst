.. _man7-dynasched:

dynasched
=========

.. include_body

dynasched |mdash| The DynaSched dynamic scheduler


DESCRIPTION
-----------

DynaSched is a resource scheduler for HPC systems, built on PMIx. It
has no protocol of its own: the scheduler is a PMIx server in the
scheduler role, and whatever asks it for resources does so through the
standard PMIx APIs, as a PMIx tool. A resource manager, a launcher, a
workflow engine or a user's own program can make an allocation request
without knowing anything about DynaSched beyond where to find it.

DynaSched is written for *dynamic* scheduling: requests can arrive at
any time, from running jobs as well as from users, and the scheduler
is built to revise its schedule as they do. Its scheduling logic is in
plugins, so the algorithm, the ordering of pending requests and the
metrics used to judge an allocation can each be replaced without
touching the rest.

.. important::

   DynaSched is under development, and this release does not yet
   schedule real resources. See `CURRENT STATUS`_ before relying on
   any of the behavior described here.


THE PROGRAMS
------------

:ref:`dsched(1) <man1-dsched>`
   The scheduler daemon. Everything else on this page is about what
   ``dsched`` does.

:ref:`dsterm(1) <man1-dsterm>`
   Stops a running ``dsched``, in an orderly way.

:ref:`dsched-info(1) <man1-dsched-info>`
   Reports how DynaSched was built and installed, which plugins it
   has, and the MCA parameters they take.


HOW THE PIECES FIT TOGETHER
---------------------------

A system running DynaSched has three kinds of PMIx process:

* **The scheduler**, ``dsched``, which decides which resources each
  request gets.

* **The system controller**, which owns and runs the resources: it
  starts jobs on the nodes the scheduler allocates, and tells the
  scheduler about the system and what happens on it. A PRRTE DVM run
  as the system controller is one. There is at most one per system.

* **Tools**: any other process that talks to the scheduler, from a
  command such as :ref:`dsterm(1) <man1-dsterm>` to a running job that
  wants more resources.

When ``dsched`` starts, it connects to the system controller, if one is
running: the one named by its ``--controller`` option, or else one PMIx
finds on the local host. A system controller started after ``dsched``
connects to it instead. Either way, ``dsched`` and the controller each
know the other once both are up.

PMIx advertises ``dsched`` on its host, so tools need not be told where
it is. PMIx writes the rendezvous files it uses for a scheduler: one
that a tool asking for *the* scheduler (``PMIX_CONNECT_TO_SCHEDULER``)
finds, and one each by ``dsched``'s pid and by its namespace, for a tool
pointed at a particular scheduler with ``PMIX_SERVER_PIDINFO`` or
``PMIX_SERVER_NSPACE``. A ``dsched`` run with ``--report-uri`` also
writes its PMIx URI where it is told, for a tool pointed at it with
``PMIX_SERVER_URI``.

``dsched`` takes the PMIx namespace ``dsched-<host>-<pid>``, with rank 0,
unless its environment names another (see :ref:`dsched(1)
<man1-dsched>`).


REQUESTS
--------

``dsched`` answers four kinds of PMIx request.

Allocation requests
^^^^^^^^^^^^^^^^^^^

A tool asks for resources with ``PMIx_Allocation_request()``.
``dsched`` reads these attributes of the request:

* how much is wanted: ``PMIX_ALLOC_NUM_NODES``,
  ``PMIX_ALLOC_NUM_CPUS``, ``PMIX_ALLOC_NUM_CPU_LIST``,
  ``PMIX_ALLOC_MEM_SIZE``;

* which resources may be used: ``PMIX_ALLOC_NODE_LIST``,
  ``PMIX_ALLOC_EXCLUDE``, ``PMIX_ALLOC_CPU_LIST``,
  ``PMIX_ALLOC_QUEUE``, ``PMIX_ALLOC_IMAGE``;

* when, and for how long: ``PMIX_ALLOC_TIME``, ``PMIX_ALLOC_BEGIN``,
  ``PMIX_ALLOC_DEPENDENCY``, ``PMIX_ALLOC_LEND``;

* how the allocation may be used: ``PMIX_ALLOC_PREEMPTIBLE``,
  ``PMIX_ALLOC_SHARE``, ``PMIX_ALLOC_NOSHELL``,
  ``PMIX_ALLOC_WAIT_ALL_NODES``;

* which allocation and session the request concerns:
  ``PMIX_ALLOC_REQ_ID``, ``PMIX_ALLOC_ID``, ``PMIX_SESSION_ID``.

A string or numeric attribute whose value is not of the type PMIx
defines for it is an error: the request is refused with
``PMIX_ERR_BAD_PARAM``.

A request is normally answered when it has been scheduled, so the
requestor waits for its allocation. A request that carries
``PMIX_ALLOC_NOT_WAITING`` is answered ``PMIX_SUCCESS`` as soon as it
has been accepted, and is scheduled afterwards.

Queries
^^^^^^^

A tool asks about the scheduler's resources with
``PMIx_Query_info()``. ``dsched`` answers these keys:

* ``PMIX_QUERY_ALLOCATION``: the nodes ``dsched`` holds, each as a
  ``PMIX_NODE_INFO`` array of its name, any aliases and the index of
  its topology, followed by each topology as hwloc XML.

* ``PMIX_HWLOC_XML_V1`` and ``PMIX_HWLOC_XML_V2``: one node's topology,
  as hwloc XML in that version's format. The query must name the node
  with a ``PMIX_HOSTNAME`` or ``PMIX_NODEID`` qualifier. A node
  ``dsched`` does not know, or one whose topology it does not hold, is
  an error (``PMIX_ERR_BAD_PARAM``).

A query that answers every key it names returns ``PMIX_SUCCESS``. One
that answers some of them returns ``PMIX_ERR_PARTIAL_SUCCESS``, with
the results it found, and one that answers none returns
``PMIX_ERR_NOT_FOUND``.

Job control
^^^^^^^^^^^

``dsched`` runs no jobs, so the only job control it acts on is
``PMIX_JOB_CTRL_TERMINATE`` with no targets, which stops ``dsched``
itself. It is what :ref:`dsterm(1) <man1-dsterm>` sends. Only root and
the user ``dsched`` runs as may send it: anyone else is refused with
``PMIX_ERR_NO_PERMISSIONS``. Any other job control is refused with
``PMIX_ERR_NOT_SUPPORTED``.

Session control
^^^^^^^^^^^^^^^

A session-control request (``PMIx_Session_control()``) is refused with
``PMIX_ERR_NOT_SUPPORTED``: none is implemented yet.


HOW A REQUEST IS SCHEDULED
--------------------------

``dsched`` passes each allocation request through three layers of
plugins, each an MCA framework:

#. a **meta-scheduler** (framework ``dmeta``) takes the request and
   hands it to the schedulers;

#. each **scheduler** (framework ``dsched``) proposes an allocation for
   it, from the resources it knows to be free, using **metrics**
   (framework ``dmetric``) to judge between the allocations it could
   make;

#. the meta-scheduler chooses one of the proposals, and ``dsched``
   answers the request with it.

Several schedulers can be active at once, each proposing its own
allocation for every request, so that their results can be compared
while one of them makes the decision.

DynaSched includes these frameworks and plugins:

``dmeta``: meta-schedulers
   ``pri``: chooses the proposal of the highest-priority scheduler.

``dsched``: schedulers
   ``fifo``: schedules requests in the order they arrive, giving each
   the first nodes ``dsched`` holds, if enough are available.

``dmetric``: metrics
   ``energy``: a metric based on energy use.

``dlog``: logging
   ``stdfd``: logging to standard output and standard error.

``dinstalldirs``: installation directories
   ``env`` and ``config``: where DynaSched finds its own files. This
   framework is internal, and is not a choice to make.

:ref:`dsched-info(1) <man1-dsched-info>` lists the plugins an
installation has. Plugins are selected by priority, and the
framework's own MCA parameter, named for it (``dsched``, ``dmeta``, and
so on), restricts which may be used; see :ref:`label-mca`.


CONFIGURATION
-------------

DynaSched is configured through MCA parameters, which can be set on
the ``dsched`` command line (``--dmca <param> <value>``), in the
environment (``DSCHED_MCA_<param>``) or in parameter files. A value
given on the command line takes precedence over one in the
environment, which takes precedence over one in a file. PMIx's own MCA
parameters are set with ``--pmixmca`` or ``PMIX_MCA_<param>`` instead.
:ref:`label-mca` describes all of this in full, and
:ref:`dsched-info(1) <man1-dsched-info>` lists every parameter with its
current value.

These are the parameters most often wanted:

``dsched_tmpdir``
   The root of ``dsched``'s session directory tree, in which PMIx also
   makes its socket. ``dsched --tmpdir`` sets it.

``dsched_verbose``
   The verbosity of ``dsched``'s own output. ``dsched -v`` sets 5 and
   ``dsched --debug`` 10, unless this parameter is set.

``<framework>_base_verbose``
   The verbosity of one framework's output - for example,
   ``dsched_base_verbose 5`` shows which schedulers are selected.

``dsched_fifo_priority``
   The priority of the ``fifo`` scheduler, which orders it among the
   active schedulers. Each plugin has a ``<framework>_<plugin>_priority``
   parameter like it.

``dsched_progress_thread_cpus``, ``dsched_bind_progress_thread_reqd``
   The CPUs to bind ``dsched``'s progress threads to, as a
   comma-delimited list of ranges, and whether that binding is
   required.

``dsched_keep_fqdn_hostnames``
   Keep fully qualified host names, rather than reducing them to their
   first component.

``dsched_param_files``
   The parameter files to read, as a comma-delimited list, in place of
   the defaults below. A file later in the list takes precedence over
   one earlier.

``dsched_override_param_file``
   One more parameter file, whose values take precedence over every
   other source, the command line included. ``dsched`` warns about each
   value it overrides, unless ``dsched_suppress_override_warning`` is
   true.

``dsched_param_files``, ``dsched_override_param_file`` and
``dsched_suppress_override_warning`` govern how the files are read, so
they are taken only from the command line or the environment.


CURRENT STATUS
--------------

DynaSched is under development. This release has the structure
described above - the daemon, its PMIx server, the request handling and
the plugin layers - but not yet the parts that make it a working
scheduler:

* ``dsched`` holds five placeholder nodes, ``foo00`` through ``foo04``,
  rather than the resources of the system it runs on, and has no
  topology for any of them. A ``PMIX_QUERY_ALLOCATION`` query reports
  those nodes.

* No allocation is made. An allocation request passes through the
  meta-scheduler and the schedulers, but none proposes an allocation,
  and the request is answered ``PMIX_ERR_NOT_SUPPORTED`` - or, if it
  carried ``PMIX_ALLOC_NOT_WAITING``, ``PMIX_SUCCESS`` when it was
  accepted, with nothing to follow.

* The ``energy`` metric and the ``stdfd`` logger do nothing yet.

* No session control is implemented.


FILES
-----

``$HOME/.dsched/mca-params.conf``
   The user's MCA parameter file. Its values take precedence over the
   system file's.

``$prefix/etc/dsched-mca-params.conf``
   The system's MCA parameter file.

``$prefix/lib/dsched/``
   DynaSched's plugins, when they are built as separate libraries.

``dsched_tmpdir``, or else the system's temporary directory
   ``dsched``'s session directory, in which PMIx makes the directory that
   holds its socket. ``dsched`` makes the directory if it does not exist,
   and then removes it when it exits; one that already existed is left
   in place.

``pmix-user.<uid>``, in the system's temporary directory
   PMIx's rendezvous files for ``dsched``: ``pmix.sched.<host>``, and
   ``pmix.<host>.tool.<pid>`` and ``pmix.<host>.tool.<namespace>``.
   They are removed when ``dsched`` exits, by ``dsterm`` or by a signal
   it can catch; the directory, which other PMIx servers of the same
   user share, stays.


.. seealso::
   :ref:`dsched(1) <man1-dsched>`,
   :ref:`dsterm(1) <man1-dsterm>`,
   :ref:`dsched-info(1) <man1-dsched-info>`,
   :ref:`The Modular Component Architecture (MCA) <label-mca>`
