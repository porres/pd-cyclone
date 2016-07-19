Cyclone: A set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pure Data with objects cloned from cycling74's Max/MSP. It thus provides some level of compatibility between the two environments, helping users of both systems in the development of cross-platform patches. 

--------------------

Pure Data (or just "Pd") project is found at: https://sourceforge.net/p/pure-data/pure-data/ci/master/tree/

Max is found at: https://cycling74.com/

--------------------

<strong>Exerpt from Cyclone's original Readme:</strong>

"Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches."

(more in the original readme provided in this repository: <https://github.com/porres/pd-cyclone/blob/master/maintenance/text_files/README_original.txt>)

-------

<strong>History of cyclone's maintenance up to the present day:</strong>

Cyclone's original author is Krzysztof Czaja, who worked on it mostly from 2002-2005. After that, cyclone ended up incorporated into Pd Extended and was eventually under the maintenance of Hans-Christoph Steiner until Pd Extended was left unmaintained (up to jan-2013). After unmaintained for a while, Fred Jan Kraan did maintain cyclone from dec-2014 to feb-2016. Since feb-2016, a new maintenance is being proposed by Porres & collaborators on this repository.

-------

<strong>About earlier versions and other repositories:</strong>

Amongst other things, the original cyclone library and repository is found at the <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/> . The original repository carries the version 0.1-Alpha56 of cyclone, present in the last release of Pd Extended [version 0.43.4 from january  2013]. The 0.1-Alpha56 version is also available as "cyclone-v0-0extended" when searching for externals in Pd Vanilla (since version 0.47-0).

Fred Jan Kraan forked the original repository to <https://github.com/electrickery/pd-miXedSon>, further developments and releases are found in this repository, such as cyclone version 0.1alpha57 from october 2015 (available here: <https://github.com/porres/pd-cyclone/releases/tag/0.1alpha57>) and a newer version "0.2beta1" (from december 2015) - also found when searching for externals in Pd Vanilla. 


-------

<strong>About this repository's Goals & Further Development:</strong>

Location of this repository is: https://github.com/porres/pd-cyclone. 

When cyclone was been left unmaintained (in February 2016), Porres forked cyclone to this new repository, where developments for further releases have been made since then.

This repository is faithful to the original goal of cyclone in creating an external Pd package with a collection of objects cloned and compatible to Max/MSP objects. 

Cyclone was originally developed around the time of Max/MSP 4.6 - but Max has since then introduced new functionalities on some objects in the more recent versions. Thus, some objects in cyclone are outdated to Max/MSP 4. Cyclone "0.2beta1" still has some objects with functionalities from Max/MSP 4, but with already some objects updated to include some of the new functionalities introduced in Max 5 (but still missing functionalities from Max 6+ versions). 

If in agreement to the Pd community, this repository will release updates of the cyclone project (starting with version "0.3-beta-1"). If so be it, besides bug fixes in current existing objects, new/further developments shall include:

<strong>- A)</strong> New functionalities introduced in newer versions of Max (from Max 5 and on) into the already existing objects in cyclone.

<strong>- B)</strong> New objects compatible to the most current Max version (Max 7 nowadays). 

This repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. Ideally, every possible objects could be cloned or updated, but since this is a fair amount of work, collaborators are free to decide and work on what they consider more relevant and important. Any object that has full or "most possible" compatibility to current Max/MSP can be included. 

Some collaborators are already helping coding new objects and updating/fixing old ones, a list of known bugs and things "to do" will be made available with the first release to come from this repository.

Check some stuff we've been doing in our changelog: https://github.com/porres/pd-cyclone/wiki/changelog

Readme file created February 22nd 2016 (last edited, july 19th 2016)

-------
<strong>Building Cyclone</strong>

Since version 0.1-alpha57, cyclone has relied on the new build system called "pd-lib-builder" by Katja Vetter (check the project in: <https://github.com/pure-data/pd-lib-builder>). With this new build system, all you have to do is type "make" and build cyclone in different platforms.  

Cyclone originally used to also compile the objects in libraries (such as "hammer" for control objects and "sickle" for signal objects). But in the Pd extended era, only single binaries were available. The new build system system, up to this moment, only builds each object in a separate file as in the old Pd extended days - with the exception a sub-library (containing 12 non alphanumeric objects). The current maintenance considers to restore the possibility of also compiling cyclone as a library in the same way it was originally developed.
