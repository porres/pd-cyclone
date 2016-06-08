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
### CYCLONE ###     ### CYCLONE ### ### CYCLONE ###     ### CYCLONE ###
### CLASSES ###     ### CLASSES ### ### CLASSES ###     ### CLASSES ###
#######################################################################


##############################################################
########### classes outside old dependencies #################
##############################################################

# CONTROL CLASSES Removed from old dependencies

# these used to end in $(hplain) which seemed
# to be doing nothing in the new build system:

accum.class.sources := classes/binaries/control/accum.c
acos.class.sources := classes/binaries/control/acos.c
asin.class.sources := classes/binaries/control/asin.c
bucket.class.sources := classes/binaries/control/bucket.c
cartopol.class.sources := classes/binaries/control/cartopol.c
cosh.class.sources := classes/binaries/control/cosh.c
flush.class.sources := classes/binaries/control/flush.c
forward.class.sources := classes/binaries/control/forward.c
fromsymbol.class.sources := classes/binaries/control/fromsymbol.c
mean.class.sources := classes/binaries/control/mean.c
midiflush.class.sources := classes/binaries/control/midiflush.c
midiformat.class.sources := classes/binaries/control/midiformat.c
midiparse.class.sources := classes/binaries/control/midiparse.c
next.class.sources := classes/binaries/control/next.c
onebang.class.sources := classes/binaries/control/onebang.c
peak.class.sources := classes/binaries/control/peak.c
poltocar.class.sources := classes/binaries/control/poltocar.c
sinh.class.sources := classes/binaries/control/sinh.c
split.class.sources := classes/binaries/control/split.c
sustain.class.sources := classes/binaries/control/sustain.c
tanh.class.sources := classes/binaries/control/tanh.c
trough.class.sources := classes/binaries/control/trough.c
uzi.class.sources := classes/binaries/control/uzi.c
xbendin.class.sources := classes/binaries/control/xbendin.c
xbendin2.class.sources := classes/binaries/control/xbendin2.c
xbendout.class.sources := classes/binaries/control/xbendout.c
xbendout2.class.sources := classes/binaries/control/xbendout2.c
xnotein.class.sources := classes/binaries/control/xnotein.c
xnoteout.class.sources := classes/binaries/control/xnoteout.c

# SIGNAL CLASSES Removed from old framework (mostly "ssic"):

abs~.class.sources := classes/binaries/signal/abs.c
acos~.class.sources := classes/binaries/signal/acos.c
acosh~.class.sources := classes/binaries/signal/acosh.c
allpass~.class.sources := classes/binaries/signal/allpass.c
asinh~.class.sources := classes/binaries/signal/asinh.c
asin~.class.sources := classes/binaries/signal/asin.c
atan~.class.sources := classes/binaries/signal/atan.c
atan2~.class.sources := classes/binaries/signal/atan2.c
atanh~.class.sources := classes/binaries/signal/atanh.c
average~.class.sources := classes/binaries/signal/average.c
avg~.class.sources := classes/binaries/signal/avg.c
bitnot~.class.sources := classes/binaries/signal/bitnot.c
bitshift~.class.sources := classes/binaries/signal/bitshift.c
change~.class.sources := classes/binaries/signal/change.c
click~.class.sources := classes/binaries/signal/click.c
clip~.class.sources := classes/binaries/signal/clip.c
cosh~.class.sources := classes/binaries/signal/cosh.c
cosx~.class.sources := classes/binaries/signal/cosx.c
count~.class.sources := classes/binaries/signal/count.c
comb~.class.sources := classes/binaries/signal/comb.c
delay~.class.sources := classes/binaries/signal/delay.c
delta~.class.sources := classes/binaries/signal/delta.c
deltaclip~.class.sources := classes/binaries/signal/deltaclip.c
edge~.class.sources := classes/binaries/signal/edge.c
kink~.class.sources := classes/binaries/signal/kink.c
log~.class.sources := classes/binaries/signal/log.c
lores~.class.sources := classes/binaries/signal/lores.c
matrix~.class.sources := classes/binaries/signal/matrix.c
maximum~.class.sources := classes/binaries/signal/maximum.c
minimum~.class.sources := classes/binaries/signal/minimum.c
minmax~.class.sources := classes/binaries/signal/minmax.c
mstosamps~.class.sources := classes/binaries/signal/mstosamps.c
onepole~.class.sources := classes/binaries/signal/onepole.c
overdrive~.class.sources := classes/binaries/signal/overdrive.c
peakamp~.class.sources := classes/binaries/signal/peakamp.c
phaseshift~.class.sources := classes/binaries/signal/phaseshift.c
phasewrap~.class.sources := classes/binaries/signal/phasewrap.c
pink~.class.sources := classes/binaries/signal/pink.c
poltocar~.class.sources := classes/binaries/signal/poltocar.c
pong~.class.sources := classes/binaries/signal/pong.c
pow~.class.sources := classes/binaries/signal/pow.c
rampsmooth~.class.sources := classes/binaries/signal/rampsmooth.c
rand~.class.sources := classes/binaries/signal/rand.c
reson~.class.sources := classes/binaries/signal/reson.c
sah~.class.sources := classes/binaries/signal/sah.c
sampstoms~.class.sources := classes/binaries/signal/sampstoms.c
sinh~.class.sources := classes/binaries/signal/sinh.c
sinx~.class.sources := classes/binaries/signal/sinx.c
slide~.class.sources := classes/binaries/signal/slide.c
snapshot~.class.sources := classes/binaries/signal/snapshot.c
spike~.class.sources := classes/binaries/signal/spike.c
svf~.class.sources := classes/binaries/signal/svf.c
tanh~.class.sources := classes/binaries/signal/tanh.c
tanx~.class.sources := classes/binaries/signal/tanx.c
train~.class.sources := classes/binaries/signal/train.c
trapezoid~.class.sources := classes/binaries/signal/trapezoid.c
triangle~.class.sources := classes/binaries/signal/triangle.c
vectral~.class.sources := classes/binaries/signal/vectral.c
zerox~.class.sources := classes/binaries/signal/zerox.c

# Cyclone sub library (with non-alphanumeric objects): #

cyclone.class.sources := classes/cyclone_lib/cyclone.c

# New objects with alphanumeric versions

# control
rminus.class.sources := classes/binaries/control/rminus.c
rdiv.class.sources := classes/binaries/control/rdiv.c

# signal
equals~.class.sources := classes/binaries/signal/equals.c
notequals~.class.sources := classes/binaries/signal/notequals.c
plusequals~.class.sources := classes/binaries/signal/plusequals.c
greaterthan~.class.sources := classes/binaries/signal/greaterthan.c
greaterthaneq~.class.sources := classes/binaries/signal/greaterthaneq.c
lessthan~.class.sources := classes/binaries/signal/lessthan.c
lessthaneq~.class.sources := classes/binaries/signal/lessthaneq.c
modulo~.class.sources := classes/binaries/signal/modulo.c
rdiv~.class.sources := classes/binaries/signal/rdiv.c
rminus~.class.sources := classes/binaries/signal/rminus.c

# Other NEW CLASSES:

# Control
acosh.class.sources := classes/binaries/control/acosh.c
asinh.class.sources := classes/binaries/control/asinh.c
atanh.class.sources := classes/binaries/control/atanh.c
atodb.class.sources := classes/binaries/control/atodb.c
dbtoa.class.sources := classes/binaries/control/dbtoa.c
join.class.sources := classes/binaries/control/join.c
pong.class.sources := classes/binaries/control/pong.c
pak.class.sources := classes/binaries/control/pak.c
round.class.sources := classes/binaries/control/round.c
scale.class.sources := classes/binaries/control/scale.c

# Signal
atodb~.class.sources := classes/binaries/signal/atodb.c
biquad~.class.sources := classes/binaries/signal/biquad.c
bitsafe~.class.sources := classes/binaries/signal/bitsafe.c
dbtoa~.class.sources := classes/binaries/signal/dbtoa.c
downsamp~.class.sources := classes/binaries/signal/downsamp.c
gate~.class.sources := classes/binaries/signal/gate.c
round~.class.sources := classes/binaries/signal/round.c
scale~.class.sources := classes/binaries/signal/scale.c
selector~.class.sources := classes/binaries/signal/selector.c
thresh~.class.sources := classes/binaries/signal/thresh.c
trunc~.class.sources := classes/binaries/signal/trunc.c

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

# Control Classes:

# hfitter classes
bangbang.class.sources := classes/binaries/control/bangbang.c $(hfitter)
counter.class.sources := classes/binaries/control/counter.c $(hfitter)
cycle.class.sources := classes/binaries/control/cycle.c $(hfitter)
decode.class.sources := classes/binaries/control/decode.c $(hfitter)
gate.class.sources := classes/binaries/control/gate.c $(hfitter)
maximum.class.sources := classes/binaries/control/maximum.c $(hfitter)
minimum.class.sources := classes/binaries/control/minimum.c $(hfitter)
switch.class.sources := classes/binaries/control/switch.c $(hfitter)

# hfragile classes
testmess.class.sources := classes/binaries/control/testmess.c $(hfragile)
universal.class.sources := classes/binaries/control/universal.c $(hfragile)
grab.class.sources := classes/binaries/control/grab.c $(hfragile)

# hfile classes
loadmess.class.sources := classes/binaries/control/loadmess.c $(hfile) # for "click" (new class)
capture.class.sources := classes/binaries/control/capture.c $(hfile)
coll.class.sources := classes/binaries/control/coll.c $(hfile)
mtr.class.sources := classes/binaries/control/mtr.c $(hfile)

# hgui classes
active.class.sources := classes/binaries/control/active.c $(hgui)
mousefilter.class.sources := classes/binaries/control/mousefilter.c $(hgui)
mousestate.class.sources := classes/binaries/control/mousestate.c $(hgui)

# hgrow classes
bondo.class.sources := classes/binaries/control/bondo.c $(hgrow)
buddy.class.sources := classes/binaries/control/buddy.c $(hgrow)
clip.class.sources := classes/binaries/control/clip.c $(hgrow)
funnel.class.sources := classes/binaries/control/funnel.c $(hgrow)
iter.class.sources := classes/binaries/control/iter.c $(hgrow)
match.class.sources := classes/binaries/control/match.c $(hgrow)
speedlim.class.sources := classes/binaries/control/speedlim.c $(hgrow)
substitute.class.sources := classes/binaries/control/substitute.c $(hgrow)
thresh.class.sources := classes/binaries/control/thresh.c $(hgrow)
tosymbol.class.sources := classes/binaries/control/tosymbol.c $(hgrow)
zl.class.sources := classes/binaries/control/zl.c $(hgrow)
pv.class.sources := classes/binaries/control/pv.c $(hgrow)

# hgrowfitter classes
append.class.sources := classes/binaries/control/append.c $(hgrowfitter)
prepend.class.sources := classes/binaries/control/prepend.c $(hgrowfitter)
past.class.sources := classes/binaries/control/past.c $(hgrowfitter)

# hloud classes
anal.class.sources := classes/binaries/control/anal.c $(hloud)
borax.class.sources := classes/binaries/control/borax.c $(hloud)
decide.class.sources := classes/binaries/control/decide.c $(hloud)
spell.class.sources := classes/binaries/control/spell.c $(hloud)
spray.class.sources := classes/binaries/control/spray.c $(hloud)
sprintf.class.sources := classes/binaries/control/sprintf.c $(hloud)
togedge.class.sources := classes/binaries/control/togedge.c $(hloud)
histo.class.sources := classes/binaries/control/histo.c $(hloud)

# special case: sickle but not tilde (see class linedrive)
splainnotilde := \
shared/common/loud.c \
shared/common/fitter.c

# Single cases:

linedrive.class.sources := classes/binaries/control/linedrive.c $(splainnotilde)

comment.class.sources := classes/binaries/control/comment.c $(hforky)

drunk.class.sources := classes/binaries/control/drunk.c $(hrand)

prob.class.sources := classes/binaries/control/prob.c $(hrandfile)

urn.class.sources := classes/binaries/control/urn.c $(hrandgrow)

table.class.sources := classes/binaries/control/table.c $(hrandgrowfile)

seq.class.sources := classes/binaries/control/seq.c $(hseq)

offer.class.sources := classes/binaries/control/offer.c $(htree)

funbuff.class.sources := classes/binaries/control/funbuff.c $(htreefilevefl)

###################
# Signal classes: #
###################

sfragile := \
shared/common/loud.c \
shared/unstable/fragile.c
cartopol~.class.sources := classes/binaries/signal/cartopol.c $(sfragile)

sforky := \
shared/unstable/forky.c
bitand~.class.sources := classes/binaries/signal/bitand.c $(sforky)
bitor~.class.sources := classes/binaries/signal/bitor.c $(sforky)
bitxor~.class.sources := classes/binaries/signal/bitxor.c $(sforky)

sgrowclc := \
shared/common/grow.c \
shared/common/clc.c \
shared/common/loud.c
frameaccum~.class.sources := classes/binaries/signal/frameaccum.c $(sgrowclc)
framedelta~.class.sources := classes/binaries/signal/framedelta.c $(sgrowclc)
line~.class.sources := classes/binaries/signal/line.c $(sgrowclc)
curve~.class.sources := classes/binaries/signal/curve.c $(sgrowclc) # only one with clc (agrouped here)

sgrowforky := \
shared/common/grow.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/forky.c
scope~.class.sources := classes/binaries/signal/scope.c $(sgrowforky)

sfile := \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/unstable/forky.c
capture~.class.sources := classes/binaries/signal/capture.c $(sfile)

# Buffer Classes (agrouped) - still "sic-fied"
sarsicfittervefl := \
shared/sickle/sic.c \
shared/sickle/arsic.c \
shared/common/vefl.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/fragile.c
buffir~.class.sources := classes/binaries/signal/buffir.c $(sarsicfittervefl) # was 'sarsicfitter'
# partially de-sic-fied:
cycle~.class.sources := classes/binaries/signal/cycle.c $(sarsicfittervefl) # was 'svefl'
# remaining ones below were 'sarsic':
index~.class.sources := classes/binaries/signal/index.c $(sarsicfittervefl)
lookup~.class.sources := classes/binaries/signal/lookup.c $(sarsicfittervefl)
peek~.class.sources := classes/binaries/signal/peek.c $(sarsicfittervefl)
play~.class.sources := classes/binaries/signal/play.c $(sarsicfittervefl)
poke~.class.sources := classes/binaries/signal/poke.c $(sarsicfittervefl)
record~.class.sources := classes/binaries/signal/record.c $(sarsicfittervefl)
wave~.class.sources := classes/binaries/signal/wave.c $(sarsicfittervefl)


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
