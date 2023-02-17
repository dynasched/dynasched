DynaSched |dsched_ver|
======================

A heterogeneous network of mobile computing devices, data collection sensors, communication channels, and multiple users can be used as an ad hoc computing grid to solve large-scale tasks. Such grids are characterized not only by constraints on the available energy and communications bandwidth associated with each device, but also by the dynamic nature of the grid itself. Assets connected to the grid (computing devices, sensors, and users) can – and frequently do – appear and disappear from the grid at unanticipated times. In addition, communication links are prone to spurious failures and occasional noise that significantly impacts the grid’s ability to transfer information between nodes. Further compounding the problem is the lack of full advanced knowledge of the characteristics and arrival time of tasks that will be submitted to the grid.

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
