
Cyclone 0.3 beta-2 (released june 19th 2017)
-------

Releases: https://github.com/porres/pd-cyclone/releases

Cyclone: A set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pure Data with objects cloned from cycling74's Max/MSP. It thus provides some good level of compatibility between the two environments, helping users of both systems in the development of equivalent patches. 

-------

Pure Data (or just "Pd") project is found at: https://sourceforge.net/p/pure-data/pure-data/ci/master/tree/
or in its github mirror https://github.com/pure-data/pure-data. The Pd webpage managed by the Pd community is at: http://puredata.info/

Max is found at: https://cycling74.com/

-------

<strong>About Cyclone 0.3:</strong>

Since cyclone 0.2 was still mostly outdated and compliant to Max 4.0 (from the early 2000s), the main focus of cyclone 0.3 (currently in beta stage) has been on updating cyclone objects to the latest Max version (Max 7.3.3 at the time of this release). We're also fixing many bugs, rewriting the documentation and including new objects. So far we have:

- 59 updated objects (only 4 reamining to be updated);
- 61 fixed objects (including objects that were and were not updated);
- 48 new objects;
- A newly written documentation, taking care of numerous issues (yet to include meta and related objects)

Check details in our changelog: https://github.com/porres/pd-cyclone/wiki/cyclone-0.3-changlelog 


-------

<strong>Installing Cyclone:</strong>

This release has been tested with Pd Vanilla 0.47-1, not guaranteed to work in any other version or in other flavors such as Pd Extended. Purr Data users should know that it contains the latest cyclone releases. 

Cyclone comes with a set of separate binaries for most of the externals, but also comes with Cyclops: a single binary pack that contains the cyclone operators, which are 12 objects with non-alphanumeric names (check "cyclops-help.pd" for more info). Details on How To Install cyclone can be found here: https://github.com/porres/pd-cyclone/wiki/How-To-Install

<strong>Building Cyclone:</strong>

Since "Cyclone 0.1-alpha57", the cyclone package has relied on the new build system called "pd-lib-builder" by Katja Vetter (check the project in: <https://github.com/pure-data/pd-lib-builder>). 

* Compiling with pdlibbuilder

PdLibBuilder tries to find the Pd source directory at several common locations, but when this fails, you have to specify the path yourself using the pdincludepath variable. Example:

<pre>make pdincludepath=~/pd-0.47-1/src/  (for Windows/MinGW add 'pdbinpath=~/pd-0.47-1/bin/)</pre>

* Installing with pdlibbuilder

use "objectsdir" to set a relative path for your build, something like:

<pre>make install objectsdir=../cyclone-build</pre>

Then move it to your preferred install folder for Pd.

-------

<strong>A Brief History of Cyclone's Development:</strong>

Excerpt from Cyclone's original Readme (by its original author Krzysztof Czaja):

* "Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. (...)." The full original readme is provided in this repository at: <https://github.com/porres/pd-cyclone/blob/master/maintenance/README_original.txt>

Cyclone's original author Krzysztof Czaja worked on it as part of his miXed library from 2002 to 2005 and later abandoned it all together. In parallel, miXed had been incorporated into Pd Extended, and it ended up eventually under the maintenance of Hans-Christoph Steiner - the main developer and maintainer of Pd-Extended. When Pd Extended was abandoned after its last release from jan 2013, cyclone and miXed were left unmaintained as a result. In dec-2014, Fred Jan Kraan took over maintainance and development for cyclone but decided to abandone development for it in feb-2016, though maintenance for his last package is still active.

Since february 21st 2016, an active further development of cyclone started on this repository by Alexandre Porres, Derek Kwan and Matt Barber (and other collaborators).

* <strong>About Other Repositories:</strong>

=> Original Repository (up to version 0.1-Alpha-57):
The original repository of MiXed as part of Pd Extended - containing cyclone and more (such as 'toxy') - resides at <https://svn.code.sf.net/p/pure-data/svn/trunk/externals/miXed/cyclone> and the migrated repository: <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/>. This repository embraces work from three different maintainance phases: 

- Czaja's era (up to 0.1-Alpha55): Cjaza (the original author) worked on cyclone from version 01-alpha-01 (2002) to 0.1-alpha-55 (2005). 

- Hans era (0.1-Alpha-55 and 0.1-Alpha-56): Hans maintained cyclone from 2005 to 2013. The 0.1-Alpha55 version of cyclone is found in most of Pd-Extended versions up to Pd-Extended 0.42-5. The last release of Pd-Extended is 0.43.4 from jan-2013 and it carries the 0.1-Alpha56 version of cyclone, which can also be found as "cyclone-v0-0extended" when searching for externals in Pd Vanilla.

- Kraan era (0.1-Alpha-57): The last version currently found in the original cyclone repository is 0.1-alpha-57, which was developed by Fred Jan Kraan, who started maintaining cyclone in dec 2014. Although a compiled version for this version was made available at some point, it is not found there anymore and it is officially an "unreleased" version.

=> Fred Jan Kraan's Repository (0.1-Alpha-57 and 0.2beta): Fred Jan Kraan forked the original repository to <https://github.com/electrickery/pd-miXedSon>, but containing only the cyclone library. This repository starts with the last snapshot of the original cyclone repository - cyclone version 0.1alpha-57, from october 2015 (found in: <http://puredata.info/downloads/cyclone/releases/0.1-alpha57> - and moves on to a new version (cyclone 0.2)! Cyclone "0.2beta" presents further development on Fred Jan's branch, not available in the original repository and is found when searching for externals in Pd Vanilla.

-------

<strong>About This Repository And Its Goals:</strong>

Location of this repository is: https://github.com/porres/pd-cyclone. This repository is faithful to the original goal of cyclone in creating an external Pd package with a collection of objects cloned and compatible to Max/MSP objects. This repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. 

Please get in touch if you're willing to collaborate (one possible way is through the Pd-list https://lists.puredata.info/listinfo/pd-list)
