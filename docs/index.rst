DynaSched |dsched_ver|
======================

Historically, high-performance computing (HPC) systems have consisted of some number of compute nodes executing static collections of processes under the control of a workload manager (which itself is comprised of a scheduler and a runtime environment) to solve large-scale problems. Scheduling execution of the applications in such systems is often characterized by constraints on the available energy, communications bandwidth, and processing power associated with each node, but is also impacted by the lack of full advanced knowledge of the characteristics and arrival time of workloads that will be submitted to the scheduler. *Static* resource managers address these issues by

The challenge confronting high performance computing (HPC) systems is to efficiently manage computational, communication, power, and other resources in this dynamic, unpredictable environment while ensuring that adequate quality of service (according to some appropriate metric) is maintained. Meeting this challenge will require a resource manager that can rapidly reschedule tasks “on-the-fly” when computing resources are unexpectedly lost or become unavailable for an unpredictable period of time. The ability to recover partial results would be a desirable feature, but may prove too costly in light of its attendant demands on the communication subsystem.

One approach to solving this problem assumes full advance knowledge of all tasks that will be executed by the system during some specified period of operation. Such “static” (or “offline”) resource managers schedule all the tasks and communications prior to any actual execution taking place. Thus, static resource managers can take advantage of this knowledge to, for example, schedule data transfers from one task to another in anticipation of the second task becoming ready for execution. In addition, static resource managers have more time to compute task/machine assignments due to their offline operation.

In contrast, “dynamic” resource managers operate “online” and hence do not have access to the full range of information available to their static counterparts. Dynamic resource managers must schedule tasks and communication while execution of previously scheduled tasks is taking place, thus placing added emphasis on the execution speed of the heuristic itself. In addition, dynamic heuristics cannot take advantage of visibility into the future to pre-schedule actions such as communications.

This paper reports on one potential dynamic solution to the ad hoc grid challenge that is based on combining the “receding horizon” concept from the control community with Lagrangian-based resource management methods found in some modern manufacturing line scheduling systems. In this initial investigation, the behavior of several possible variations of the proposed solution were studied in a simplified situation as a first step towards developing an overall solution to the problem. The results of these tests were subsequently compared against the performance from a common static heuristic.


Table of contents
=================

.. toctree::
   :maxdepth: 4
   :numbered:

   quickstart
   history
   release-notes
   getting-help
   install
   versions
   developers/index
   contributing
   license
   security
   man/index
