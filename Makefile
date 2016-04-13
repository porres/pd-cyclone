# Makefile for pure data externals in lib cyclone

lib.name = cyclone

# for the MINGW which has the timespec struct defined twice
cflags = -Ishared -DHAVE_STRUCT_TIMESPEC

uname := $(shell uname -s)
ifeq (MINGW,$(findstring MINGW,$(uname)))
    ldlibs += -lpthread
endif


################################################################################
### new classes which don'tuse the old framework functions #####################
################################################################################

# hammer (control; Max) classes

atodb.class.sources := classes/hammer/atodb.c
dbtoa.class.sources := classes/hammer/dbtoa.c
pong.class.sources := classes/hammer/pong.c
scale.class.sources := classes/hammer/scale.c
round.class.sources := classes/hammer/round.c

# sickle (signal; MSP) classes

atodb~.class.sources := classes/sickle/atodb.c
biquad~.class.sources := classes/sickle/biquad.c
# biquad2~.class.sources := classes/sickle/biquad2.c
# downsamp~.class.sources := classes/sickle/downsamp.c
dbtoa~.class.sources := classes/sickle/dbtoa.c
round~.class.sources := classes/sickle/round.c
trunc~.class.sources := classes/sickle/trunc.c

################################################################################
### hammer (control; Max) objects ##############################################
################################################################################


# common sources for hammer types

hloud := \
shared/common/loud.c

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

hfile := \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/common/fitter.c \
shared/unstable/forky.c

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

hgui := \
shared/hammer/gui.c \
shared/common/loud.c

hseq := \
shared/common/mifi.c \
shared/hammer/file.c \
shared/common/grow.c \
shared/common/loud.c \
shared/common/os.c \
shared/common/fitter.c \
shared/unstable/forky.c


# hammer (control; Max) classes

accum.class.sources := classes/hammer/accum.c $(hplain)
acos.class.sources := classes/hammer/acos.c $(hplain)
acosh.class.sources := classes/hammer/acosh.c $(hplain)  # NEW CLASS!
active.class.sources := classes/hammer/active.c $(hgui)
anal.class.sources := classes/hammer/anal.c $(hloud)
append.class.sources := classes/hammer/append.c $(hgrowfitter)
asin.class.sources := classes/hammer/asin.c $(hplain)
asinh.class.sources := classes/hammer/asinh.c $(hplain)  # NEW CLASS!
atanh.class.sources := classes/hammer/atanh.c $(hplain)  # NEW CLASS!
bangbang.class.sources := classes/hammer/bangbang.c $(hfitter)
bondo.class.sources := classes/hammer/bondo.c $(hgrow)
borax.class.sources := classes/hammer/borax.c $(hloud)
bucket.class.sources := classes/hammer/bucket.c $(hplain)
buddy.class.sources := classes/hammer/buddy.c $(hgrow)
capture.class.sources := classes/hammer/capture.c $(hfile)
cartopol.class.sources := classes/hammer/cartopol.c $(hplain)
clip.class.sources := classes/hammer/clip.c $(hgrow)
coll.class.sources := classes/hammer/coll.c $(hfile)
comment.class.sources := classes/hammer/comment.c $(hforky)
cosh.class.sources := classes/hammer/cosh.c $(hplain)
counter.class.sources := classes/hammer/counter.c $(hfitter)
cycle.class.sources := classes/hammer/cycle.c $(hfitter)
decide.class.sources := classes/hammer/decide.c $(hloud)
decode.class.sources := classes/hammer/decode.c $(hfitter)
drunk.class.sources := classes/hammer/drunk.c $(hrand)
flush.class.sources := classes/hammer/flush.c $(hplain)
forward.class.sources := classes/hammer/forward.c $(hplain)
fromsymbol.class.sources := classes/hammer/fromsymbol.c $(hplain)
funbuff.class.sources := classes/hammer/funbuff.c $(htreefilevefl)
funnel.class.sources := classes/hammer/funnel.c $(hgrow)
gate.class.sources := classes/hammer/gate.c $(hfitter)
grab.class.sources := classes/hammer/grab.c $(hfragile)
histo.class.sources := classes/hammer/histo.c $(hloud)
iter.class.sources := classes/hammer/iter.c $(hgrow)
loadmess.class.sources := classes/hammer/loadmess.c $(hfile) # NEW CLASS!
match.class.sources := classes/hammer/match.c $(hgrow)
maximum.class.sources := classes/hammer/maximum.c $(hfitter)
mean.class.sources := classes/hammer/mean.c $(hplain)
midiflush.class.sources := classes/hammer/midiflush.c $(hplain)
midiformat.class.sources := classes/hammer/midiformat.c $(hplain)
midiparse.class.sources := classes/hammer/midiparse.c $(hplain)
minimum.class.sources := classes/hammer/minimum.c $(hfitter)
mousefilter.class.sources := classes/hammer/mousefilter.c $(hgui)
mousestate.class.sources := classes/hammer/mousestate.c $(hgui)
mtr.class.sources := classes/hammer/mtr.c $(hfile)
next.class.sources := classes/hammer/next.c $(hplain)
offer.class.sources := classes/hammer/offer.c $(htree)
onebang.class.sources := classes/hammer/onebang.c $(hplain)
past.class.sources := classes/hammer/past.c $(hgrowfitter)
peak.class.sources := classes/hammer/peak.c $(hplain)
poltocar.class.sources := classes/hammer/poltocar.c $(hplain)
prepend.class.sources := classes/hammer/prepend.c $(hgrowfitter)
prob.class.sources := classes/hammer/prob.c $(hrandfile)
pv.class.sources := classes/hammer/pv.c $(hgrow)
rdiv.class.sources := classes/hammer/rdiv.c $(hplain)
rminus.class.sources := classes/hammer/rminus.c $(hplain)
seq.class.sources := classes/hammer/seq.c $(hseq)
sinh.class.sources := classes/hammer/sinh.c $(hplain)
speedlim.class.sources := classes/hammer/speedlim.c $(hgrow)
spell.class.sources := classes/hammer/spell.c $(hloud)
split.class.sources := classes/hammer/split.c $(hplain)
spray.class.sources := classes/hammer/spray.c $(hloud)
sprintf.class.sources := classes/hammer/sprintf.c $(hloud)
substitute.class.sources := classes/hammer/substitute.c $(hgrow)
sustain.class.sources := classes/hammer/sustain.c $(hplain)
switch.class.sources := classes/hammer/switch.c $(hfitter)
table.class.sources := classes/hammer/table.c $(hrandgrowfile)
tanh.class.sources := classes/hammer/tanh.c $(hplain)
testmess.class.sources := classes/hammer/testmess.c $(hfragile)
thresh.class.sources := classes/hammer/thresh.c $(hgrow) # NEW CLASS!
togedge.class.sources := classes/hammer/togedge.c $(hloud)
tosymbol.class.sources := classes/hammer/tosymbol.c $(hgrow)
trough.class.sources := classes/hammer/trough.c $(hplain)
universal.class.sources := classes/hammer/universal.c $(hfragile)
urn.class.sources := classes/hammer/urn.c $(hrandgrow)
uzi.class.sources := classes/hammer/uzi.c $(hplain)
# xbendin2.class.sources := classes/hammer/xbendin2.c $(hplain)
xbendin.class.sources := classes/hammer/xbendin.c $(hplain)
# xbendout2.class.sources := classes/hammer/xbendout2.c $(hplain)
xbendout.class.sources := classes/hammer/xbendout.c $(hplain)
xnotein.class.sources := classes/hammer/xnotein.c $(hplain)
xnoteout.class.sources := classes/hammer/xnoteout.c $(hplain)
zl.class.sources := classes/hammer/zl.c $(hgrow)


################################################################################
### sickle (signal; MSP) objects ###############################################
################################################################################


# common sources for sickle types

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


# sickle (signal; MSP) classes

abs~.class.sources := classes/sickle/abs.c $(ssic)
acos~.class.sources := classes/sickle/acos.c $(ssic)
acosh~.class.sources := classes/sickle/acosh.c $(ssic)
allpass~.class.sources := classes/sickle/allpass.c $(ssic)
asin~.class.sources := classes/sickle/asin.c $(ssic)
asinh~.class.sources := classes/sickle/asinh.c $(ssic)
atan2~.class.sources := classes/sickle/atan2.c $(ssic)
atan~.class.sources := classes/sickle/atan.c $(ssic)
atanh~.class.sources := classes/sickle/atanh.c $(ssic)
average~.class.sources := classes/sickle/average.c $(ssic)
# average2~.class.sources := classes/sickle/average2.c $(ssic)
avg~.class.sources := classes/sickle/avg.c $(ssic)
bitand~.class.sources := classes/sickle/bitand.c $(sforky)
bitnot~.class.sources := classes/sickle/bitnot.c $(ssic)
bitor~.class.sources := classes/sickle/bitor.c $(sforky)
bitshift~.class.sources := classes/sickle/bitshift.c $(ssic)
bitxor~.class.sources := classes/sickle/bitxor.c $(sforky)
buffir~.class.sources := classes/sickle/buffir.c $(sarsicfitter)
capture~.class.sources := classes/sickle/capture.c $(sfile)
cartopol~.class.sources := classes/sickle/cartopol.c $(sfragile)
change~.class.sources := classes/sickle/change.c $(ssic)
click~.class.sources := classes/sickle/click.c $(sgrow)
clip~.class.sources := classes/sickle/clip.c $(ssic)
comb~.class.sources := classes/sickle/comb.c $(ssic)
cosh~.class.sources := classes/sickle/cosh.c $(ssic)
cosx~.class.sources := classes/sickle/cosx.c $(ssic)
count~.class.sources := classes/sickle/count.c $(ssic)
curve~.class.sources := classes/sickle/curve.c $(sgrowclc)
cycle~.class.sources := classes/sickle/cycle.c $(svefl)
delay~.class.sources := classes/sickle/delay.c $(ssic)
delta~.class.sources := classes/sickle/delta.c $(ssic)
deltaclip~.class.sources := classes/sickle/deltaclip.c $(ssic)
edge~.class.sources := classes/sickle/edge.c $(ssic)
equals~.class.sources := classes/sickle/equals.c $(ssic)
frameaccum~.class.sources := classes/sickle/frameaccum.c $(sgrow)
framedelta~.class.sources := classes/sickle/framedelta.c $(sgrow)
greaterthan~.class.sources := classes/sickle/greaterthan.c $(ssic)
greaterthaneq~.class.sources := classes/sickle/greaterthaneq.c $(ssic)
index~.class.sources := classes/sickle/index.c $(sarsic)
kink~.class.sources := classes/sickle/kink.c $(ssic)
lessthan~.class.sources := classes/sickle/lessthan.c $(ssic)
lessthaneq~.class.sources := classes/sickle/lessthaneq.c $(ssic)
line~.class.sources := classes/sickle/line.c $(sgrow)
linedrive.class.sources := classes/sickle/linedrive.c $(splainnotilde)
log~.class.sources := classes/sickle/log.c $(ssic)
lookup~.class.sources := classes/sickle/lookup.c $(sarsic)
lores~.class.sources := classes/sickle/lores.c $(ssic)
matrix~.class.sources := classes/sickle/matrix.c $(sfragilefitter)
maximum~.class.sources := classes/sickle/maximum.c $(ssic)
minimum~.class.sources := classes/sickle/minimum.c $(ssic)
minmax~.class.sources := classes/sickle/minmax.c $(ssic)
modulo~.class.sources := classes/sickle/modulo.c $(ssic)
mstosamps~.class.sources := classes/sickle/mstosamps.c $(ssic)
notequals~.class.sources := classes/sickle/notequals.c $(ssic)
onepole~.class.sources := classes/sickle/onepole.c $(ssic)
overdrive~.class.sources := classes/sickle/overdrive.c $(ssic)
peakamp~.class.sources := classes/sickle/peakamp.c $(ssic)
peek~.class.sources := classes/sickle/peek.c $(sarsic)
phasewrap~.class.sources := classes/sickle/phasewrap.c $(ssic)
pink~.class.sources := classes/sickle/pink.c $(ssic)
play~.class.sources := classes/sickle/play.c $(sarsic)
plusequals~.class.sources := classes/sickle/plusequals.c $(ssic)
poke~.class.sources := classes/sickle/poke.c $(sarsic)
poltocar~.class.sources := classes/sickle/poltocar.c $(sfragile)
pong~.class.sources := classes/sickle/pong.c
pow~.class.sources := classes/sickle/pow.c $(ssic)
rampsmooth~.class.sources := classes/sickle/rampsmooth.c $(ssic)
rand~.class.sources := classes/sickle/rand.c $(ssic)
rdiv~.class.sources := classes/sickle/rdiv.c $(ssic)
rminus~.class.sources := classes/sickle/rminus.c $(ssic)
record~.class.sources := classes/sickle/record.c $(sarsic)
reson~.class.sources := classes/sickle/reson.c $(ssic)
sah~.class.sources := classes/sickle/sah.c $(ssic)
sampstoms~.class.sources := classes/sickle/sampstoms.c $(ssic)
scope~.class.sources := classes/sickle/scope.c $(sgrowforky)
sinh~.class.sources := classes/sickle/sinh.c $(ssic)
sinx~.class.sources := classes/sickle/sinx.c $(ssic)
slide~.class.sources := classes/sickle/slide.c $(ssic)
snapshot~.class.sources := classes/sickle/snapshot.c $(ssic)
spike~.class.sources := classes/sickle/spike.c $(ssic)
svf~.class.sources := classes/sickle/svf.c $(ssic)
tanh~.class.sources := classes/sickle/tanh.c $(ssic)
tanx~.class.sources := classes/sickle/tanx.c $(ssic)
thresh~.class.sources := classes/sickle/thresh.c $(ssic)  # NEW CLASS!
train~.class.sources := classes/sickle/train.c $(ssic)
trapezoid~.class.sources := classes/sickle/trapezoid.c $(ssic)
triangle~.class.sources := classes/sickle/triangle.c $(ssic)
vectral~.class.sources := classes/sickle/vectral.c $(ssic)
wave~.class.sources := classes/sickle/wave.c $(sarsic)
zerox~.class.sources := classes/sickle/zerox.c $(ssic)

################################################################################
### Cyclone (sub library with non-alphanumeric objects (Max and MSP classes) ###
################################################################################

cyclone.class.sources := classes/cyclone_lib/cyclone_lib.c $(ssic)

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

# on Linux, add symbolic links for UPPER case aliases

install-aliases: all
ifeq ($(uname), Linux)
	$(INSTALL_DIR) -v $(installpath)
	cd $(installpath); \
# Control
        ln -s -f append.$(extension) Append.$(extension); \ # ???
        ln -s -f append-help.pd Append-help.pd; \ # ???
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
# Signal
        ln -s -f clip~.$(extension) Clip~.$(extension); \ 
        ln -s -f clip~-help.pd Clip~-help.pd; \
        ln -s -f line~.$(extension) Line~.$(extension); \
        ln -s -f line~-help.pd Line~-help.pd; \
        ln -s -f scope~.$(extension) Scope~.$(extension); \
        ln -s -f scope~-help.pd Scope~-help.pd; \
        ln -s -f snapshot~.$(extension) Snapshot~.$(extension); \
        ln -s -f snapshot~-help.pd Snapshot~-help.pd
endif

