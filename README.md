Cyclone: a set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pure Data Vanilla with objects cloned from Max/MSP. It also provides some level of compatibility between the two environments, helping users of both systems in the development of cross-platform patches. 

--------------------

Pure Data (or just "Pd") project is found at: https://sourceforge.net/p/pure-data/pure-data/ci/master/tree/

Max is found at: https://cycling74.com/products/max/

<strong>Exerpt from Cyclone's original Readme (by Krzysztof Czaja):</strong>

"Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches.

(more words and about the original project on the original readme provided in this repository: <https://github.com/porres/pd-cyclone/blob/master/maintenance/text_files/README_original.txt>)

-------

<strong>History of cyclone's maintenance up to the present day:</strong>

Original author of cyclone is Krzysztof Czaja, who developed it from 2002-2005. After that, cyclone ended up incorporated into Pd Extended, and was eventually under the maintenance of Hans-Christoph Steiner until Pd Extended was left unmaintained (up to jan-2013). Fred Jan Kraan maintained cyclone from dec-2014 to feb-2016. Since feb-2016, a new maintenance is being proposed on this repository by Porres/Schiavoni & collaborators.

-------

<strong>About this repository and earlier versions:</strong>

Location of this repository is: https://github.com/porres/pd-cyclone. 

The original cyclone library and repository, amongst other things, is found at the <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/>, which carries the cyclone library version 0.1-Alpha56, present in the last release of Pd Extended [version 0.43.4] from january  2013. The 0.1-Alpha56 version is now also available via the deken plug in as "cyclone-v0-0extended". Check the deken plugin at: <https://github.com/pure-data/deken>.  

A fork from this original repository is found at <https://github.com/electrickery/pd-miXedSon> and includes the version 0.1alpha57 from october 2015 (which can be found here <https://github.com/porres/pd-cyclone/releases/tag/0.1alpha57>) and a newer version "0.2beta1" (from december 2015). Cyclone 0.2beta1 is available for download via the deken plugin as well - but you can also find it here <https://github.com/porres/pd-cyclone/releases/tag/0.2beta1>. 

The work made up to cyclone 0.2beta1 is now forked to this repository. An update version of cyclone could emerge anytime from this repository with bug fixes, new objects and new functionalities aiming to an up to date set of Max/MSP cloned objects.

-------

<strong>This repository's Goals & Further Development:</strong>

Cyclone was originally developed around the time of Max/MSP 4.6 - but since then, Max has introduced new functionalities on some objects in Max 5+ versions. Thus, some objects in cyclone are outdated to Max/MSP 4. Cyclone "0.2beta1" had still some objects with functionalities from Max/MSP 4, but with already some objects updated to include new functionalities introduced in Max 5 (but still missing functionalities from Max 6+ versions). 

This repository is faithful to the original goal of cyclone in creating an external Pd package with a collection of objects cloned and compatible to Max/MSP objects. If in agreement to the Pd community of users and developers, this repository will release updates of the cyclone project (starting with version "0.3-beta-1"). If so be it, besides bug fixes in current existing objects, new/further developments can include:

<strong>- A)</strong> New functionalities introduced in newer versions of Max (from Max 5 and on) into the already existing objects in cyclone.

<strong>- B)</strong> New objects comptaible to the most current Max version (Max 7 nowadays). 

This repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. Ideally, every possible objects could be cloned or updated, but since this is a fair amount of work, collaborators are free to decide and work on what they consider more relevant and important. Any object that has full or "most possible" compatibility to current Max/MSP can be included. 

Some collaborators are already helping coding new objects and updating/fixing old ones, a list of known bugs and things "to do" will be made available with the first release to come from this repository.

Check more stuff in our wiki page: https://github.com/porres/pd-cyclone/wiki/

February 22nd 2016 (last edited, april 12th)

-------
<strong>pd-lib-builder</strong>

Cyclone has relyed on a new build system called "pd-lib-builder" by Katja Vetter, available here: <https://github.com/pure-data/pd-lib-builder> 

The transition to this new build system started with 0.1-alpha57. The old build configuration used to also compile the objects in libraries. The new system, at the moment, only builds each object in a separate file plus the cyclone sub-library containing 12 non alphanumeric objects.

