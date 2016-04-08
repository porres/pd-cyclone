Cyclone: a set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pd Vanilla with objects cloned from Max/MSP. For users of both Max/MSP and Pd, it provides some level of compatibility between the two environments, helping in the development of cross-platform patches. 

--------------------

<strong>Original Project Description (by Krzysztof Czaja):</strong>

Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. In this respect, cyclone has much in common with Thomas Grill's flext, and flext-based externals. See Thomas' page. While flext enables developing new cross-platform classes, cyclone makes existing classes cross-platform. 

Cyclone also comes handy, somewhat, in the task of importing Max/MSP 4.x patches into Pd. Do not expect miracles, though, it is usually not an easy task.

(more about the original project on the original readme provided in this repo)

-------

<strong>History of cyclone up to the present day:</strong>

Original author of cyclone is Krzysztof Czaja, who maintained it from 2002-2005. After that, cyclone was also a part of Pd Extended and was maintained by Hans-Christoph Steiner until Pd Extended was left unmaintained (up to 2013). Fred Jan Kraan maintained cyclone from dec-2014 to feb-2016. Since feb-2016, a new maintenance is being proposed on this repository by Porres/Schiavoni & collaborators.

-------

<strong>About this repository - Goals & Further Development:</strong>

Location of this repository is: https://github.com/porres/pd-cyclone. 

The original cyclone library, amongst other things, is found at the <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/> repository, which carries the cyclone library version 0.1-Alpha56, present in the last release of Pd Extended [version 0.43.4] - made available in january  2013. The 0.1-Alpha56 version is now also available in the deken plug in as "cyclone-v0-0extended". A version 0.1alpha57 was developed (and can be found here <https://github.com/porres/pd-cyclone/releases/tag/0.1alpha57>).

A fork from this original repository is found at <https://github.com/electrickery/pd-miXedSon> and includes the version 0.1alpha57 and a newer version "0.2beta1" (from december 2015). Cyclone 0.2beta1 is also available for download via the deken plug in. The work from <https://github.com/electrickery/pd-miXedSon> is now forked to this repository. 

An update version of cyclone could emerge anytime from this repository with bug fixes, new objects and new functionalities aiming to an up to date set of Max/MSP cloned objects. Cyclone was originally developed around the time of Max/MSP 4.6 and since then Max has introduced new functionalities on some objects in Max 5+ versions. Thus, some objects in cyclone are outdated to Max/MSP 4. Cyclone "0.2beta1" had still some objects with functionalities from Max/MSP 4, but with already some objects updated to include new functionalities introduced in Max 5 (but still missed functionalities from Max 6+ versions). 

This repository is faithful to the original goal of cyclone in creating an external Pd package with a collection of objects cloned and compatible to Max/MSP objects. If in agreement to the Pd community of users and developers, this repository will release updates of the cyclone project (starting with version "0.3-beta-1"). If so be it, besides bug fixes in current existing objects, new/further developments can include:

<strong>- A)</strong> New functionalities introduced in newer versions of Max (from 5 to 7) into the already existing objects in cyclone.

<strong>- B)</strong> New objects according to the most current Max version of version (7 nowadays). 

This repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. Ideally, every possible objects could be cloned or updated, but since this is a fair amount of work, collaborators are free to decide and work on what they consider more relevant and important. Any object that has full or "most possible" compatibility to current Max/MSP can be included. 

Some collaborators are already helping coding new objects and updating/fixing old ones, a list of known bugs and things "to do" will be made available with the first release to come from this repository.


February 22nd 2016 (last edited, april 8th)

-------
<strong>pdlibbuilder</strong>

Cyclone has relyed on a new build system called "pd-lib-builder" by Katja Vetter, available here: <https://github.com/pure-data/pd-lib-builder> 

The transition to this new build system started with 0.1-alpha57. The old build configuration used to also compile the objects in libraries. The new system builds each object in a separate file plus the cyclone sub-library containing 12 non alphanumeric objects.

