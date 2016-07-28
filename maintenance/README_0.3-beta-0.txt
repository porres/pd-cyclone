———————————————————————————————————————————————————————————————————————————

Cyclone: a set of Pure Data objects cloned from Max/MSP 

https://github.com/porres/pd-cyclone

———————————————————————————————————————————————————————————————————————————

Current version: cyclone 0.3-beta-0 
released: xx/xx/2016

This release was tested in Pd Vanilla 0.47-1 (32 & 64 bits);
It is not expected to work in earlier versions.


——————————————
About Cyclone:
——————————————

Cyclone expands Pd Vanilla with objects cloned from Max/MSP. For users of both Max/MSP and Pd, it provides some level of compatibility between the two environments, helping in the development of cross-platform patches. 


——————————————————————————
About the current version:
——————————————————————————

Cyclone 0.3-beta-0 carries a a set of over 200 Pure Data externals, it comes with; A) single external binaries or abstractions & B) cyclone sub-library:

- A) The individual external binaries and abstractions can be loaded only by inserting cyclone folder in the search path of Pd (or using Pd Vanilla’s [declare] object). 

- B) The cyclone sub-library loads a small set of objects with non alphanumeric names: [!-], [!/], [==~], [!=~], [<~], [<=~], [>~], [>=~], [!-~], [!/~], [%~], [+=~]. The library can be loaded in Pd’s startup (or using Pd Vanilla’s [declare] object). Objects with alphanumeric names corresponding to these objects are also available as single binaries outside this sub-library.


———————————————————————————————————————————————————————————————————————————

! Check “How to Install” for more details:
     https://github.com/porres/pd-cyclone/wiki/How-To-Install

! Check list of objects:
     https://github.com/porres/pd-cyclone/wiki/List-of-objects

! Report a bug or issue:
     https://github.com/porres/pd-cyclone/issues
