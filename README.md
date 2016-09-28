Cyclone: A set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pure Data with objects cloned from cycling74's Max/MSP. It thus provides some level of compatibility between the two environments, helping users of both systems in the development of equivalent patches. 

--------------------

Pure Data (or just "Pd") project is found at: https://sourceforge.net/p/pure-data/pure-data/ci/master/tree/

Max is found at: https://cycling74.com/

--------------------

<strong>Exerpt from Cyclone's original Readme:</strong>

"Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches."

(the full original readme is provided in this repository at: <https://github.com/porres/pd-cyclone/blob/master/maintenance/README_original.txt>)

-------

<strong>History of cyclone's maintenance up to the present day:</strong>

Cyclone's original author is Krzysztof Czaja, who worked on it mostly from 2002 to 2005 and later abandoned it. 

Cyclone then ended up incorporated into Pd Extended and, around 2007, was under the maintenance of Hans-Christoph Steiner - the main developer and maintainer of the Pd-Extended project. Cyclone was left unmaintained after the last release of Pd Extended (in jan-2013). After another hiatus, Fred Jan Kraan did maintain and develop for cyclone from dec-2014 to feb-2016, when development for cyclone was abandoned (though maintenance for his last package is still active). Since feb-2016, an active further development of cyclone is being proposed by Alexandre Porres & collaborators (mainly Derek Kwan and Matt Barber) on this repository.

-------

<strong>About earlier versions and other repositories:</strong>

Original Repository (up to version 0.1-Alpha-57):

Cyclone was originally part of the MiXed library (also written by cyclone's original author Krzysztof Czaja). The original repository of MiXed, containing cyclone and more (such as 'toxy , 'ViCious', 'riddle') resides at <https://svn.code.sf.net/p/pure-data/svn/trunk/externals/miXed/cyclone> and the migrated repository: <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/>. 

Krzysztof Czaja had a webpage for cyclone that no longer exists, but a copy of it from its last state is available here:  <http://fjkraan.home.xs4all.nl/digaud/puredata/cyclone/cyclone_site/cyclone.html> - where you can find the cyclone version 0.1-Alpha54 from 2005 for download. 

The 0.1-Alpha55 version of cyclone is found in most of Pd-Extended versions up to Pd-Extended 0.42-5. The last release of Pd-Extended is 0.43.4 from jan-2013 and it carries the 0.1-Alpha56 version of cyclone, which can also be found as "cyclone-v0-0extended" when searching for externals in Pd Vanilla (with the new build in deken plug in since Pd Vanilla 0.47-0).

The last version currently found in the original cyclone repository is 0.1-alpha-57, which was developed by Fred Jan Kraan. 

Fred Jan Kraan's Repository (versions 0.1-Alpha-57 and 0.2beta):

Fred Jan Kraan forked the original repository to <https://github.com/electrickery/pd-miXedSon>, but containing only the cyclone library, where he worked on further developments and releases, such as cyclone version 0.1alpha-57 from october 2015 (found in: <http://puredata.info/downloads/cyclone/releases/0.1-alpha57>). As mentioned, this version is also found in the last snapshot of the original cyclone repository.

Cyclone "0.2beta" is the next and last currently released version of Cyclone. It is only found in Fred Jan Kraan's repository and also available when searching for externals in Pd Vanilla - check https://github.com/electrickery/pd-miXedSon/releases

-------

<strong>About this repository and its Goals:</strong>

Location of this repository is: https://github.com/porres/pd-cyclone. 

When cyclone was been left unmaintained (in february 2016), Porres forked cyclone in its last 0.2beta1 state to this new repository, where developments for a further release have been in the makings since then.

This repository is faithful to the original goal of cyclone in creating an external Pd package with a collection of objects cloned and compatible to Max/MSP objects, which is a ground rule manifested by Hans-Christoph Steiner on the Pd-List about further maintenance of cyclone.

Cyclone was originally developed in Max/MSP 4 era - but Max has since then introduced new functionalities on some objects in the more recent versions (from Max 5 to Max 7). The current version of Cyclone (0.2beta) is still mostly outdated to Max/MSP 4, but with already some objects updated to include some of the new functionalities introduced in Max 5 (but still missing functionalities from Max 6 and Max 7 versions). 

If in agreement to the Pd community, this repository will release updates of the cyclone project (starting with version "0.3-alpha-0"). If so be it, besides bug fixes in current existing objects, new/further developments shall include:

<strong>- A)</strong> New functionalities introduced in newer versions of Max (from Max 5 and on) into the already existing objects in cyclone.

<strong>- B)</strong> New objects compatible to the up to date Max version (currently Max 7.2.5). 

Such an update represents the major overhaul since the original author abandoned cyclone in 2005. With such a great deal of work, this repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. Ideally, any possible Max/MSP object could be cloned or updated, but since this is a fair amount of work, collaborators are free to decide and work on what they consider more relevant and important. Any object that has full or "most possible" compatibility to current Max/MSP can be included. 

Some collaborators are already helping coding new objects and updating/fixing old ones, a list of known bugs and things "to do" will be made available with the first release to come from this repository.

Check some stuff we've been doing in our changelog: https://github.com/porres/pd-cyclone/wiki/cyclone-0.3-changlelog

Readme file created on February 22nd 2016 (last edited on sep 28th 2016)

-------
<strong>Building Cyclone</strong>

Since version 0.1-alpha57, cyclone has relied on the new build system called "pd-lib-builder" by Katja Vetter (check the project in: <https://github.com/pure-data/pd-lib-builder>). With this new build system, all you have to do is type "make" and build cyclone for your platform.

Cyclone originally used to also compile the objects in libraries (such as "hammer" for control objects and "sickle" for signal objects). But at some point in the Pd-Extended era, only single binaries were available. The new build system system, up to this moment, only builds each object in a separate file as in the old Pd extended days - with the exception of a sub-library (containing 12 non alphanumeric objects). The current maintenance considers to restore the possibility of also compiling cyclone as a library as originally.
