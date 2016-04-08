Cyclone - a set of Pure Data objects cloned from Max/MSP 
-------

> Cyclone expands Pd Vanilla with objects cloned from Max/MSP. For users of both Max/MSP and Pd, It provides some level of compatibility between the two environments, helping in the development of cross-platform patches. 

--------------------

<strong>Original Project Description (by Krzysztof Czaja):</strong>

Cyclone is a library of Pure Data classes, bringing some level of compatibility between Max/MSP and Pd environments. Although being itself in the early stage of development, it is meant to eventually become part of a much larger project, aiming at unification and standardization of computer musician's tools. 

In its current form, cyclone is mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. In this respect, cyclone has much in common with Thomas Grill's flext, and flext-based externals. See Thomas' page. While flext enables developing new cross-platform classes, cyclone makes existing classes cross-platform. 

Cyclone also comes handy, somewhat, in the task of importing Max/MSP 4.x patches into Pd. Do not expect miracles, though, it is usually not an easy task.

(more about the original project on the original readme provided at the bottom)

-------

<strong>History of cyclone up to the present day:</strong>

Original author of cyclone is Krzysztof Czaja, who maintained it from 2002-2005. After that, cyclone was also a part of Pd Extended and was maintained by Hans-Christoph Steiner until Pd Extended was left unmaintained (up to 2013). Fred Jan Kraan maintained cyclone from dec-2014 to feb-2016. Since feb-2016, a new maintenance is being proposed on this repository by Porres/Schiavoni & collaborators.

-------

<strong>About this repository - Goals & Further Development:</strong>

Location of this repository is: https://github.com/porres/pd-cyclone. 

The original cyclone library, amongst other things, is found at the <https://git.puredata.info/cgit/svn2git/libraries/miXed.git/> repository, which carries the cyclone library version 0.1-Alpha56, present in the last release of Pd Extended [version 0.43.4] - made available in january  2013. The 0.1-Alpha56 version is now also available in the deken plug in as "cyclone-v0-0extended". A fork from this original repository is found at <https://github.com/electrickery/pd-miXedSon> and includes a newer version "0.2beta1" of cyclone from december 2015 - also available for download via the deken plug in. The work from <https://github.com/electrickery/pd-miXedSon> is now forked this repository. 

An update version of cyclone could emerge anytime from this repository with bug fixes, new objects and new functionalities aiming to an up to date clone of Max/MSP objects. Cyclone was originally developed around the time of Max/MSP 4.6, since Max has introduced new functionalities on some objects in Max 5+ versions, some objects in cyclone are outdated with functionalities still from Max 4. Cyclone "0.2beta1" had still some objects with functionalities from Max/MSP 4, but with already some objects updated to include new functionalities introduced in Max/MSP 5 (so it still missed functionalities from Max 6+ versions). 

This repository is faithful to the original goal of cyclone in creating an external library with a collection of objects for Pure Data that are cloned and compatible to Max/MSP objects. If in agreement to the Pd community of users and developers, this repository will release updates of the cyclone project (starting with version "0.3-beta-1") where, besides bug fixes in current existing objects, new/further developments can include:

<strong>- A)</strong> New functionalities introduced in newer versions of Max/MSP (from version 5 to 7) into the already existing objects in cyclone.

<strong>- B)</strong> New objects according to the most current version of Max/MSP version (version 7 nowadays). 

This repository/project is open to collaboration to anyone who wishes to work according to the key and central goal of Max/MSP compatibility. Ideally, all of the possible Max/MSP objects could be cloned or updated, but this is a fair amount of work, so collaborators are free to decide and work on what they consider more relevant and important. Any object that has full or "most possible" compatibility to current Max/MSP can be included. Some collaborators are already helping coding new objects, a list of known bugs and things "to do" will be made available with the first release to come from this repository.


February 22nd 2016 (last edited, april 7th)


-------
<strong>pdlibbuilder</strong>
-------

Cyclone is currently relying on the "pd-lib-builder" by Katja Vetter, available here: https://github.com/pure-data/pd-lib-builder

<strong>Compiling with pdlibbuilder</strong>

PdLibBuilder tries to find the Pd source directory at several common locations, but when this fails, yo have to specify the path yourself using the pdincludepath variable. Example:

<pre>make pdincludepath=~/pd-0.46-6/src/  (for Windows/MinGW add 'pdbinpath=~/pd-0.46-6/bin/)</pre>

<strong>Installing with pdlibbuilder</strong>

The default path for installing might not be the best, surely for testing. Use the pkglibdir variable for this. Example:

<pre>make install pkglibdir=~/pd-externals/</pre>

============================================================================================================================

-------
Cyclone's original ReadMe:
-------

http://suita.chopin.edu.pl/~czaja/miXed/externs/cyclone.html

-------
cyclone
-------

Cyclone is a library of Pure Data classes, bringing some level of compatibility
between Max/MSP and Pd environments.  Although being itself in the early stage
of development, it is meant to eventually become part of a much larger
project, aiming at unification and standardization of computer musician's
tools.

In its current form, cyclone is mainly for people using both Max and Pd, and
thus wanting to develop cross-platform patches.  In this respect, cyclone has
much in common with Thomas Grill's flext, and flext-based externals.  See
Thomas' page.  While flext enables developing new cross-platform classes,
cyclone makes existing classes cross-platform.

Cyclone also comes handy, somewhat, in the task of importing Max/MSP patches
into Pd.  Do not expect miracles, though, it is usually not an easy task.

The entire cyclone library, which might be preloaded with either -lib cyclone
or -lib maxmode option, consists of:

 * the main hammer and sickle sub-libraries, containing Pd versions of,
   respectively, Max and MSP classes;

 * cyclone sub-library, taking care of loading hammer and sickle, and which
   itself contains: 
- a small set of operators !-, !/, ==~, !=~, <~, <=~, >~, >=~, !-~, !/~, %~, +=~  
- an importing mechanism provided by the cyclone class.

 * optional dummies sub-library, which contains a large set of dummy classes,
   serving as substitutions for Max/MSP classes not (yet) implemented in
   cyclone;

 * maxmode sub-library, which imposes strict compatibility mode, and loads all
   the other components, including dummies.

The two main sub-libraries might be loaded separately, by using -lib hammer
and/or -lib sickle options.  There is also a possibility of loading any single
class from hammer or sickle library dynamically (this feature is only
available in the linux snapshot).

Currently, the hammer part contains: accum, acos, active, anal, Append (more
info), asin, bangbang, bondo, Borax, Bucket, buddy, capture, cartopol, Clip,
coll, comment, cosh, counter, cycle, decide, Decode, drunk, flush, forward,
fromsymbol, funbuff, funnel, gate, grab, Histo, iter, match, maximum, mean,
midiflush, midiformat, midiparse, minimum, mousefilter, MouseState, mtr (more
info), next, offer, onebang, past, Peak, poltocar, prepend (more info), prob,
pv, seq (more info), sinh, speedlim, spell, split, spray, sprintf, substitute,
sustain, switch, Table (more info), tanh, thresh, TogEdge, tosymbol, Trough,
universal, urn, Uzi, xbendin, xbendout, xnotein, xnoteout, and zl.

The sickle part contains: abs~, acos~, acosh~, allpass~, asin~, asinh~, atan~,
atan2~, atanh~, average~, avg~, bitand~, bitnot~, bitor~, bitshift~, bitxor~,
buffir~, capture~, cartopol~, change~, click~, Clip~, comb~, cosh~, cosx~,
count~, curve~, cycle~, delay~, delta~, deltaclip~, edge~, frameaccum~,
framedelta~, index~, kink~, Line~, linedrive, log~, lookup~, lores~, matrix~
(more info), maximum~, minimum~, minmax~, mstosamps~, onepole~, peakamp~,
peek~, phasewrap~, pink~, play~, poke~, poltocar~, pong~, pow~, rampsmooth~,
rand~, record~, reson~, sah~, sampstoms~, Scope~, sinh~, sinx~, slide~,
Snapshot~, spike~, svf~, tanh~, tanx~, train~, trapezoid~, triangle~,
vectral~, wave~, and zerox~.

Cyclone comes without any documentation.  All the included patches were
created merely for testing.

Caveats:

* The binaries provided in this snapshot release are not expected to run
  inside of a pre-0.36 version of Pd, without prior recompiling.

* If a single -lib cyclone startup option is used, cyclone in turn loads its
  two main components: hammer and sickle.  If a single -lib maxmode startup
  option is used, all the remaining library components are going to be loaded:
  cyclone, hammer, sickle, and dummies.  In these cases, all the required
  libraries should be accessible by Pd.
