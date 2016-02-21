pd-cyclone is a 'fork' of the https://git.puredata.info/cgit/svn2git/libraries/miXed.git/ migrated repository (forked from Fred Jan Kraan's github at https://github.com/electrickery/pd-cyclone). It only contains the cyclone functionality. Other parts of the miXed library are either moved (pddp) or unmaintained (toxy, ViCious, riddle).

The version of cyclone available in https://git.puredata.info/cgit/svn2git/libraries/miXed.git/ is 0.1-Alpha56, present in Pd Extended 0.43. A new "unreleased" version 0.1-Alpha57 that was maintained by Fred is found in <http://puredata.info/downloads/cyclone>. Fred was also working a new version 0.2beta1, which is now forked to this repositoty. The original location of this repository is https://github.com/porres/pd-cyclone.

Version 0.2beta1 is available for download via the deken plug in, another version will emerge from this repository with new objects, new functionalities, and bug fixes.

<strong>Goals</strong>

This repository is faithful to the original goal of cyclone in creating a collection of Max/MSP objects for PureData. The early version was in time of Max/MSP 4.6. Further development can include new objects and new functionalities in existing from the most current Max/MSP version (Max 7 nowadays). 

Project is open to collaboration. Ideally all possible objects could be cloned and kept updated, but this is a fair amount of work, so collaborators are free to decide and work on what they consider more relevant and important.

<strong>Compiling with pdlibbuilder</strong>

Original Project Description (from http://puredata.info/downloads/cyclone)

Cyclone is a library of PureData classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. In this respect, cyclone has much in common with Thomas Grill's flext, and flext-based externals. See Thomas' page. While flext enables developing new cross-platform classes, cyclone makes existing classes cross-platform. 

Cyclone also comes handy, somewhat, in the task of importing Max/MSP 4.x patches into Pd. Do not expect miracles, though, it is usually not an easy task. 
