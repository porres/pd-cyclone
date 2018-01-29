———————————————————————————————————————————————————————————————————————————

Cyclone: a set of Pure Data objects cloned from Max/MSP 

https://github.com/porres/pd-cyclone

———————————————————————————————————————————————————————————————————————————

Current version: cyclone 0.3
released: xx/xx/2018

This release needs at leas Pd Vanilla 0.48-1;
It is not expected to work in earlier versions.

——————————————
About Cyclone:
——————————————

Cyclone expands Pure Data with objects cloned from cycling74's Max/MSP. It thus provides some level of compatibility between the two environments.

——————————————————————————
About the current version:
——————————————————————————

Cyclone 0.3-beta-3 carries a a set of over 200 Pure Data externals, it comes with externals as abstractions, separate binaries, and a single binary containing the cyclone sub-library.

- The individual external binaries and abstractions can be loaded only by inserting cyclone folder in the search path of Pd (or using Pd Vanilla’s [declare] object). 

- B) The cyclone sub-library loads a small set of objects with non alphanumeric names: [!-], [!/], [==~], [!=~], [<~], [<=~], [>~], [>=~], [!-~], [!/~], [%~], [+=~]. The library can be loaded in Pd’s startup (or using Pd Vanilla’s [declare] object). Objects with alphanumeric names corresponding to these objects are also available as single binaries outside this sub-library.

———————————————————————————————————————————————————————————————————————————

! Check “How to Install” for more details:
     https://github.com/porres/pd-cyclone/wiki/How-To-Install

! Report a bug or issue:
     https://github.com/porres/pd-cyclone/issues
