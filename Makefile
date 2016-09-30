################################################################################
######## Makefile for the Cyclone library of Pure Data externals ###############
################################################################################

lib.name = cyclone

# for the MINGW which has the timespec struct defined twice
cflags = -Ishared -DHAVE_STRUCT_TIMESPEC

uname := $(shell uname -s)
ifeq (MINGW,$(findstring MINGW,$(uname)))
ldlibs += -lpthread
exe.extension = .exe
endif

#######################################################################

                    ## START OF CYCLONE CLASSES ##

#######################################################################

# CONTROL CLASSES
accum.class.sources := binaries_src/control/accum.c
acos.class.sources := binaries_src/control/acos.c
asin.class.sources := binaries_src/control/asin.c
bucket.class.sources := binaries_src/control/bucket.c
cartopol.class.sources := binaries_src/control/cartopol.c
cosh.class.sources := binaries_src/control/cosh.c
flush.class.sources := binaries_src/control/flush.c
forward.class.sources := binaries_src/control/forward.c
fromsymbol.class.sources := binaries_src/control/fromsymbol.c
funnel.class.sources := binaries_src/control/funnel.c
mean.class.sources := binaries_src/control/mean.c
midiflush.class.sources := binaries_src/control/midiflush.c
midiformat.class.sources := binaries_src/control/midiformat.c
midiparse.class.sources := binaries_src/control/midiparse.c
next.class.sources := binaries_src/control/next.c
onebang.class.sources := binaries_src/control/onebang.c
peak.class.sources := binaries_src/control/peak.c
poltocar.class.sources := binaries_src/control/poltocar.c
sinh.class.sources := binaries_src/control/sinh.c
split.class.sources := binaries_src/control/split.c
sustain.class.sources := binaries_src/control/sustain.c
tanh.class.sources := binaries_src/control/tanh.c
trough.class.sources := binaries_src/control/trough.c
uzi.class.sources := binaries_src/control/uzi.c
xbendin.class.sources := binaries_src/control/xbendin.c
xbendin2.class.sources := binaries_src/control/xbendin2.c
xbendout.class.sources := binaries_src/control/xbendout.c
xbendout2.class.sources := binaries_src/control/xbendout2.c
xnotein.class.sources := binaries_src/control/xnotein.c
xnoteout.class.sources := binaries_src/control/xnoteout.c
# NEW ones:
acosh.class.sources := binaries_src/control/acosh.c
asinh.class.sources := binaries_src/control/asinh.c
atanh.class.sources := binaries_src/control/atanh.c
atodb.class.sources := binaries_src/control/atodb.c
dbtoa.class.sources := binaries_src/control/dbtoa.c
join.class.sources := binaries_src/control/join.c
pong.class.sources := binaries_src/control/pong.c
pak.class.sources := binaries_src/control/pak.c
round.class.sources := binaries_src/control/round.c
scale.class.sources := binaries_src/control/scale.c

# SIGNAL CLASSES:
abs~.class.sources := binaries_src/signal/abs.c
acos~.class.sources := binaries_src/signal/acos.c
acosh~.class.sources := binaries_src/signal/acosh.c
allpass~.class.sources := binaries_src/signal/allpass.c
asinh~.class.sources := binaries_src/signal/asinh.c
asin~.class.sources := binaries_src/signal/asin.c
atan~.class.sources := binaries_src/signal/atan.c
atan2~.class.sources := binaries_src/signal/atan2.c
atanh~.class.sources := binaries_src/signal/atanh.c
average~.class.sources := binaries_src/signal/average.c
avg~.class.sources := binaries_src/signal/avg.c
bitnot~.class.sources := binaries_src/signal/bitnot.c
bitshift~.class.sources := binaries_src/signal/bitshift.c
cascade~.class.sources := binaries_src/signal/cascade.c
change~.class.sources := binaries_src/signal/change.c
click~.class.sources := binaries_src/signal/click.c
clip~.class.sources := binaries_src/signal/clip.c
cosh~.class.sources := binaries_src/signal/cosh.c
cosx~.class.sources := binaries_src/signal/cosx.c
count~.class.sources := binaries_src/signal/count.c
comb~.class.sources := binaries_src/signal/comb.c
cycle~.class.sources := binaries_src/signal/cycle.c
delay~.class.sources := binaries_src/signal/delay.c
delta~.class.sources := binaries_src/signal/delta.c
deltaclip~.class.sources := binaries_src/signal/deltaclip.c
edge~.class.sources := binaries_src/signal/edge.c
kink~.class.sources := binaries_src/signal/kink.c
log~.class.sources := binaries_src/signal/log.c
lookup~.class.sources := binaries_src/signal/lookup.c
lores~.class.sources := binaries_src/signal/lores.c
maximum~.class.sources := binaries_src/signal/maximum.c
minimum~.class.sources := binaries_src/signal/minimum.c
mstosamps~.class.sources := binaries_src/signal/mstosamps.c
onepole~.class.sources := binaries_src/signal/onepole.c
overdrive~.class.sources := binaries_src/signal/overdrive.c
peakamp~.class.sources := binaries_src/signal/peakamp.c
phasewrap~.class.sources := binaries_src/signal/phasewrap.c
pink~.class.sources := binaries_src/signal/pink.c
pong~.class.sources := binaries_src/signal/pong.c
pow~.class.sources := binaries_src/signal/pow.c
rampsmooth~.class.sources := binaries_src/signal/rampsmooth.c
rand~.class.sources := binaries_src/signal/rand.c
reson~.class.sources := binaries_src/signal/reson.c
sah~.class.sources := binaries_src/signal/sah.c
sampstoms~.class.sources := binaries_src/signal/sampstoms.c
sinh~.class.sources := binaries_src/signal/sinh.c
sinx~.class.sources := binaries_src/signal/sinx.c
slide~.class.sources := binaries_src/signal/slide.c
snapshot~.class.sources := binaries_src/signal/snapshot.c
spike~.class.sources := binaries_src/signal/spike.c
svf~.class.sources := binaries_src/signal/svf.c
tanh~.class.sources := binaries_src/signal/tanh.c
tanx~.class.sources := binaries_src/signal/tanx.c
teeth~.class.sources := binaries_src/signal/teeth.c
train~.class.sources := binaries_src/signal/train.c
trapezoid~.class.sources := binaries_src/signal/trapezoid.c
triangle~.class.sources := binaries_src/signal/triangle.c
vectral~.class.sources := binaries_src/signal/vectral.c
zerox~.class.sources := binaries_src/signal/zerox.c
# NEW ones:
atodb~.class.sources := binaries_src/signal/atodb.c
biquad~.class.sources := binaries_src/signal/biquad.c
bitsafe~.class.sources := binaries_src/signal/bitsafe.c
cross~.class.sources := binaries_src/signal/cross.c
dbtoa~.class.sources := binaries_src/signal/dbtoa.c
degrade~.class.sources := binaries_src/signal/degrade.c
downsamp~.class.sources := binaries_src/signal/downsamp.c
filtercoeff~.class.sources := binaries_src/signal/filtercoeff.c
freqshift~.class.sources := binaries_src/signal/freqshift.c
gate~.class.sources := binaries_src/signal/gate.c
hilbert~.class.sources := binaries_src/signal/hilbert.c
phaseshift~.class.sources := binaries_src/signal/phaseshift.c
round~.class.sources := binaries_src/signal/round.c
scale~.class.sources := binaries_src/signal/scale.c
selector~.class.sources := binaries_src/signal/selector.c
thresh~.class.sources := binaries_src/signal/thresh.c
trunc~.class.sources := binaries_src/signal/trunc.c

rminus.class.sources := binaries_src/control/rminus.c
rdiv.class.sources := binaries_src/control/rdiv.c

equals~.class.sources := binaries_src/signal/equals.c
notequals~.class.sources := binaries_src/signal/notequals.c
greaterthan~.class.sources := binaries_src/signal/greaterthan.c
greaterthaneq~.class.sources := binaries_src/signal/greaterthaneq.c
lessthan~.class.sources := binaries_src/signal/lessthan.c
lessthaneq~.class.sources := binaries_src/signal/lessthaneq.c
modulo~.class.sources := binaries_src/signal/modulo.c
rdiv~.class.sources := binaries_src/signal/rdiv.c
rminus~.class.sources := binaries_src/signal/rminus.c


## classes with dependencies #################################################

# Control Classes

hfile := \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/common/fitter.c \
shared/unstable/forky.c

hfitter := \
shared/common/loud.c \
shared/common/fitter.c

hforky := \
shared/common/loud.c \
shared/unstable/forky.c

hfragile := \
shared/common/loud.c \
shared/unstable/fragile.c

hgrow := \
shared/common/grow.c \
shared/common/loud.c

hgrowfitter := \
shared/common/grow.c \
shared/common/loud.c \
shared/common/fitter.c

hgui := \
shared/hammer/gui.c \
shared/common/loud.c

hloud := \
shared/common/loud.c

hrand := \
shared/common/rand.c \
shared/common/loud.c

hrandfile := \
shared/common/rand.c \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/common/fitter.c \
shared/unstable/forky.c

hrandgrow := \
shared/common/rand.c \
shared/common/grow.c \
shared/common/loud.c \
shared/common/fitter.c

hrandgrowfile := \
shared/common/rand.c \
shared/common/grow.c \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/unstable/forky.c

hseq := \
shared/common/mifi.c \
shared/hammer/file.c \
shared/common/grow.c \
shared/common/loud.c \
shared/common/os.c \
shared/common/fitter.c \
shared/unstable/forky.c

htree := \
shared/hammer/tree.c \
shared/common/loud.c

htreefilevefl := \
shared/hammer/tree.c \
shared/hammer/file.c \
shared/common/vefl.c \
shared/common/loud.c \
shared/common/os.c \
shared/unstable/forky.c \
shared/unstable/fragile.c

splainnotilde := \
shared/common/loud.c \
shared/common/fitter.c

# hfitter classes
bangbang.class.sources := binaries_src/control/bangbang.c $(hfitter)
counter.class.sources := binaries_src/control/counter.c $(hfitter)
cycle.class.sources := binaries_src/control/cycle.c $(hfitter)
decode.class.sources := binaries_src/control/decode.c $(hfitter)
gate.class.sources := binaries_src/control/gate.c $(hfitter)
maximum.class.sources := binaries_src/control/maximum.c $(hfitter)
minimum.class.sources := binaries_src/control/minimum.c $(hfitter)
switch.class.sources := binaries_src/control/switch.c $(hfitter)

# hfragile classes
universal.class.sources := binaries_src/control/universal.c $(hfragile)
grab.class.sources := binaries_src/control/grab.c $(hfragile)

# hfile classes
loadmess.class.sources := binaries_src/control/loadmess.c $(hfile) # for "click" (new class)
capture.class.sources := binaries_src/control/capture.c $(hfile)
coll.class.sources := binaries_src/control/coll.c $(hfile)
mtr.class.sources := binaries_src/control/mtr.c $(hfile)

# hgui classes
active.class.sources := binaries_src/control/active.c $(hgui)
mousefilter.class.sources := binaries_src/control/mousefilter.c $(hgui)
mousestate.class.sources := binaries_src/control/mousestate.c $(hgui)

# hgrow classes
bondo.class.sources := binaries_src/control/bondo.c $(hgrow)
buddy.class.sources := binaries_src/control/buddy.c $(hgrow)
clip.class.sources := binaries_src/control/clip.c $(hgrow)
iter.class.sources := binaries_src/control/iter.c $(hgrow)
match.class.sources := binaries_src/control/match.c $(hgrow)
speedlim.class.sources := binaries_src/control/speedlim.c $(hgrow)
substitute.class.sources := binaries_src/control/substitute.c $(hgrow)
thresh.class.sources := binaries_src/control/thresh.c $(hgrow)
tosymbol.class.sources := binaries_src/control/tosymbol.c $(hgrow)
zl.class.sources := binaries_src/control/zl.c $(hgrow)
pv.class.sources := binaries_src/control/pv.c $(hgrow)

# hgrowfitter classes
append.class.sources := binaries_src/control/append.c $(hgrowfitter)
prepend.class.sources := binaries_src/control/prepend.c $(hgrowfitter)
past.class.sources := binaries_src/control/past.c $(hgrowfitter)

# hloud classes
anal.class.sources := binaries_src/control/anal.c $(hloud)
borax.class.sources := binaries_src/control/borax.c $(hloud)
decide.class.sources := binaries_src/control/decide.c $(hloud)
spell.class.sources := binaries_src/control/spell.c $(hloud)
spray.class.sources := binaries_src/control/spray.c $(hloud)
sprintf.class.sources := binaries_src/control/sprintf.c $(hloud)
togedge.class.sources := binaries_src/control/togedge.c $(hloud)
histo.class.sources := binaries_src/control/histo.c $(hloud)

# Single cases:

linedrive.class.sources := binaries_src/control/linedrive.c $(splainnotilde)

comment.class.sources := binaries_src/control/comment.c $(hforky)

drunk.class.sources := binaries_src/control/drunk.c $(hrand)

prob.class.sources := binaries_src/control/prob.c $(hrandfile)

urn.class.sources := binaries_src/control/urn.c $(hrandgrow)

table.class.sources := binaries_src/control/table.c $(hrandgrowfile)

seq.class.sources := binaries_src/control/seq.c $(hseq)

offer.class.sources := binaries_src/control/offer.c $(htree)

funbuff.class.sources := binaries_src/control/funbuff.c $(htreefilevefl)

##############################################################################

# Signal classes:

sfragileforkyloud := \
shared/common/loud.c \
shared/unstable/fragile.c \
shared/unstable/forky.c
    cartopol~.class.sources := binaries_src/signal/cartopol.c $(sfragileforkyloud)

# loud? - get rid of loud and get fragile and forky into the same one, then agroup with the classes below

sforky := \
shared/unstable/forky.c
    bitand~.class.sources := binaries_src/signal/bitand.c $(sforky)
    bitor~.class.sources := binaries_src/signal/bitor.c $(sforky)
    bitxor~.class.sources := binaries_src/signal/bitxor.c $(sforky)
    plusequals~.class.sources := binaries_src/signal/plusequals.c $(sforky)
    cyclone.class.sources := binaries_src/sub_lib_cyclone.c $(sforky)
    minmax~.class.sources := binaries_src/signal/minmax.c $(sforky)
    poltocar~.class.sources := binaries_src/signal/poltocar.c $(sforky)
    matrix~.class.sources := binaries_src/signal/matrix.c $(sforky)

sgrowclc := \
shared/common/grow.c \
shared/common/clc.c \
shared/common/loud.c
    frameaccum~.class.sources := binaries_src/signal/frameaccum.c $(sgrowclc)
    framedelta~.class.sources := binaries_src/signal/framedelta.c $(sgrowclc)
    line~.class.sources := binaries_src/signal/line.c $(sgrowclc)
    curve~.class.sources := binaries_src/signal/curve.c $(sgrowclc) # only one with clc (agrouped here)

sgrowforky := \
shared/common/grow.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/forky.c
    scope~.class.sources := binaries_src/signal/scope.c $(sgrowforky)

sfile := \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/unstable/forky.c
    capture~.class.sources := binaries_src/signal/capture.c $(sfile)

# Buffer Classes (agrouped) - still "sic-fied"
sarsic := \
shared/sickle/sic.c \
shared/sickle/arsic.c \
shared/common/loud.c \
shared/common/vefl.c \
shared/common/fitter.c \
shared/unstable/fragile.c
    peek~.class.sources := binaries_src/signal/peek.c $(sarsic)
    play~.class.sources := binaries_src/signal/play.c $(sarsic)
    poke~.class.sources := binaries_src/signal/poke.c $(sarsic) # only using fragile
    record~.class.sources := binaries_src/signal/record.c $(sarsic)
    wave~.class.sources := binaries_src/signal/wave.c $(sarsic)

#cybuf (aka arsic replacement) classes
scybuf := shared/cybuf.c
    index~.class.sources := binaries_src/signal/index.c $(scybuf)
    buffir~.class.sources := binaries_src/signal/buffir.c $(scybuf)

#######################################################################
### CYCLONE ###     ### CYCLONE ### ### CYCLONE ###     ### CYCLONE ###
### CLASSES ###     ### CLASSES ### ### CLASSES ###     ### CLASSES ###
#######################################################################

                        ## END OF CYCLONE CLASSES ##

#######################################################################

datafiles = \
$(wildcard help/*-help.pd) \
help/output~.pd \
help/test.mid \
help/voice.wav \
LICENSE.txt \
README.md \
cyclone-meta.pd \

# pthreadGC2.dll is required for Windows installation. It can be found in
# the MinGW directory (usually C:\MinGW\bin) directory and should be
# copied to the current directory before installation or packaging.

ifeq (MINGW,$(findstring MINGW,$(uname)))
datafiles += pthreadGC2.dll
endif

################################################################################
### pd-lib-builder #############################################################
################################################################################

include pd-lib-builder/Makefile.pdlibbuilder

################################################################################
### Install UPPER case aliases for Linux #######################################
################################################################################

install: install-aliases
install-aliases: all
ifeq ($(uname), Linux)
	$(INSTALL_DIR) -v $(installpath)
	cd $(installpath); \
	ln -s -f append.$(extension) Append.$(extension); \
	ln -s -f append-help.pd Append-help.pd; \
	ln -s -f borax.$(extension) Borax.$(extension); \
	ln -s -f borax-help.pd Borax-help.pd; \
	ln -s -f bucket.$(extension) Bucket.$(extension); \
	ln -s -f bucket-help.pd Bucket-help.pd; \
	ln -s -f clip.$(extension) Clip.$(extension); \
	ln -s -f clip-help.pd Clip-help.pd; \
	ln -s -f decode.$(extension) Decode.$(extension); \
	ln -s -f decode-help.pd Decode-help.pd; \
	ln -s -f histo.$(extension) Histo.$(extension); \
	ln -s -f histo-help.pd Histo-help.pd; \
	ln -s -f mousestate.$(extension) MouseState.$(extension); \
	ln -s -f mousestate-help.pd MouseState-help.pd; \
	ln -s -f peak.$(extension) Peak.$(extension); \
	ln -s -f peak-help.pd Peak-help.pd; \
	ln -s -f table.$(extension) Table.$(extension); \
	ln -s -f table-help.pd Table-help.pd; \
	ln -s -f togedge.$(extension) TogEdge.$(extension); \
	ln -s -f togedge-help.pd TogEdge-help.pd; \
	ln -s -f trough.$(extension) Trough.$(extension); \
	ln -s -f trough-help.pd Trough-help.pd; \
	ln -s -f uzi.$(extension) Uzi.$(extension); \
	ln -s -f uzi-help.pd Uzi-help.pd; \
	ln -s -f clip~.$(extension) Clip~.$(extension); \
	ln -s -f clip~-help.pd Clip~-help.pd; \
	ln -s -f line~.$(extension) Line~.$(extension); \
	ln -s -f line~-help.pd Line~-help.pd; \
	ln -s -f scope~.$(extension) Scope~.$(extension); \
	ln -s -f scope~-help.pd Scope~-help.pd; \
	ln -s -f snapshot~.$(extension) Snapshot~.$(extension); \
	ln -s -f snapshot~-help.pd Snapshot~-help.pd
endif
