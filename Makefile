# Makefile for pure data externals in lib cyclone

lib.name = cyclone

# for the MINGW which has the timespec struct defined twice
cflags = -Ishared -DHAVE_STRUCT_TIMESPEC

uname := $(shell uname -s)
ifeq (MINGW,$(findstring MINGW,$(uname)))
    ldlibs += -lpthread
endif


################################################################################
### new classes which don'tuse the old framework common functions ##############
################################################################################

# Max (or "hammer") classes

atodb.class.sources := classes/binaries/MAX/atodb.c
dbtoa.class.sources := classes/binaries/MAX/dbtoa.c
pong.class.sources := classes/binaries/MAX/pong.c
scale.class.sources := classes/binaries/MAX/scale.c
round.class.sources := classes/binaries/MAX/round.c

# MSP (or "sickle") classes

atodb~.class.sources := classes/binaries/MSP/atodb.c
biquad~.class.sources := classes/binaries/MSP/biquad.c
# biquad2~.class.sources := classes/binaries/MSP/biquad2.c
# downsamp~.class.sources := classes/binaries/MSP/downsamp.c
dbtoa~.class.sources := classes/binaries/MSP/dbtoa.c
round~.class.sources := classes/binaries/MSP/round.c
trunc~.class.sources := classes/binaries/MSP/trunc.c

################################################################################
### Max (or "hammer") objects ##################################################
################################################################################

# common sources for Max (or "hammer") classes

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


# Max classes (or "hammer classes")

# The old build system also allowed these classes to be compiled into the "hammer library"
# Since cyclone version alpha57, this new build system only allows single binaries

accum.class.sources := classes/binaries/MAX/accum.c $(hplain)
acos.class.sources := classes/binaries/MAX/acos.c $(hplain)
acosh.class.sources := classes/binaries/MAX/acosh.c $(hplain)  # NEW CLASS!
active.class.sources := classes/binaries/MAX/active.c $(hgui)
anal.class.sources := classes/binaries/MAX/anal.c $(hloud)
append.class.sources := classes/binaries/MAX/append.c $(hgrowfitter)
asin.class.sources := classes/binaries/MAX/asin.c $(hplain)
asinh.class.sources := classes/binaries/MAX/asinh.c $(hplain)  # NEW CLASS!
atanh.class.sources := classes/binaries/MAX/atanh.c $(hplain)  # NEW CLASS!
bangbang.class.sources := classes/binaries/MAX/bangbang.c $(hfitter)
bondo.class.sources := classes/binaries/MAX/bondo.c $(hgrow)
borax.class.sources := classes/binaries/MAX/borax.c $(hloud)
bucket.class.sources := classes/binaries/MAX/bucket.c $(hplain)
buddy.class.sources := classes/binaries/MAX/buddy.c $(hgrow)
capture.class.sources := classes/binaries/MAX/capture.c $(hfile)
cartopol.class.sources := classes/binaries/MAX/cartopol.c $(hplain)
clip.class.sources := classes/binaries/MAX/clip.c $(hgrow)
coll.class.sources := classes/binaries/MAX/coll.c $(hfile)
comment.class.sources := classes/binaries/MAX/comment.c $(hforky)
cosh.class.sources := classes/binaries/MAX/cosh.c $(hplain)
counter.class.sources := classes/binaries/MAX/counter.c $(hfitter)
cycle.class.sources := classes/binaries/MAX/cycle.c $(hfitter)
decide.class.sources := classes/binaries/MAX/decide.c $(hloud)
decode.class.sources := classes/binaries/MAX/decode.c $(hfitter)
drunk.class.sources := classes/binaries/MAX/drunk.c $(hrand)
flush.class.sources := classes/binaries/MAX/flush.c $(hplain)
forward.class.sources := classes/binaries/MAX/forward.c $(hplain)
fromsymbol.class.sources := classes/binaries/MAX/fromsymbol.c $(hplain)
funbuff.class.sources := classes/binaries/MAX/funbuff.c $(htreefilevefl)
funnel.class.sources := classes/binaries/MAX/funnel.c $(hgrow)
gate.class.sources := classes/binaries/MAX/gate.c $(hfitter)
grab.class.sources := classes/binaries/MAX/grab.c $(hfragile)
histo.class.sources := classes/binaries/MAX/histo.c $(hloud)
iter.class.sources := classes/binaries/MAX/iter.c $(hgrow)
loadmess.class.sources := classes/binaries/MAX/loadmess.c $(hfile) # NEW CLASS!
match.class.sources := classes/binaries/MAX/match.c $(hgrow)
maximum.class.sources := classes/binaries/MAX/maximum.c $(hfitter)
mean.class.sources := classes/binaries/MAX/mean.c $(hplain)
midiflush.class.sources := classes/binaries/MAX/midiflush.c $(hplain)
midiformat.class.sources := classes/binaries/MAX/midiformat.c $(hplain)
midiparse.class.sources := classes/binaries/MAX/midiparse.c $(hplain)
minimum.class.sources := classes/binaries/MAX/minimum.c $(hfitter)
mousefilter.class.sources := classes/binaries/MAX/mousefilter.c $(hgui)
mousestate.class.sources := classes/binaries/MAX/mousestate.c $(hgui)
mtr.class.sources := classes/binaries/MAX/mtr.c $(hfile)
next.class.sources := classes/binaries/MAX/next.c $(hplain)
offer.class.sources := classes/binaries/MAX/offer.c $(htree)
onebang.class.sources := classes/binaries/MAX/onebang.c $(hplain)
past.class.sources := classes/binaries/MAX/past.c $(hgrowfitter)
peak.class.sources := classes/binaries/MAX/peak.c $(hplain)
poltocar.class.sources := classes/binaries/MAX/poltocar.c $(hplain)
prepend.class.sources := classes/binaries/MAX/prepend.c $(hgrowfitter)
prob.class.sources := classes/binaries/MAX/prob.c $(hrandfile)
pv.class.sources := classes/binaries/MAX/pv.c $(hgrow)
rdiv.class.sources := classes/binaries/MAX/rdiv.c $(hplain)
rminus.class.sources := classes/binaries/MAX/rminus.c $(hplain)
seq.class.sources := classes/binaries/MAX/seq.c $(hseq)
sinh.class.sources := classes/binaries/MAX/sinh.c $(hplain)
speedlim.class.sources := classes/binaries/MAX/speedlim.c $(hgrow)
spell.class.sources := classes/binaries/MAX/spell.c $(hloud)
split.class.sources := classes/binaries/MAX/split.c $(hplain)
spray.class.sources := classes/binaries/MAX/spray.c $(hloud)
sprintf.class.sources := classes/binaries/MAX/sprintf.c $(hloud)
substitute.class.sources := classes/binaries/MAX/substitute.c $(hgrow)
sustain.class.sources := classes/binaries/MAX/sustain.c $(hplain)
switch.class.sources := classes/binaries/MAX/switch.c $(hfitter)
table.class.sources := classes/binaries/MAX/table.c $(hrandgrowfile)
tanh.class.sources := classes/binaries/MAX/tanh.c $(hplain)
testmess.class.sources := classes/binaries/MAX/testmess.c $(hfragile)
thresh.class.sources := classes/binaries/MAX/thresh.c $(hgrow) # NEW CLASS!
togedge.class.sources := classes/binaries/MAX/togedge.c $(hloud)
tosymbol.class.sources := classes/binaries/MAX/tosymbol.c $(hgrow)
trough.class.sources := classes/binaries/MAX/trough.c $(hplain)
universal.class.sources := classes/binaries/MAX/universal.c $(hfragile)
urn.class.sources := classes/binaries/MAX/urn.c $(hrandgrow)
uzi.class.sources := classes/binaries/MAX/uzi.c $(hplain)
# xbendin2.class.sources := classes/binaries/MAX/xbendin2.c $(hplain)
xbendin.class.sources := classes/binaries/MAX/xbendin.c $(hplain)
# xbendout2.class.sources := classes/binaries/MAX/xbendout2.c $(hplain)
xbendout.class.sources := classes/binaries/MAX/xbendout.c $(hplain)
xnotein.class.sources := classes/binaries/MAX/xnotein.c $(hplain)
xnoteout.class.sources := classes/binaries/MAX/xnoteout.c $(hplain)
zl.class.sources := classes/binaries/MAX/zl.c $(hgrow)


################################################################################
### MSP (or "sickle") objects ##################################################
################################################################################


# common sources for MSP (or "sickle") classes

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


# MSP classes (or "sickle classes")

# The old build system also allowed these classes to be compiled into the "sickle library"
# Since cyclone version alpha57, this new build system only allows single binaries

abs~.class.sources := classes/binaries/MSP/abs.c $(ssic)
acos~.class.sources := classes/binaries/MSP/acos.c $(ssic)
acosh~.class.sources := classes/binaries/MSP/acosh.c $(ssic)
allpass~.class.sources := classes/binaries/MSP/allpass.c $(ssic)
asin~.class.sources := classes/binaries/MSP/asin.c $(ssic)
asinh~.class.sources := classes/binaries/MSP/asinh.c $(ssic)
atan2~.class.sources := classes/binaries/MSP/atan2.c $(ssic)
atan~.class.sources := classes/binaries/MSP/atan.c $(ssic)
atanh~.class.sources := classes/binaries/MSP/atanh.c $(ssic)
average~.class.sources := classes/binaries/MSP/average.c $(ssic)
# average2~.class.sources := classes/binaries/MSP/average2.c $(ssic)
avg~.class.sources := classes/binaries/MSP/avg.c $(ssic)
bitand~.class.sources := classes/binaries/MSP/bitand.c $(sforky)
bitnot~.class.sources := classes/binaries/MSP/bitnot.c $(ssic)
bitor~.class.sources := classes/binaries/MSP/bitor.c $(sforky)
bitshift~.class.sources := classes/binaries/MSP/bitshift.c $(ssic)
bitxor~.class.sources := classes/binaries/MSP/bitxor.c $(sforky)
buffir~.class.sources := classes/binaries/MSP/buffir.c $(sarsicfitter)
capture~.class.sources := classes/binaries/MSP/capture.c $(sfile)
cartopol~.class.sources := classes/binaries/MSP/cartopol.c $(sfragile)
change~.class.sources := classes/binaries/MSP/change.c $(ssic)
click~.class.sources := classes/binaries/MSP/click.c $(sgrow)
clip~.class.sources := classes/binaries/MSP/clip.c $(ssic)
comb~.class.sources := classes/binaries/MSP/comb.c $(ssic)
cosh~.class.sources := classes/binaries/MSP/cosh.c $(ssic)
cosx~.class.sources := classes/binaries/MSP/cosx.c $(ssic)
count~.class.sources := classes/binaries/MSP/count.c $(ssic)
curve~.class.sources := classes/binaries/MSP/curve.c $(sgrowclc)
cycle~.class.sources := classes/binaries/MSP/cycle.c $(svefl)
delay~.class.sources := classes/binaries/MSP/delay.c $(ssic)
delta~.class.sources := classes/binaries/MSP/delta.c $(ssic)
deltaclip~.class.sources := classes/binaries/MSP/deltaclip.c $(ssic)
edge~.class.sources := classes/binaries/MSP/edge.c $(ssic)
equals~.class.sources := classes/binaries/MSP/equals.c $(ssic)
frameaccum~.class.sources := classes/binaries/MSP/frameaccum.c $(sgrow)
framedelta~.class.sources := classes/binaries/MSP/framedelta.c $(sgrow)
greaterthan~.class.sources := classes/binaries/MSP/greaterthan.c $(ssic)
greaterthaneq~.class.sources := classes/binaries/MSP/greaterthaneq.c $(ssic)
index~.class.sources := classes/binaries/MSP/index.c $(sarsic)
kink~.class.sources := classes/binaries/MSP/kink.c $(ssic)
lessthan~.class.sources := classes/binaries/MSP/lessthan.c $(ssic)
lessthaneq~.class.sources := classes/binaries/MSP/lessthaneq.c $(ssic)
line~.class.sources := classes/binaries/MSP/line.c $(sgrow)
linedrive.class.sources := classes/binaries/MSP/linedrive.c $(splainnotilde)
log~.class.sources := classes/binaries/MSP/log.c $(ssic)
lookup~.class.sources := classes/binaries/MSP/lookup.c $(sarsic)
lores~.class.sources := classes/binaries/MSP/lores.c $(ssic)
matrix~.class.sources := classes/binaries/MSP/matrix.c $(sfragilefitter)
maximum~.class.sources := classes/binaries/MSP/maximum.c $(ssic)
minimum~.class.sources := classes/binaries/MSP/minimum.c $(ssic)
minmax~.class.sources := classes/binaries/MSP/minmax.c $(ssic)
modulo~.class.sources := classes/binaries/MSP/modulo.c $(ssic)
mstosamps~.class.sources := classes/binaries/MSP/mstosamps.c $(ssic)
notequals~.class.sources := classes/binaries/MSP/notequals.c $(ssic)
onepole~.class.sources := classes/binaries/MSP/onepole.c $(ssic)
overdrive~.class.sources := classes/binaries/MSP/overdrive.c $(ssic)
peakamp~.class.sources := classes/binaries/MSP/peakamp.c $(ssic)
peek~.class.sources := classes/binaries/MSP/peek.c $(sarsic)
phasewrap~.class.sources := classes/binaries/MSP/phasewrap.c $(ssic)
pink~.class.sources := classes/binaries/MSP/pink.c $(ssic)
play~.class.sources := classes/binaries/MSP/play.c $(sarsic)
plusequals~.class.sources := classes/binaries/MSP/plusequals.c $(ssic)
poke~.class.sources := classes/binaries/MSP/poke.c $(sarsic)
poltocar~.class.sources := classes/binaries/MSP/poltocar.c $(sfragile)
pong~.class.sources := classes/binaries/MSP/pong.c
pow~.class.sources := classes/binaries/MSP/pow.c $(ssic)
rampsmooth~.class.sources := classes/binaries/MSP/rampsmooth.c $(ssic)
rand~.class.sources := classes/binaries/MSP/rand.c $(ssic)
rdiv~.class.sources := classes/binaries/MSP/rdiv.c $(ssic)
rminus~.class.sources := classes/binaries/MSP/rminus.c $(ssic)
record~.class.sources := classes/binaries/MSP/record.c $(sarsic)
reson~.class.sources := classes/binaries/MSP/reson.c $(ssic)
sah~.class.sources := classes/binaries/MSP/sah.c $(ssic)
sampstoms~.class.sources := classes/binaries/MSP/sampstoms.c $(ssic)
scope~.class.sources := classes/binaries/MSP/scope.c $(sgrowforky)
sinh~.class.sources := classes/binaries/MSP/sinh.c $(ssic)
sinx~.class.sources := classes/binaries/MSP/sinx.c $(ssic)
slide~.class.sources := classes/binaries/MSP/slide.c $(ssic)
snapshot~.class.sources := classes/binaries/MSP/snapshot.c $(ssic)
spike~.class.sources := classes/binaries/MSP/spike.c $(ssic)
svf~.class.sources := classes/binaries/MSP/svf.c $(ssic)
tanh~.class.sources := classes/binaries/MSP/tanh.c $(ssic)
tanx~.class.sources := classes/binaries/MSP/tanx.c $(ssic)
thresh~.class.sources := classes/binaries/MSP/thresh.c $(ssic)  # NEW CLASS!
train~.class.sources := classes/binaries/MSP/train.c $(ssic)
trapezoid~.class.sources := classes/binaries/MSP/trapezoid.c $(ssic)
triangle~.class.sources := classes/binaries/MSP/triangle.c $(ssic)
vectral~.class.sources := classes/binaries/MSP/vectral.c $(ssic)
wave~.class.sources := classes/binaries/MSP/wave.c $(sarsic)
zerox~.class.sources := classes/binaries/MSP/zerox.c $(ssic)

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

