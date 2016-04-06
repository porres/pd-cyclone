https://github.com/porres/pd-cyclone

-------
cyclone
-------

Cyclone is a library of PureData classes, bringing some level of compatibility
between Max/MSP and Pd environments, mainly for people using both Max and Pd, and
thus wanting to develop cross-platform patches. Cyclone comes with individual external binaries and a cyclone sub-library:

 * The individual external binaries can be loaded only by inserting cyclone folder in the search path of Pd. 

 * The cyclone sub-library loads a small set of operators (!-, !/, ==~, !=~, <~, <=~, >~, >=~, !-~, !/~, %~, +=~) and can be loaded in Pd’s startup once the search path contains the cyclone folder.

Current objects:

Max Objects: accum, acos, active, anal, Append (more
info), asin, bangbang, bondo, Borax, Bucket, buddy, capture, cartopol, Clip,
coll, comment, cosh, counter, cycle, decide, Decode, drunk, flush, forward,
fromsymbol, funbuff, funnel, gate, grab, Histo, iter, match, maximum, mean,
midiflush, midiformat, midiparse, minimum, mousefilter, MouseState, mtr (more
info), next, offer, onebang, past, Peak, poltocar, prepend (more info), prob,
pv, seq (more info), sinh, speedlim, spell, split, spray, sprintf, substitute,
sustain, switch, Table (more info), tanh, thresh, TogEdge, tosymbol, Trough,
universal, urn, Uzi, xbendin, xbendout, xnotein, xnoteout, and zl.

MSP Objects: abs~, acos~, acosh~, allpass~, asin~, asinh~, atan~,
atan2~, atanh~, average~, avg~, bitand~, bitnot~, bitor~, bitshift~, bitxor~,
buffir~, capture~, cartopol~, change~, click~, Clip~, comb~, cosh~, cosx~,
count~, curve~, cycle~, delay~, delta~, deltaclip~, edge~, frameaccum~,
framedelta~, index~, kink~, Line~, linedrive, log~, lookup~, lores~, matrix~
(more info), maximum~, minimum~, minmax~, mstosamps~, onepole~, peakamp~,
peek~, phasewrap~, pink~, play~, poke~, poltocar~, pong~, pow~, rampsmooth~,
rand~, record~, reson~, sah~, sampstoms~, Scope~, sinh~, sinx~, slide~,
Snapshot~, spike~, svf~, tanh~, tanx~, train~, trapezoid~, triangle~,
vectral~, wave~, and zerox~.


Caveats:

* The binaries provided in this release were tested in xxx

* First capital letters.