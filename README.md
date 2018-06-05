

Cyclone: A set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pure Data with objects cloned from cycling74's Max/MSP and provides some good level of compatibility between the two environments. 

-------

Pure Data (or just "Pd") project is found at: https://sourceforge.net/p/pure-data/pure-data/ci/master/tree/
or in its github mirror https://github.com/pure-data/pure-data. The offical download page is here: http://msp.ucsd.edu/software.html

Max is found at: https://cycling74.com/

-------

Current Release: Cyclone 0.3 beta-4 (this release needs at least Pd Vanilla 0.48-1)

Release Date: June 5th 2018

Find cyclone's latest releases at: https://github.com/porres/pd-cyclone/releases or directly via Pd's external manager (Help => Find Externals)

-------
<strong>About Cyclone 0.3:</strong>

Cyclone 0.3 needs at least Pd Vanilla 0.48-1 and it doesn't fully work in any other version of Pd or in other flavours such as Pd Extended and Purr Data - both of which carry outdated versions of cyclone. Pd-Extended carries older 0.1 versions. Purr Data still carries the outdated cyclone 0.2 release, which is not yet fully ported either to Purr Data.

The original author of Cyclone (Krzysztof Czaja) abandoned it in 2005 at version 0.1alpha55. Cyclone was then incorporated and available in Pd-Extended, where it only a had a minor update in 2013 (0.1alpha56) under the maintenance of Hans-Christoph Steiner, right before Cyclone and Pd Extended were abandoned altogether. Under a new maintenance phase by Fred Jan Kraan, 0.1alpha57 and cyclone 0.2 beta versions were released, still closely related to the previous '0.1alpha' releases and mostly compliant to Max 4.0!

The main goal of cyclone 0.3 is to update Cyclone's objects to the latest Max version, which is Max 7.3.5 at the time of this release. We're also fixing many bugs, rewriting the documentation and including new objects. So far we have:

- 61 updated objects;
- 65 fixed objects (including objects that were updated, will be updated and did not need updates);
- 44 new objects;
- A newly written documentation, taking care of numerous issues (yet to include "meta" and related objects)

Check details in the provided CHANGELOG.txt file, or here: https://github.com/porres/pd-cyclone/wiki/cyclone-0.3-changlelog

-------

<strong>Installing Cyclone:</strong>

You can compile cyclone from the source provided in this repository for the current bleeding edge last state or download one of the more stable compiled releases from <https://github.com/porres/pd-cyclone/releases>. A good alternative is simply use Pd's own external download manager (a.k.a deken plugin), just click on the "find externals" option under the Help menu and search for cyclone.

When installing cyclone, just make sure the cyclone folder is included in a folder that Pd searches for, such as ~/Documents/Pd/externals - which is what Pd directs you to do (since version 0.48).

Now you can install cyclone by loading it in the startup: go to "Preferences => Startup", then click "New", type "cyclone" and hit ok. Next time you restart Pd, the cyclone library binary will be loaded.

This library binary loads the operators objects (which are: !- , !-~ , !/ , !/~ , !=~ , %~ , +=~ , <=~ , <~ , ==~ , >=~ and >~), but it also adds cyclone's path to Pd's preferences, so you can load the other objects from cyclone (which are separate binaries or abstractions). 

You can also use the [declare -lib cyclone] in a patch to load the library if you don't want to always have cyclone loaded when Pd starts.

Note that loading the [cyclone] object also loads the library, see its help file for more details.

-------

<strong>Building Cyclone for Pd Vanilla:</strong>

Since "Cyclone 0.1-alpha57", the cyclone package has relied on the new build system called "pd-lib-builder" by Katja Vetter (check the project in: <https://github.com/pure-data/pd-lib-builder>). 

* Compiling with pdlibbuilder

PdLibBuilder tries to find the Pd source directory at several common locations, but when this fails, you have to specify the path yourself using the pdincludepath variable. Example:

<pre>make pdincludepath=~/pd-0.48-1/src/  (for Windows/MinGW add 'pdbinpath=~/pd-0.48-1/bin/)</pre>

* Make Install

Use "objectsdir" to set a relative path for your build, something like:

<pre>make install objectsdir=../cyclone-build</pre>

Then move it to your preferred install folder for Pd.

-------

<strong>A Brief History of Cyclone's Development:</strong>

Excerpt from Cyclone's original Readme (by its original author Krzysztof Czaja):

* "Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. (...)." The full original readme is provided in this repository at: <https://github.com/porres/pd-cyclone/blob/master/maintenance/README_original.txt>

Cyclone's original author Krzysztof Czaja worked on it as part of his miXed library from 2002 to 2005 and later abandoned it all together. In parallel, miXed had been incorporated into Pd Extended, and it ended up eventually under the maintenance of Hans-Christoph Steiner - the main developer and maintainer of Pd-Extended. When Pd Extended was abandoned after its last release from jan 2013, cyclone and miXed were left unmaintained as a result. In dec-2014, Fred Jan Kraan took over maintainance and development for cyclone (but not the rest of the miXed library) and released 0.1alpha57 and cyclone 0.2 beta versions, but decided to abandon development for it in feb-2016.

Since february 21st 2016, an active further development of cyclone (now as version 0.3) started on this repository by Alexandre Porres, Derek Kwan, Matt Barber and other collaborators.

* <strong>About Other Repositories:</strong>

=> Original Repository (up to version 0.1-Alpha-56):
The original repository of MiXed as part of Pd Extended - containing cyclone and more (such as 'toxy') - resides at <https://svn.code.sf.net/p/pure-data/svn/trunk/externals/miXed/cyclone> and the migrated repository: <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/>. This repository embraces work from three different maintainance phases: 

- Czaja's era (until 2005 and up to 0.1-Alpha55): Czaja (the original author) worked on cyclone from version 01-alpha-01 (2002) to 0.1-alpha-55 (2005). 

- Hans era (up to 2013 and 0.1-Alpha-56): Hans maintained cyclone from 2005 to 2013. The 0.1-Alpha55 version of cyclone is found in most of Pd-Extended versions up to Pd-Extended 0.42-5. The last release of Pd-Extended is 0.43.4 from jan-2013 and it carries the 0.1-Alpha56 version of cyclone, which can also be found as "cyclone-v0-0extended" when searching for externals in Pd Vanilla.

- Kraan era (up to 2015): The later work in this repository was not made available into a new release

=> Fred Jan Kraan's Repository (0.1-Alpha-57 and 0.2beta):

Fred Jan Kraan forked the original repository to <https://github.com/electrickery/pd-miXedSon>, but containing only the Cyclone library. This repository has a few releases - see https://github.com/electrickery/pd-miXedSon/releases - it starts with cyclone version 0.1alpha-57, from october 2015, which is basically the last developments from the original repository. Then it moves on to a new Cyclone 0.2 version which stopped at a beta stage.

-------

<strong>About This Repository And Its Goals:</strong>

This repository was forked from fred Jan Kraan's at cyclone 0.2 beta stage and is releasing new cyclone versions starting at cyclone 0.3. The location of this repository is: https://github.com/porres/pd-cyclone. This repository is faithful to the original goal of cyclone in creating an external Pd package with a collection of objects cloned and compatible to Max/MSP objects. This repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. 

Please get in touch if you're willing to collaborate (one possible way is through the Pd-list https://lists.puredata.info/listinfo/pd-list)
