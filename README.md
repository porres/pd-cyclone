<strong>Previous & Current state / Goals & Further Development</strong>

Original author of cyclone is Krzysztof Czaja, who maintained it from 2002-2005. In the late Pd Extended years (up to 2013), cyclone was a part of the project and was maintained by Hans-Christoph Steiner. Fred Jan Kraan maintained it from dec-2014 to feb-2016.

The original location of this repository is https://github.com/porres/pd-cyclone. This repository starts as a 'fork' from Fred Jan Kraan's github (https://github.com/electrickery/pd-cyclone), which was a 'fork' on its own from <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/> containing only the cyclone functionality and removing other libraries. The version of cyclone available in https://git.puredata.info/cgit/svn2git/libraries/miXed.git/ is 0.1-Alpha56, present in the last released Pd Extended (version 0.43.4). Under the maintenance of Fred, there's an unreleased 0.1-Alpha57 version.  A newer version 0.2beta1 was forked to this new repository. Version 0.2beta1 is available for download via the deken plug in, another version will emerge from this repository with new objects, new functionalities, and bug fixes as soon as possible.

This repository is faithful to the original goal of cyclone in creating an external library with a collection of Max/MSP objects for Pure Data. Original version of cyclone was developed in time of Max/MSP 4.6, so most of the work made in cyclone is still outdated to that version. If in agreement to the Pd community of users and developers, this repository can become an update to the cyclone project, where new further developments can include:

<strong>- A)</strong> New functionalities introduced in newer versions of Max/MSP into the already existing objects in cyclone.

<strong>- B)</strong> New objects according to the most current version of Max/MSP version (version 7 nowadays). 

This repository/project is open to collaboration to anyone who wishes to work according to the central goal of Max/MSP compatibilty. Ideally, all of the possible objects could be cloned and kept updated, but this is a fair amount of work so collaborators are free to decide and work on what they consider more relevant and important. Any object that has full or most possible compatibility to current Max/MSP can be included. A list of known bugs and things "to do" will be made available with the first release to come from this repository, scheduled for ASAP.

February 22nd 2016

<strong>Original Project Description by Krzysztof Czaja 2002-2005</strong>

"Cyclone is a library of PureData classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. In this respect, cyclone has much in common with Thomas Grill's flext, and flext-based externals. See Thomas' page. While flext enables developing new cross-platform classes, cyclone makes existing classes cross-platform. 

Cyclone also comes handy, somewhat, in the task of importing Max/MSP 4.x patches into Pd. Do not expect miracles, though, it is usually not an easy task."
