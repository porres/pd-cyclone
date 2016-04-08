
———————————————————————————————————————————————————————————————————————————

Cyclone - a set of objects for Pure Data cloned from Max/MSP 

https://github.com/porres/pd-cyclone

———————————————————————————————————————————————————————————————————————————

version: cyclone 0.3-beta-1 - released xx/xx/xx

This release was tested in Pd Vanilla 0.46-7 (32 & 64 bits);
It can work elsewhere, but hasn’t been tested.

———————————————————————————————————————————————————————————————————————————

——————————————
About Cyclone:
——————————————

Cyclone expands Pd Vanilla with objects cloned from Max/MSP. For users of both Max/MSP and Pd, it provides some level of compatibility between the two environments, helping in the development of cross-platform patches. 

———————————————————————————————————————————————————————————————————————————

Cyclone 0.3-beta-1 carries a a set of xxx Pure Data externals, it comes with A) individual external binaries & B) cyclone sub-library:

- A) The individual external binaries can be loaded only by inserting cyclone folder in the search path of Pd. It is recommend to include it in the “extra” folder of pd vanilla to work properly. 

- B) The cyclone sub-library loads a small set of objects with non alphanumeric names: [!-], [!/], [==~], [!=~], [<~], [<=~], [>~], [>=~], [!-~], [!/~], [%~], [+=~]. The library can be loaded in Pd’s startup once the search path contains the cyclone folder. Objects with alphanumeric names corresponding to these objects are also available as single binaries outside the cyclone library.

———————————————————————————————————————————————————————————————————————————

! Check “How to Install” for more details:
     https://github.com/porres/pd-cyclone/wiki/How-To-Install

! Check list of objects:
     https://github.com/porres/pd-cyclone/wiki/List-of-objects

! Report a bug or issue:
     https://github.com/porres/pd-cyclone/issues
