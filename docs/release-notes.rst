Release Notes
=============

The following abbreviated list of release notes applies to this code
base as of this writing (12 November 2015):

General notes
-------------

Systems that have been tested are:

  * Linux (various flavors/distros), 64 bit (x86)
  * OS X (12.0 and above), 64 bit (x86_64, and M-1/2)

Python versions that have been tested include:

  * v3.10 and above


PMIx Support
-------------

The minimum required version of PMIx is v5.0.0 as this is where the scheduler integration support was introduced. PMIx guarantees backward compatibility for APIs (i.e., no API will be modified or removed once released), including for its Python bindings. Thus, compatibility with PMIx versions above v5.0.0 hinges upon the PMIx guarantee.


PRRTE Support
-------------

The minimum required version of PRRTE is v4.0.0 as this is where the scheduler integration support was introduced. All interactions with PRRTE occur via PMIx, and thus there is no compatibility requirement other than inclusion of the scheduler integration support. All PRRTE versions above v4.0.0 should, therefore, be supported.
