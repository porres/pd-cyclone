
Cyclone 0.3 alpha 1 
-------

Released: February 21st, 2017

Cyclone: A set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pure Data with objects cloned from cycling74's Max/MSP. It thus provides some good level of compatibility between the two environments, helping users of both systems in the development of equivalent patches. 

-------

Pure Data (or just "Pd") project is found at: https://sourceforge.net/p/pure-data/pure-data/ci/master/tree/

Max is found at: https://cycling74.com/

-------

<strong>About Cyclone 0.3:</strong>

Since cyclone 0.2 was still mostly outdated and compliant to Max 4.0 (from the early 2000s), the main focus of cyclone 0.3 (currently in alpha stage) has been on updating cyclone objects to the latest Max version (Max 7.3.1 at the time of this release). We're also fixing many bugs, rewriting the documentation and including new objects. So far we have:

- 60 updated objects (only 3 reamining to be updated);
- 54 bug fixes (including objects that were and were not updated);
- 46 new objects (some still experimental);
- A newly written documentation, from scratch (yet to include meta and related objects)

Check details in our changelog: https://github.com/porres/pd-cyclone/wiki/cyclone-0.3-changlelog 

Notes on How To Install cyclone can be found here: https://github.com/porres/pd-cyclone/wiki/How-To-Install

<strong>Building Cyclone:</strong>

Since cyclone 0.1-alpha57, the cyclone package started relying on the new build system called "pd-lib-builder" by Katja Vetter (check the project in: <https://github.com/pure-data/pd-lib-builder>). 

* Compiling with pdlibbuilder

PdLibBuilder tries to find the Pd source directory at several common locations, but when this fails, yo have to specify the path yourself using the pdincludepath variable. Example:

<pre>make pdincludepath=~/pd-0.47-1/src/  (for Windows/MinGW add 'pdbinpath=~/pd-0.47-1/bin/)</pre>

* Installing with pdlibbuilder

The default path for installing might not be the best, surely for testing. Use the pkglibdir variable for this. Example:

<pre>make install pkglibdir=~/pd-externals/</pre>

-------

<strong>Excerpt from Cyclone's original Readme (by its original author Krzysztof Czaja):</strong>

"Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. (...)."

The full original readme is provided in this repository at: <https://github.com/porres/pd-cyclone/blob/master/maintenance/README_original.txt>

-------

<strong>A Brief History of cyclone's development:</strong>

Cyclone's original author is Krzysztof Czaja, who worked on it as part of his miXed library from 2002 to 2005 and later abandoned it all together. In parallel, miXed had been incorporated into into Pd Extended, so it eventually was under the maintenance of Hans-Christoph Steiner - the main developer and maintainer of Pd-Extended until 2013, when Pd Extended was abandoned (leaving cyclone and miXed also unmaintained as a result). In dec-2014, Fred Jan Kraan took over maintainance and development for cyclone but decided to abandone development for it in feb-2016, though maintenance for his last package is still active.

Since february 21st 2016, an active further development of cyclone started on this repository by Alexandre Porres, Derek Kwan and Matt Barber (and other collaborators).

-------

<strong>About other repositories:</strong>

=> <strong>Original Repository (up to version 0.1-Alpha-57):</strong>
The original repository of MiXed as part of Pd Extended - containing cyclone and more (such as 'toxy') - resides at <https://svn.code.sf.net/p/pure-data/svn/trunk/externals/miXed/cyclone> and the migrated repository: <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/>. This repository embraces work from three different maintainance phases: 

- <strong>Czaja's era (up to 0.1-Alpha55):</strong> Cjaza (the original author) worked on cyclone from version 01-alpha-01 (2002) to 0.1-alpha-55 (2005). 

- <strong>Hans era (0.1-Alpha-55 and 0.1-Alpha-56):</strong> Hans maintained cyclone from 2005 to 2013. The 0.1-Alpha55 version of cyclone is found in most of Pd-Extended versions up to Pd-Extended 0.42-5. The last release of Pd-Extended is 0.43.4 from jan-2013 and it carries the 0.1-Alpha56 version of cyclone, which can also be found as "cyclone-v0-0extended" when searching for externals in Pd Vanilla.

- <strong>Kraan era (0.1-Alpha-57):</strong> The last version currently found in the original cyclone repository is 0.1-alpha-57, which was developed by Fred Jan Kraan, who started maintaining cyclone in dec 2014. Although a compiled version for this version was made available at some point, it is not found there anymore and it is officially an "unreleased" version.

=> <strong>Fred Jan Kraan's Repository (0.1-Alpha-57 and 0.2beta):</strong> Fred Jan Kraan forked the original repository to <https://github.com/electrickery/pd-miXedSon>, but containing only the cyclone library. This repository starts with the last snapshot of the original cyclone repository - cyclone version 0.1alpha-57, from october 2015 (found in: <http://puredata.info/downloads/cyclone/releases/0.1-alpha57> - and moves on to a new version (cyclone 0.2)! Cyclone "0.2beta" presents further development on Fred Jan's branch, not available in the original repository and is found when searching for externals in Pd Vanilla.

-------

<strong>About this repository and its Goals:</strong>

Location of this repository is: https://github.com/porres/pd-cyclone. This repository is faithful to the original goal of cyclone in creating an external Pd package with a collection of objects cloned and compatible to Max/MSP objects, which is a ground rule manifested by Hans-Christoph Steiner on the Pd-List about further maintenance of cyclone. Keep working and expanding the Max compatibility is the main goal of this development.

This repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. 
