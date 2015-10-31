pd-cyclone is a 'fork' of the 
https://git.puredata.info/cgit/svn2git/libraries/miXed.git/ migrated repository.
It is cleaned to contain only the cyclone functionality. Other parts of the
miXed library are either moved (pddp) or unmaintained (toxy, ViCious, riddle).

Within the cyclone file set, the transition to a new build system, 
started with 0.1-alpha57 is completed. 

The new build system is pd-lib-builder based and builds each object in a 
separate file. The old build configuration also compiled to the hammer and sickle library objects. 

This branch is created to test and evaluate a threaded version of the 
hammer/coll object as provided by Ivica Ico Bukvic from the Pd-l2ork fork.

Compiling with pdlibbuilder

PdLibBuilder tries to find the Pd source directory at several common locations, 
but when this fails, yo have to specify the path yourself using the pdincludepath
variable. Example:

make pdincludepath=~/pd-0.46-6/src/

Installing with pdlibbuilder

The default path for installing might not be the best, surely for testing. 
Use the pkglibdir variable for this. Example:

make install pkglibdir=~/pd-externals/
