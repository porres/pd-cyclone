pd-cyclone is a 'fork' of the 
https://git.puredata.info/cgit/svn2git/libraries/miXed.git/ migrated repository.
It is cleaned to contain only the cyclone functionality. Other parts of the
miXed library are either moved (pddp) or unmaintained (toxy, ViCious, riddle).

Within the cyclone file set, the transition to a new build system, 
started with 0.1-alpha57 is completed. The initial version at this github
repository will be 0.2beta1.

The new build system is pd-lib-builder based and only builds each object 
in a separate file. The old build configuration also compiled to the hammer 
and sickle library objects and included a cyclone meta-library. The original 
location of this repository is https://github.com/electrickery/pd-cyclone.

<strong>Goals</strong>

The original goal of cyclone was to create a collection of Max/MSP objects 
for PureData. This was in the 2000s area, Max/MSP version 4.6. Since then 
MAX evolved its architecture and file format to something that is 
incompatible with PureData. Compatibility is limited to a very old version 
of Max/MSP.

Since then an unknown number of patches is made with these objects. This 
leads to an additional goal of keeping the functionality backward compatible.
Only bug fixes and new objects will be added. All other objects should be 
placed in other libraries. The <a 
href="https://github.com/electrickery/pd-playground">pd-playground</a> is 
where I keep mine.

<strong>Compiling with pdlibbuilder</strong>

PdLibBuilder tries to find the Pd source directory at several common 
locations, but when this fails, yo have to specify the path yourself 
using the pdincludepath variable. Example:

<pre>make pdincludepath=~/pd-0.46-6/src/</pre>

<strong>Installing with pdlibbuilder</strong>

The default path for installing might not be the best, surely for 
testing. Use the pkglibdir variable for this. Example:

<pre>make install pkglibdir=~/pd-externals/</pre>
