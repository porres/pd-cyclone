https://github.com/porres/pd-cyclone

—————————————————————————————————————————————————————
cyclone 0.3-beta-1 *

* The binaries and library provided in this release were tested in Pd Vanilla 0.46-7 32 and 64 bits - it can work elsewhere, but hasn’t been tested.

—————————————————————————————————————————————————————

About Cyclone:
——————————————

Cyclone is a library of Pure Data objects cloned from Max/MSP. It brings some level of compatibility between Max/MSP and Pd environments, mainly for people using both Max and Pd, and thus wanting to develop cross-platform patches. 

Cyclone comes with individual external binaries and a cyclone sub-library:

* The individual external binaries can be loaded only by inserting cyclone folder in the search path of Pd. It is recommend to include it in the “extra” folder of pd vanilla to work properly. 

 * The cyclone sub-library loads a small set of operators (!-, !/, ==~, !=~, <~, <=~, >~, >=~, !-~, !/~, %~, +=~) and can be loaded in Pd’s startup once the search path contains the cyclone folder. Alphanumeric versions of these objects are also available as single binaries outside this library subset.

! Check “How to Install” for more details 

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

To Do: include new objects