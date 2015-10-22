pd-cyclone is a 'fork' of the 
https://git.puredata.info/cgit/svn2git/libraries/miXed.git/ migrated repository.
It is cleaned to contain only the cyclone functionality. Other parts of the
miXed library are either moved (pddp) or unmaintained (toxy, ViCious, riddle).

Within the cyclone file set, the transition to a new build system, 
started with 0.1-alpha57 is completed. The initial version at this github
repository will be 0.2beta1.

The new build system is pd-lib-builder based and only builds each object 
in a separate file. The old build configuration also compiled to the hammer 
and sickle library objects and included a cyclone meta-library. 
