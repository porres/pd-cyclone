################################################################################
######## Makefile for the Cyclone library of Pure Data externals ###############
################################################################################

lib.name = cyclone

# for the MINGW which has the timespec struct defined twice
cflags = -Ishared -DHAVE_STRUCT_TIMESPEC

uname := $(shell uname -s)
ifeq (MINGW,$(findstring MINGW,$(uname)))
ldlibs += -lpthread
endif

#######################################################################

                    ## START OF CYCLONE CLASSES ##

#######################################################################

# CONTROL CLASSES
accum.class.sources := src/control/accum.c
acos.class.sources := src/control/acos.c
asin.class.sources := src/control/asin.c
bucket.class.sources := src/control/bucket.c
cartopol.class.sources := src/control/cartopol.c
cosh.class.sources := src/control/cosh.c
flush.class.sources := src/control/flush.c
forward.class.sources := src/control/forward.c
fromsymbol.class.sources := src/control/fromsymbol.c
mean.class.sources := src/control/mean.c
midiflush.class.sources := src/control/midiflush.c
midiformat.class.sources := src/control/midiformat.c
midiparse.class.sources := src/control/midiparse.c
next.class.sources := src/control/next.c
onebang.class.sources := src/control/onebang.c
peak.class.sources := src/control/peak.c
poltocar.class.sources := src/control/poltocar.c
sinh.class.sources := src/control/sinh.c
split.class.sources := src/control/split.c
sustain.class.sources := src/control/sustain.c
tanh.class.sources := src/control/tanh.c
trough.class.sources := src/control/trough.c
uzi.class.sources := src/control/uzi.c
xbendin.class.sources := src/control/xbendin.c
xbendin2.class.sources := src/control/xbendin2.c
xbendout.class.sources := src/control/xbendout.c
xbendout2.class.sources := src/control/xbendout2.c
xnotein.class.sources := src/control/xnotein.c
xnoteout.class.sources := src/control/xnoteout.c
# NEW ones:
acosh.class.sources := src/control/acosh.c
asinh.class.sources := src/control/asinh.c
atanh.class.sources := src/control/atanh.c
atodb.class.sources := src/control/atodb.c
dbtoa.class.sources := src/control/dbtoa.c
join.class.sources := src/control/join.c
pong.class.sources := src/control/pong.c
pak.class.sources := src/control/pak.c
round.class.sources := src/control/round.c
scale.class.sources := src/control/scale.c

# SIGNAL CLASSES:
abs~.class.sources := src/signal/abs.c
acos~.class.sources := src/signal/acos.c
acosh~.class.sources := src/signal/acosh.c
allpass~.class.sources := src/signal/allpass.c
asinh~.class.sources := src/signal/asinh.c
asin~.class.sources := src/signal/asin.c
atan~.class.sources := src/signal/atan.c
atan2~.class.sources := src/signal/atan2.c
atanh~.class.sources := src/signal/atanh.c
average~.class.sources := src/signal/average.c
avg~.class.sources := src/signal/avg.c
bitnot~.class.sources := src/signal/bitnot.c
bitshift~.class.sources := src/signal/bitshift.c
cascade~.class.sources := src/signal/cascade.c
change~.class.sources := src/signal/change.c
click~.class.sources := src/signal/click.c
clip~.class.sources := src/signal/clip.c
cosh~.class.sources := src/signal/cosh.c
cosx~.class.sources := src/signal/cosx.c
count~.class.sources := src/signal/count.c
comb~.class.sources := src/signal/comb.c
cycle~.class.sources := src/signal/cycle.c
delay~.class.sources := src/signal/delay.c
delta~.class.sources := src/signal/delta.c
deltaclip~.class.sources := src/signal/deltaclip.c
edge~.class.sources := src/signal/edge.c
kink~.class.sources := src/signal/kink.c
log~.class.sources := src/signal/log.c
lookup~.class.sources := src/signal/lookup.c
lores~.class.sources := src/signal/lores.c
matrix~.class.sources := src/signal/matrix.c
maximum~.class.sources := src/signal/maximum.c
minimum~.class.sources := src/signal/minimum.c
minmax~.class.sources := src/signal/minmax.c
mstosamps~.class.sources := src/signal/mstosamps.c
onepole~.class.sources := src/signal/onepole.c
overdrive~.class.sources := src/signal/overdrive.c
peakamp~.class.sources := src/signal/peakamp.c
phasewrap~.class.sources := src/signal/phasewrap.c
pink~.class.sources := src/signal/pink.c
poltocar~.class.sources := src/signal/poltocar.c
pong~.class.sources := src/signal/pong.c
pow~.class.sources := src/signal/pow.c
rampsmooth~.class.sources := src/signal/rampsmooth.c
rand~.class.sources := src/signal/rand.c
reson~.class.sources := src/signal/reson.c
sah~.class.sources := src/signal/sah.c
sampstoms~.class.sources := src/signal/sampstoms.c
sinh~.class.sources := src/signal/sinh.c
sinx~.class.sources := src/signal/sinx.c
slide~.class.sources := src/signal/slide.c
snapshot~.class.sources := src/signal/snapshot.c
spike~.class.sources := src/signal/spike.c
svf~.class.sources := src/signal/svf.c
tanh~.class.sources := src/signal/tanh.c
tanx~.class.sources := src/signal/tanx.c
train~.class.sources := src/signal/train.c
trapezoid~.class.sources := src/signal/trapezoid.c
triangle~.class.sources := src/signal/triangle.c
vectral~.class.sources := src/signal/vectral.c
zerox~.class.sources := src/signal/zerox.c
# NEW ones:
atodb~.class.sources := src/signal/atodb.c
biquad~.class.sources := src/signal/biquad.c
bitsafe~.class.sources := src/signal/bitsafe.c
cross~.class.sources := src/signal/cross.c
dbtoa~.class.sources := src/signal/dbtoa.c
degrade~.class.sources := src/signal/degrade.c
downsamp~.class.sources := src/signal/downsamp.c
filtercoeff~.class.sources := src/signal/filtercoeff.c
freqshift~.class.sources := src/signal/freqshift.c
gate~.class.sources := src/signal/gate.c
hilbert~.class.sources := src/signal/hilbert.c
phaseshift~.class.sources := src/signal/phaseshift.c
round~.class.sources := src/signal/round.c
scale~.class.sources := src/signal/scale.c
selector~.class.sources := src/signal/selector.c
thresh~.class.sources := src/signal/thresh.c
trunc~.class.sources := src/signal/trunc.c

# CYCLONE SUB LIBRARY (with non-alphanumeric objects): #
cyclone.class.sources := src/sub_lib_cyclone.c

# New objects with alphanumeric versions
rminus.class.sources := src/control/rminus.c
rdiv.class.sources := src/control/rdiv.c
equals~.class.sources := src/signal/equals.c
notequals~.class.sources := src/signal/notequals.c
plusequals~.class.sources := src/signal/plusequals.c
greaterthan~.class.sources := src/signal/greaterthan.c
greaterthaneq~.class.sources := src/signal/greaterthaneq.c
lessthan~.class.sources := src/signal/lessthan.c
lessthaneq~.class.sources := src/signal/lessthaneq.c
modulo~.class.sources := src/signal/modulo.c
rdiv~.class.sources := src/signal/rdiv.c
rminus~.class.sources := src/signal/rminus.c


############################################################
## classes still inside the old framework of dependencies ##
############################################################

#######################
### Control objects ###
#######################

# dependencies:

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

# Control Classes:

# hfitter classes
bangbang.class.sources := src/control/bangbang.c $(hfitter)
counter.class.sources := src/control/counter.c $(hfitter)
cycle.class.sources := src/control/cycle.c $(hfitter)
decode.class.sources := src/control/decode.c $(hfitter)
gate.class.sources := src/control/gate.c $(hfitter)
maximum.class.sources := src/control/maximum.c $(hfitter)
minimum.class.sources := src/control/minimum.c $(hfitter)
switch.class.sources := src/control/switch.c $(hfitter)

# hfragile classes
universal.class.sources := src/control/universal.c $(hfragile)
grab.class.sources := src/control/grab.c $(hfragile)

# hfile classes
loadmess.class.sources := src/control/loadmess.c $(hfile) # for "click" (new class)
capture.class.sources := src/control/capture.c $(hfile)
coll.class.sources := src/control/coll.c $(hfile)
mtr.class.sources := src/control/mtr.c $(hfile)

# hgui classes
active.class.sources := src/control/active.c $(hgui)
mousefilter.class.sources := src/control/mousefilter.c $(hgui)
mousestate.class.sources := src/control/mousestate.c $(hgui)

# hgrow classes
bondo.class.sources := src/control/bondo.c $(hgrow)
buddy.class.sources := src/control/buddy.c $(hgrow)
clip.class.sources := src/control/clip.c $(hgrow)
funnel.class.sources := src/control/funnel.c $(hgrow)
iter.class.sources := src/control/iter.c $(hgrow)
match.class.sources := src/control/match.c $(hgrow)
speedlim.class.sources := src/control/speedlim.c $(hgrow)
substitute.class.sources := src/control/substitute.c $(hgrow)
thresh.class.sources := src/control/thresh.c $(hgrow)
tosymbol.class.sources := src/control/tosymbol.c $(hgrow)
zl.class.sources := src/control/zl.c $(hgrow)
pv.class.sources := src/control/pv.c $(hgrow)

# hgrowfitter classes
append.class.sources := src/control/append.c $(hgrowfitter)
prepend.class.sources := src/control/prepend.c $(hgrowfitter)
past.class.sources := src/control/past.c $(hgrowfitter)

# hloud classes
anal.class.sources := src/control/anal.c $(hloud)
borax.class.sources := src/control/borax.c $(hloud)
decide.class.sources := src/control/decide.c $(hloud)
spell.class.sources := src/control/spell.c $(hloud)
spray.class.sources := src/control/spray.c $(hloud)
sprintf.class.sources := src/control/sprintf.c $(hloud)
togedge.class.sources := src/control/togedge.c $(hloud)
histo.class.sources := src/control/histo.c $(hloud)

# Single cases:

linedrive.class.sources := src/control/linedrive.c $(splainnotilde)

comment.class.sources := src/control/comment.c $(hforky)

drunk.class.sources := src/control/drunk.c $(hrand)

prob.class.sources := src/control/prob.c $(hrandfile)

urn.class.sources := src/control/urn.c $(hrandgrow)

table.class.sources := src/control/table.c $(hrandgrowfile)

seq.class.sources := src/control/seq.c $(hseq)

offer.class.sources := src/control/offer.c $(htree)

funbuff.class.sources := src/control/funbuff.c $(htreefilevefl)

###################
# Signal classes: #
###################

sfragile := \
shared/common/loud.c \
shared/unstable/fragile.c
    cartopol~.class.sources := src/signal/cartopol.c $(sfragile)

sforky := \
shared/unstable/forky.c
    bitand~.class.sources := src/signal/bitand.c $(sforky)
    bitor~.class.sources := src/signal/bitor.c $(sforky)
    bitxor~.class.sources := src/signal/bitxor.c $(sforky)

sgrowclc := \
shared/common/grow.c \
shared/common/clc.c \
shared/common/loud.c
    frameaccum~.class.sources := src/signal/frameaccum.c $(sgrowclc)
    framedelta~.class.sources := src/signal/framedelta.c $(sgrowclc)
    line~.class.sources := src/signal/line.c $(sgrowclc)
    curve~.class.sources := src/signal/curve.c $(sgrowclc) # only one with clc (agrouped here)

sgrowforky := \
shared/common/grow.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/forky.c
    scope~.class.sources := src/signal/scope.c $(sgrowforky)

sfile := \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/unstable/forky.c
    capture~.class.sources := src/signal/capture.c $(sfile)

# Buffer Classes (agrouped) - still "sic-fied"
sarsicfitter := \
shared/sickle/sic.c \
shared/sickle/arsic.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/fragile.c
# were 'sarsic':
    peek~.class.sources := src/signal/peek.c $(sarsicfitter)
    play~.class.sources := src/signal/play.c $(sarsicfitter)
    poke~.class.sources := src/signal/poke.c $(sarsicfitter) # fragile
    record~.class.sources := src/signal/record.c $(sarsicfitter)
    wave~.class.sources := src/signal/wave.c $(sarsicfitter)
# was 'sarsicfitter' (so only one with fitter)
    buffir~.class.sources := src/signal/buffir.c $(sarsicfitter)

#cybuf (aka arsic replacement) classes
scybuf := shared/cybuf.c
    index~.class.sources := src/signal/index.c $(scybuf)

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
$(wildcard abstractions/*.pd)

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
