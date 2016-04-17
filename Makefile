# Makefile for pure data externals in lib cyclone

lib.name = cyclone

# for the MINGW which has the timespec struct defined twice
cflags = -Ishared -DHAVE_STRUCT_TIMESPEC

uname := $(shell uname -s)
ifeq (MINGW,$(findstring MINGW,$(uname)))
    ldlibs += -lpthread
endif


################################################################################
######## new/updated classes & the old framework common functions ##############
################################################################################

# NEW CLASSES - that don'tuse the old framework common functions

    # Max (or "hammer") classes

    atodb.class.sources := classes/binaries/control/atodb.c
    dbtoa.class.sources := classes/binaries/control/dbtoa.c
    pong.class.sources := classes/binaries/control/pong.c
    scale.class.sources := classes/binaries/control/scale.c
    round.class.sources := classes/binaries/control/round.c
    acosh.class.sources := classes/binaries/control/acosh.c
    asinh.class.sources := classes/binaries/control/asinh.c
    atanh.class.sources := classes/binaries/control/atanh.c

    # MSP (or "sickle") classes

    atodb~.class.sources := classes/binaries/signal/atodb.c
    biquad~.class.sources := classes/binaries/signal/biquad.c
    dbtoa~.class.sources := classes/binaries/signal/dbtoa.c
    round~.class.sources := classes/binaries/signal/round.c
    trunc~.class.sources := classes/binaries/signal/trunc.c

# UPDATED CLASSES that were Removed from old framework:

    count~.class.sources := classes/binaries/signal/count.c
    curve~.class.sources := classes/binaries/signal/curve.c
    deltaclip~.class.sources := classes/binaries/signal/deltaclip.c
    pong~.class.sources := classes/binaries/signal/pong.c
    snapshot~.class.sources := classes/binaries/signal/snapshot.c
    trapezoid~.class.sources := classes/binaries/signal/trapezoid.c
    triangle~.class.sources := classes/binaries/signal/triangle.c

# New classes still in the old framework

    thresh~.class.sources := classes/binaries/signal/thresh.c $(ssic)
    loadmess.class.sources := classes/binaries/control/loadmess.c $(hfile)

################################################################################
################################################################################

# hplain: olde classes thatused to end in $(hplain)
# which seemed to be doing nothing in the new build system:

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
rdiv.class.sources := classes/binaries/control/rdiv.c
rminus.class.sources := classes/binaries/control/rminus.c
sinh.class.sources := classes/binaries/control/sinh.c
split.class.sources := classes/binaries/control/split.c
sustain.class.sources := classes/binaries/control/sustain.c
tanh.class.sources := classes/binaries/control/tanh.c
trough.class.sources := classes/binaries/control/trough.c
uzi.class.sources := classes/binaries/control/uzi.c
xbendin.class.sources := classes/binaries/control/xbendin.c
xbendout.class.sources := classes/binaries/control/xbendout.c
xnotein.class.sources := classes/binaries/control/xnotein.c
xnoteout.class.sources := classes/binaries/control/xnoteout.c

# Obsolete

# xbendin2.class.sources := classes/binaries/control/xbendin2.c
# xbendout2.class.sources := classes/binaries/control/xbendout2.c


################################################################################
### Control (or "hammer") objects ##############################################
################################################################################

# common sources for control/Max (or "hammer") classes

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

# Control/Max classes (or "hammer classes")

# The old build system also allowed these classes to be compiled into the "hammer library"
# Since cyclone version alpha57, this new build system only allows single binaries

active.class.sources := classes/binaries/control/active.c $(hgui)
anal.class.sources := classes/binaries/control/anal.c $(hloud)
append.class.sources := classes/binaries/control/append.c $(hgrowfitter)
bangbang.class.sources := classes/binaries/control/bangbang.c $(hfitter)
bondo.class.sources := classes/binaries/control/bondo.c $(hgrow)
borax.class.sources := classes/binaries/control/borax.c $(hloud)
buddy.class.sources := classes/binaries/control/buddy.c $(hgrow)
capture.class.sources := classes/binaries/control/capture.c $(hfile)
clip.class.sources := classes/binaries/control/clip.c $(hgrow)
coll.class.sources := classes/binaries/control/coll.c $(hfile)
comment.class.sources := classes/binaries/control/comment.c $(hforky)
counter.class.sources := classes/binaries/control/counter.c $(hfitter)
cycle.class.sources := classes/binaries/control/cycle.c $(hfitter)
decide.class.sources := classes/binaries/control/decide.c $(hloud)
decode.class.sources := classes/binaries/control/decode.c $(hfitter)
drunk.class.sources := classes/binaries/control/drunk.c $(hrand)
funbuff.class.sources := classes/binaries/control/funbuff.c $(htreefilevefl)
funnel.class.sources := classes/binaries/control/funnel.c $(hgrow)
gate.class.sources := classes/binaries/control/gate.c $(hfitter)
grab.class.sources := classes/binaries/control/grab.c $(hfragile)
histo.class.sources := classes/binaries/control/histo.c $(hloud)
iter.class.sources := classes/binaries/control/iter.c $(hgrow)
match.class.sources := classes/binaries/control/match.c $(hgrow)
maximum.class.sources := classes/binaries/control/maximum.c $(hfitter)
minimum.class.sources := classes/binaries/control/minimum.c $(hfitter)
mousefilter.class.sources := classes/binaries/control/mousefilter.c $(hgui)
mousestate.class.sources := classes/binaries/control/mousestate.c $(hgui)
mtr.class.sources := classes/binaries/control/mtr.c $(hfile)
offer.class.sources := classes/binaries/control/offer.c $(htree)
past.class.sources := classes/binaries/control/past.c $(hgrowfitter)
prepend.class.sources := classes/binaries/control/prepend.c $(hgrowfitter)
prob.class.sources := classes/binaries/control/prob.c $(hrandfile)
pv.class.sources := classes/binaries/control/pv.c $(hgrow)
seq.class.sources := classes/binaries/control/seq.c $(hseq)
speedlim.class.sources := classes/binaries/control/speedlim.c $(hgrow)
spell.class.sources := classes/binaries/control/spell.c $(hloud)
spray.class.sources := classes/binaries/control/spray.c $(hloud)
sprintf.class.sources := classes/binaries/control/sprintf.c $(hloud)
substitute.class.sources := classes/binaries/control/substitute.c $(hgrow)
switch.class.sources := classes/binaries/control/switch.c $(hfitter)
table.class.sources := classes/binaries/control/table.c $(hrandgrowfile)
testmess.class.sources := classes/binaries/control/testmess.c $(hfragile)
thresh.class.sources := classes/binaries/control/thresh.c $(hgrow)
togedge.class.sources := classes/binaries/control/togedge.c $(hloud)
tosymbol.class.sources := classes/binaries/control/tosymbol.c $(hgrow)
universal.class.sources := classes/binaries/control/universal.c $(hfragile)
urn.class.sources := classes/binaries/control/urn.c $(hrandgrow)
zl.class.sources := classes/binaries/control/zl.c $(hgrow)


################################################################################
### Signal (or "sickle") objects ##################################################
################################################################################


# common sources for singal/MSP (or "sickle") classes

ssic := \
shared/sickle/sic.c \
shared/common/loud.c

sforky := \
shared/sickle/sic.c \
shared/common/loud.c \
shared/unstable/forky.c

sfragile := \
shared/sickle/sic.c \
shared/common/loud.c \
shared/unstable/fragile.c

sfragilefitter := \
shared/sickle/sic.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/fragile.c

sgrow := \
shared/common/grow.c \
shared/sickle/sic.c \
shared/common/loud.c

sgrowclc := \
shared/common/grow.c \
shared/common/clc.c \
shared/sickle/sic.c \
shared/common/loud.c

sgrowforky := \
shared/common/grow.c \
shared/sickle/sic.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/forky.c

svefl := \
shared/common/vefl.c \
shared/sickle/sic.c \
shared/common/loud.c \
shared/unstable/fragile.c

sarsic := \
shared/sickle/sic.c \
shared/sickle/arsic.c \
shared/common/vefl.c \
shared/common/loud.c \
shared/unstable/fragile.c

sarsicfitter := \
shared/sickle/sic.c \
shared/sickle/arsic.c \
shared/common/vefl.c \
shared/common/loud.c \
shared/common/fitter.c \
shared/unstable/fragile.c

sfile := \
shared/hammer/file.c \
shared/sickle/sic.c \
shared/common/loud.c \
shared/common/os.c \
shared/unstable/forky.c

# special case: sickle but not tilde (see class linedrive)
splainnotilde := \
shared/common/loud.c \
shared/common/fitter.c


# Signal/MSP classes (or "sickle classes")

# The old build system also allowed these classes to be compiled into the "sickle library"
# Since cyclone version alpha57, this new build system only allows single binaries

abs~.class.sources := classes/binaries/signal/abs.c $(ssic)
acos~.class.sources := classes/binaries/signal/acos.c $(ssic)
acosh~.class.sources := classes/binaries/signal/acosh.c $(ssic)
allpass~.class.sources := classes/binaries/signal/allpass.c $(ssic)
asin~.class.sources := classes/binaries/signal/asin.c $(ssic)
asinh~.class.sources := classes/binaries/signal/asinh.c $(ssic)
atan2~.class.sources := classes/binaries/signal/atan2.c $(ssic)
atan~.class.sources := classes/binaries/signal/atan.c $(ssic)
atanh~.class.sources := classes/binaries/signal/atanh.c $(ssic)
average~.class.sources := classes/binaries/signal/average.c $(ssic)
# average2~.class.sources := classes/binaries/signal/average2.c $(ssic)
avg~.class.sources := classes/binaries/signal/avg.c $(ssic)
bitand~.class.sources := classes/binaries/signal/bitand.c $(sforky)
bitnot~.class.sources := classes/binaries/signal/bitnot.c $(ssic)
bitor~.class.sources := classes/binaries/signal/bitor.c $(sforky)
bitshift~.class.sources := classes/binaries/signal/bitshift.c $(ssic)
bitxor~.class.sources := classes/binaries/signal/bitxor.c $(sforky)
buffir~.class.sources := classes/binaries/signal/buffir.c $(sarsicfitter)
capture~.class.sources := classes/binaries/signal/capture.c $(sfile)
cartopol~.class.sources := classes/binaries/signal/cartopol.c $(sfragile)
change~.class.sources := classes/binaries/signal/change.c $(ssic)
click~.class.sources := classes/binaries/signal/click.c $(sgrow)
clip~.class.sources := classes/binaries/signal/clip.c $(ssic)
comb~.class.sources := classes/binaries/signal/comb.c $(ssic)
cosh~.class.sources := classes/binaries/signal/cosh.c $(ssic)
cosx~.class.sources := classes/binaries/signal/cosx.c $(ssic)
cycle~.class.sources := classes/binaries/signal/cycle.c $(svefl)
delay~.class.sources := classes/binaries/signal/delay.c $(ssic)
delta~.class.sources := classes/binaries/signal/delta.c $(ssic)
# downsamp~.class.sources := classes/binaries/signal/downsamp.c $(ssic)
edge~.class.sources := classes/binaries/signal/edge.c $(ssic)
equals~.class.sources := classes/binaries/signal/equals.c $(ssic)
frameaccum~.class.sources := classes/binaries/signal/frameaccum.c $(sgrow)
framedelta~.class.sources := classes/binaries/signal/framedelta.c $(sgrow)
greaterthan~.class.sources := classes/binaries/signal/greaterthan.c $(ssic)
greaterthaneq~.class.sources := classes/binaries/signal/greaterthaneq.c $(ssic)
index~.class.sources := classes/binaries/signal/index.c $(sarsic)
kink~.class.sources := classes/binaries/signal/kink.c $(ssic)
lessthan~.class.sources := classes/binaries/signal/lessthan.c $(ssic)
lessthaneq~.class.sources := classes/binaries/signal/lessthaneq.c $(ssic)
line~.class.sources := classes/binaries/signal/line.c $(sgrow)
linedrive.class.sources := classes/binaries/signal/linedrive.c $(splainnotilde)
log~.class.sources := classes/binaries/signal/log.c $(ssic)
lookup~.class.sources := classes/binaries/signal/lookup.c $(sarsic)
lores~.class.sources := classes/binaries/signal/lores.c $(ssic)
matrix~.class.sources := classes/binaries/signal/matrix.c $(sfragilefitter)
maximum~.class.sources := classes/binaries/signal/maximum.c $(ssic)
minimum~.class.sources := classes/binaries/signal/minimum.c $(ssic)
minmax~.class.sources := classes/binaries/signal/minmax.c $(ssic)
modulo~.class.sources := classes/binaries/signal/modulo.c $(ssic)
mstosamps~.class.sources := classes/binaries/signal/mstosamps.c $(ssic)
notequals~.class.sources := classes/binaries/signal/notequals.c $(ssic)
onepole~.class.sources := classes/binaries/signal/onepole.c $(ssic)
overdrive~.class.sources := classes/binaries/signal/overdrive.c $(ssic)
peakamp~.class.sources := classes/binaries/signal/peakamp.c $(ssic)
peek~.class.sources := classes/binaries/signal/peek.c $(sarsic)
phasewrap~.class.sources := classes/binaries/signal/phasewrap.c $(ssic)
pink~.class.sources := classes/binaries/signal/pink.c $(ssic)
play~.class.sources := classes/binaries/signal/play.c $(sarsic)
plusequals~.class.sources := classes/binaries/signal/plusequals.c $(ssic)
poke~.class.sources := classes/binaries/signal/poke.c $(sarsic)
poltocar~.class.sources := classes/binaries/signal/poltocar.c $(sfragile)
pow~.class.sources := classes/binaries/signal/pow.c $(ssic)
rampsmooth~.class.sources := classes/binaries/signal/rampsmooth.c $(ssic)
rand~.class.sources := classes/binaries/signal/rand.c $(ssic)
rdiv~.class.sources := classes/binaries/signal/rdiv.c $(ssic)
rminus~.class.sources := classes/binaries/signal/rminus.c $(ssic)
record~.class.sources := classes/binaries/signal/record.c $(sarsic)
reson~.class.sources := classes/binaries/signal/reson.c $(ssic)
sah~.class.sources := classes/binaries/signal/sah.c $(ssic)
sampstoms~.class.sources := classes/binaries/signal/sampstoms.c $(ssic)
scope~.class.sources := classes/binaries/signal/scope.c $(sgrowforky) # UPDATED
sinh~.class.sources := classes/binaries/signal/sinh.c $(ssic)
sinx~.class.sources := classes/binaries/signal/sinx.c $(ssic)
slide~.class.sources := classes/binaries/signal/slide.c $(ssic)
spike~.class.sources := classes/binaries/signal/spike.c $(ssic)
svf~.class.sources := classes/binaries/signal/svf.c $(ssic)
tanh~.class.sources := classes/binaries/signal/tanh.c $(ssic)
tanx~.class.sources := classes/binaries/signal/tanx.c $(ssic)
train~.class.sources := classes/binaries/signal/train.c $(ssic)
vectral~.class.sources := classes/binaries/signal/vectral.c $(ssic)
wave~.class.sources := classes/binaries/signal/wave.c $(sarsic)
zerox~.class.sources := classes/binaries/signal/zerox.c $(ssic)

################################################################################
### Cyclone (sub library with non-alphanumeric objects (Max and MSP classes) ###
################################################################################

cyclone.class.sources := classes/cyclone_lib/cyclone.c $(ssic)

################################################################################

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


# Include Makefile.pdlibbuilder from this directory, or else from central
# externals directory in pd-extended configuration.

externalsdir = ../..

include $(firstword $(wildcard Makefile.pdlibbuilder \
  $(externalsdir)/Makefile.pdlibbuilder))


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