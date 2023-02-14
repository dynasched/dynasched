
DynaSched Security Policy
========================

DynaSched is expected to operate at a privileged level in relatively controlled environments (i.e., the typical high-performance computing cluster embedded in a protected network, and thus not exposed to the general Internet)
and interacting solely with a privileged runtime environment (RTE). This situation is, however, is not globally true and the scheduler may find itself in situations where its communications are between entities at different privilege levels (e.g., between root and user) and operating on head nodes with direct connection to more exposed networks.

The DynaSched community takes security associated with use of its scheduler seriously, recognizing that our ability to respond to concerns is bound by our limited access to volunteer resources. We deeply appreciate coordinated efforts done in partnership with security reporters as these have the highest probability for a successful and satisfactory resolution. Reports that simply state something is wrong while providing no assistance in triaging the problem or developing the solution will be treated seriously, but with correspondingly longer response times.

DynaSched does not have formal, contractual relationships with its users. Instead, we have informal relationships with downstream packagers (e.g., Debian, Fedora, SUSE, Spack) and system administrators. This quite frequently takes the form of individuals as opposed to organizational contacts. Thus, it isn't possible for the DynaSched community to offer any guarantees as to the breadth or immediacy of notification for security issues. We can only do our best to alert the people we know about, and hope that they spread the word as required.

Our vulnerability disclosure process reflects this situation. While we strongly encourage discoverers to report potential security issues to us and follow our process, we will respect their wishes and work with them in cases where their preferred process may vary.


NOTE: any potential security issue should be reported immediately to us at security@pmix.org (a partner organization)

Vulnerability Disclosure Process
--------------------------------

This process covers security issues pertaining to the DynaSched scheduler itself. Issues with closely related components — such as PMIx or third-party plugins — may be raised with us and we will work with the reporter to identify the appropriate contacts and coordinate disclosure.

The DynaSched Vulnerability Disclosure Process consists of several distinct, but possibly overlapping steps:

1. Problem identification and initial triage.
    The problem is reported to the community and a first-level triage performed. Primary focus here is on scoping the extent of the issue so it can properly be communicated (both in terms of severity and complexity to address) to the rest of the community.

2. Notification and embargo
    Once the problem has been triaged, it will be communicated to the known consumers of the DynaSched scheduler (as noted above). This will be done in a private manner and all informed will be asked to maintain that privacy during the embargo period. The purpose of this step is to ensure the timely notification of the problem without alerting potential bad players as to the vulnerability, thus giving the community time to respond to the problem. The community will use this period to assemble a team to address the problem.

    The embargo period is expected to continue during the entire time work is being performed towards a resolution of the problem. The DynaSched community, of course, has no control over the actions of its members or users, and therefore cannot guarantee confidentiality throughout the resolution process. However, the community will request best efforts from all involved due to the shared interest.

3. Private release
    Once a solution to the problem has been devised, a new release of the scheduler will be made that includes the fix. This will first be made available to the known users of the scheduler, as well as the initial reporter if not a member of that list. A reasonable amount of time will be provided (as defined by general discussion across the involved parties) for rollout to occur through the participants. The embargo on public discussion will be maintained throughout this step.

4. General release
    At this point, the embargo will be lifted and a general announcement (including email to any DynaSched mailing lists) of the problem and availability of the solution will be made. All users will be urged to update as quickly as possible. All vulnerable releases will be removed from the GitHub release pages, though the tags corresponding to those releases will be retained for historical purposes.


Software Authenticity and Integrity
-----------------------------------
Authenticity and integrity of DynaSched software should always be confirmed by computing the checksum of the archive and comparing it with the value listed on the GitHub release page. Assuming you downloaded the file dynasched-1.0.0.tar.bz2, you can run the ``sha1sum`` command like this:

.. code-block:: sh

    shell$ sha1sum dynasched-1.0.0.tar.bz2

Check that the output matches what is printed in the release announcement, which may look like this:

.. code-block:: sh

    b4e1cb79dfd94c1b9db8eaba02f725c07ef9df2b  dynasched-1.0.0.tar.bz2

To avoid having to manually compare the string, you may copy/paste the posted sha1sum line and use the ``sha1sum -c parameter`` as follows:

.. code-block:: sh

    echo 'b4e1cb79dfd94c1b9db8eaba02f725c07ef9df2b  dynasched-1.0.0.tar.bz2'|sha1sum -c


