Cyclone is a library of clones of Max/MSP objects for Pure Data

<strong>Original Project Description (by Krzysztof Czaja):</strong>

Cyclone is a library of PureData classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. In this respect, cyclone has much in common with Thomas Grill's flext, and flext-based externals. See Thomas' page. While flext enables developing new cross-platform classes, cyclone makes existing classes cross-platform. 

Cyclone also comes handy, somewhat, in the task of importing Max/MSP 4.x patches into Pd. Do not expect miracles, though, it is usually not an easy task.

<strong>Previous & Current state / Goals & Further Development:</strong>

Original author of cyclone is Krzysztof Czaja, who maintained it from 2002-2005. Cyclone was a part of Pd Extended and was maintained by Hans-Christoph Steiner until Pd Extended was left unmaintained (up to 2013). Fred Jan Kraan maintained cyclone from dec-2014 to feb-2016. A new maintenance is now being proposed by Porres and Schiavoni on this repository.

The original location of this new cyclone repository is https://github.com/porres/pd-cyclone. This repository started as a 'fork' from Fred Jan Kraan's github (https://github.com/electrickery/pd-cyclone), which was a 'fork' on its own from <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/> containing only the cyclone library (version 0.1-Alpha56, present in the last release of Pd Extended [version 0.43.4] - available in the deken plug in as "cyclone-v0-0extended"). A new version 0.2beta1 is also available for download via the deken plug in and was forked this new repository. Another version (0.2beta2) will emerge from this repository with new objects, new functionalities, and bug fixes as soon as possible.

This repository is faithful to the original goal of cyclone in creating an external library with a collection of Max/MSP objects for Pure Data. Original version of cyclone was developed in time of Max/MSP 4.6, so most of the work made in cyclone is still outdated to that version. If in agreement to the Pd community of users and developers, this repository can become an update to the cyclone project, where, besides current bug fixes, new/further developments can include:

<strong>- A)</strong> New functionalities introduced in newer versions of Max/MSP into the already existing objects in cyclone.

<strong>- B)</strong> New objects according to the most current version of Max/MSP version (version 7 nowadays). 

This repository/project is open to collaboration to anyone who wishes to work according to the central goal of Max/MSP compatibility. Ideally, all of the possible objects could be cloned and kept updated, but this is a fair amount of work so collaborators are free to decide and work on what they consider more relevant and important. Any object that has full or "most possible" compatibility to current Max/MSP can be included. A list of known bugs and things "to do" will be made available with the first release (0.2beta2) to come from this repository ASAP.

February 22nd 2016
