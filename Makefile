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

atodb.class.sources := classes/MAX/atodb.c
dbtoa.class.sources := classes/MAX/dbtoa.c
pong.class.sources := classes/MAX/pong.c
scale.class.sources := classes/MAX/scale.c
round.class.sources := classes/MAX/round.c

# MSP (or "sickle") classes

atodb~.class.sources := classes/MSP/atodb.c
biquad~.class.sources := classes/MSP/biquad.c
# biquad2~.class.sources := classes/MSP/biquad2.c
# downsamp~.class.sources := classes/MSP/downsamp.c
dbtoa~.class.sources := classes/MSP/dbtoa.c
round~.class.sources := classes/MSP/round.c
trunc~.class.sources := classes/MSP/trunc.c

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

accum.class.sources := classes/MAX/accum.c $(hplain)
acos.class.sources := classes/MAX/acos.c $(hplain)
acosh.class.sources := classes/MAX/acosh.c $(hplain)  # NEW CLASS!
active.class.sources := classes/MAX/active.c $(hgui)
anal.class.sources := classes/MAX/anal.c $(hloud)
append.class.sources := classes/MAX/append.c $(hgrowfitter)
asin.class.sources := classes/MAX/asin.c $(hplain)
asinh.class.sources := classes/MAX/asinh.c $(hplain)  # NEW CLASS!
atanh.class.sources := classes/MAX/atanh.c $(hplain)  # NEW CLASS!
bangbang.class.sources := classes/MAX/bangbang.c $(hfitter)
bondo.class.sources := classes/MAX/bondo.c $(hgrow)
borax.class.sources := classes/MAX/borax.c $(hloud)
bucket.class.sources := classes/MAX/bucket.c $(hplain)
buddy.class.sources := classes/MAX/buddy.c $(hgrow)
capture.class.sources := classes/MAX/capture.c $(hfile)
cartopol.class.sources := classes/MAX/cartopol.c $(hplain)
clip.class.sources := classes/MAX/clip.c $(hgrow)
coll.class.sources := classes/MAX/coll.c $(hfile)
comment.class.sources := classes/MAX/comment.c $(hforky)
cosh.class.sources := classes/MAX/cosh.c $(hplain)
counter.class.sources := classes/MAX/counter.c $(hfitter)
cycle.class.sources := classes/MAX/cycle.c $(hfitter)
decide.class.sources := classes/MAX/decide.c $(hloud)
decode.class.sources := classes/MAX/decode.c $(hfitter)
drunk.class.sources := classes/MAX/drunk.c $(hrand)
flush.class.sources := classes/MAX/flush.c $(hplain)
forward.class.sources := classes/MAX/forward.c $(hplain)
fromsymbol.class.sources := classes/MAX/fromsymbol.c $(hplain)
funbuff.class.sources := classes/MAX/funbuff.c $(htreefilevefl)
funnel.class.sources := classes/MAX/funnel.c $(hgrow)
gate.class.sources := classes/MAX/gate.c $(hfitter)
grab.class.sources := classes/MAX/grab.c $(hfragile)
histo.class.sources := classes/MAX/histo.c $(hloud)
iter.class.sources := classes/MAX/iter.c $(hgrow)
loadmess.class.sources := classes/MAX/loadmess.c $(hfile) # NEW CLASS!
match.class.sources := classes/MAX/match.c $(hgrow)
maximum.class.sources := classes/MAX/maximum.c $(hfitter)
mean.class.sources := classes/MAX/mean.c $(hplain)
midiflush.class.sources := classes/MAX/midiflush.c $(hplain)
midiformat.class.sources := classes/MAX/midiformat.c $(hplain)
midiparse.class.sources := classes/MAX/midiparse.c $(hplain)
minimum.class.sources := classes/MAX/minimum.c $(hfitter)
mousefilter.class.sources := classes/MAX/mousefilter.c $(hgui)
mousestate.class.sources := classes/MAX/mousestate.c $(hgui)
mtr.class.sources := classes/MAX/mtr.c $(hfile)
next.class.sources := classes/MAX/next.c $(hplain)
offer.class.sources := classes/MAX/offer.c $(htree)
onebang.class.sources := classes/MAX/onebang.c $(hplain)
past.class.sources := classes/MAX/past.c $(hgrowfitter)
peak.class.sources := classes/MAX/peak.c $(hplain)
poltocar.class.sources := classes/MAX/poltocar.c $(hplain)
prepend.class.sources := classes/MAX/prepend.c $(hgrowfitter)
prob.class.sources := classes/MAX/prob.c $(hrandfile)
pv.class.sources := classes/MAX/pv.c $(hgrow)
rdiv.class.sources := classes/MAX/rdiv.c $(hplain)
rminus.class.sources := classes/MAX/rminus.c $(hplain)
seq.class.sources := classes/MAX/seq.c $(hseq)
sinh.class.sources := classes/MAX/sinh.c $(hplain)
speedlim.class.sources := classes/MAX/speedlim.c $(hgrow)
spell.class.sources := classes/MAX/spell.c $(hloud)
split.class.sources := classes/MAX/split.c $(hplain)
spray.class.sources := classes/MAX/spray.c $(hloud)
sprintf.class.sources := classes/MAX/sprintf.c $(hloud)
substitute.class.sources := classes/MAX/substitute.c $(hgrow)
sustain.class.sources := classes/MAX/sustain.c $(hplain)
switch.class.sources := classes/MAX/switch.c $(hfitter)
table.class.sources := classes/MAX/table.c $(hrandgrowfile)
tanh.class.sources := classes/MAX/tanh.c $(hplain)
testmess.class.sources := classes/MAX/testmess.c $(hfragile)
thresh.class.sources := classes/MAX/thresh.c $(hgrow) # NEW CLASS!
togedge.class.sources := classes/MAX/togedge.c $(hloud)
tosymbol.class.sources := classes/MAX/tosymbol.c $(hgrow)
trough.class.sources := classes/MAX/trough.c $(hplain)
universal.class.sources := classes/MAX/universal.c $(hfragile)
urn.class.sources := classes/MAX/urn.c $(hrandgrow)
uzi.class.sources := classes/MAX/uzi.c $(hplain)
# xbendin2.class.sources := classes/MAX/xbendin2.c $(hplain)
xbendin.class.sources := classes/MAX/xbendin.c $(hplain)
# xbendout2.class.sources := classes/MAX/xbendout2.c $(hplain)
xbendout.class.sources := classes/MAX/xbendout.c $(hplain)
xnotein.class.sources := classes/MAX/xnotein.c $(hplain)
xnoteout.class.sources := classes/MAX/xnoteout.c $(hplain)
zl.class.sources := classes/MAX/zl.c $(hgrow)


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

abs~.class.sources := classes/MSP/abs.c $(ssic)
acos~.class.sources := classes/MSP/acos.c $(ssic)
acosh~.class.sources := classes/MSP/acosh.c $(ssic)
allpass~.class.sources := classes/MSP/allpass.c $(ssic)
asin~.class.sources := classes/MSP/asin.c $(ssic)
asinh~.class.sources := classes/MSP/asinh.c $(ssic)
atan2~.class.sources := classes/MSP/atan2.c $(ssic)
atan~.class.sources := classes/MSP/atan.c $(ssic)
atanh~.class.sources := classes/MSP/atanh.c $(ssic)
average~.class.sources := classes/MSP/average.c $(ssic)
# average2~.class.sources := classes/MSP/average2.c $(ssic)
avg~.class.sources := classes/MSP/avg.c $(ssic)
bitand~.class.sources := classes/MSP/bitand.c $(sforky)
bitnot~.class.sources := classes/MSP/bitnot.c $(ssic)
bitor~.class.sources := classes/MSP/bitor.c $(sforky)
bitshift~.class.sources := classes/MSP/bitshift.c $(ssic)
bitxor~.class.sources := classes/MSP/bitxor.c $(sforky)
buffir~.class.sources := classes/MSP/buffir.c $(sarsicfitter)
capture~.class.sources := classes/MSP/capture.c $(sfile)
cartopol~.class.sources := classes/MSP/cartopol.c $(sfragile)
change~.class.sources := classes/MSP/change.c $(ssic)
click~.class.sources := classes/MSP/click.c $(sgrow)
clip~.class.sources := classes/MSP/clip.c $(ssic)
comb~.class.sources := classes/MSP/comb.c $(ssic)
cosh~.class.sources := classes/MSP/cosh.c $(ssic)
cosx~.class.sources := classes/MSP/cosx.c $(ssic)
count~.class.sources := classes/MSP/count.c $(ssic)
curve~.class.sources := classes/MSP/curve.c $(sgrowclc)
cycle~.class.sources := classes/MSP/cycle.c $(svefl)
delay~.class.sources := classes/MSP/delay.c $(ssic)
delta~.class.sources := classes/MSP/delta.c $(ssic)
deltaclip~.class.sources := classes/MSP/deltaclip.c $(ssic)
edge~.class.sources := classes/MSP/edge.c $(ssic)
equals~.class.sources := classes/MSP/equals.c $(ssic)
frameaccum~.class.sources := classes/MSP/frameaccum.c $(sgrow)
framedelta~.class.sources := classes/MSP/framedelta.c $(sgrow)
greaterthan~.class.sources := classes/MSP/greaterthan.c $(ssic)
greaterthaneq~.class.sources := classes/MSP/greaterthaneq.c $(ssic)
index~.class.sources := classes/MSP/index.c $(sarsic)
kink~.class.sources := classes/MSP/kink.c $(ssic)
lessthan~.class.sources := classes/MSP/lessthan.c $(ssic)
lessthaneq~.class.sources := classes/MSP/lessthaneq.c $(ssic)
line~.class.sources := classes/MSP/line.c $(sgrow)
linedrive.class.sources := classes/MSP/linedrive.c $(splainnotilde)
log~.class.sources := classes/MSP/log.c $(ssic)
lookup~.class.sources := classes/MSP/lookup.c $(sarsic)
lores~.class.sources := classes/MSP/lores.c $(ssic)
matrix~.class.sources := classes/MSP/matrix.c $(sfragilefitter)
maximum~.class.sources := classes/MSP/maximum.c $(ssic)
minimum~.class.sources := classes/MSP/minimum.c $(ssic)
minmax~.class.sources := classes/MSP/minmax.c $(ssic)
modulo~.class.sources := classes/MSP/modulo.c $(ssic)
mstosamps~.class.sources := classes/MSP/mstosamps.c $(ssic)
notequals~.class.sources := classes/MSP/notequals.c $(ssic)
onepole~.class.sources := classes/MSP/onepole.c $(ssic)
overdrive~.class.sources := classes/MSP/overdrive.c $(ssic)
peakamp~.class.sources := classes/MSP/peakamp.c $(ssic)
peek~.class.sources := classes/MSP/peek.c $(sarsic)
phasewrap~.class.sources := classes/MSP/phasewrap.c $(ssic)
pink~.class.sources := classes/MSP/pink.c $(ssic)
play~.class.sources := classes/MSP/play.c $(sarsic)
plusequals~.class.sources := classes/MSP/plusequals.c $(ssic)
poke~.class.sources := classes/MSP/poke.c $(sarsic)
poltocar~.class.sources := classes/MSP/poltocar.c $(sfragile)
pong~.class.sources := classes/MSP/pong.c
pow~.class.sources := classes/MSP/pow.c $(ssic)
rampsmooth~.class.sources := classes/MSP/rampsmooth.c $(ssic)
rand~.class.sources := classes/MSP/rand.c $(ssic)
rdiv~.class.sources := classes/MSP/rdiv.c $(ssic)
rminus~.class.sources := classes/MSP/rminus.c $(ssic)
record~.class.sources := classes/MSP/record.c $(sarsic)
reson~.class.sources := classes/MSP/reson.c $(ssic)
sah~.class.sources := classes/MSP/sah.c $(ssic)
sampstoms~.class.sources := classes/MSP/sampstoms.c $(ssic)
scope~.class.sources := classes/MSP/scope.c $(sgrowforky)
sinh~.class.sources := classes/MSP/sinh.c $(ssic)
sinx~.class.sources := classes/MSP/sinx.c $(ssic)
slide~.class.sources := classes/MSP/slide.c $(ssic)
snapshot~.class.sources := classes/MSP/snapshot.c $(ssic)
spike~.class.sources := classes/MSP/spike.c $(ssic)
svf~.class.sources := classes/MSP/svf.c $(ssic)
tanh~.class.sources := classes/MSP/tanh.c $(ssic)
tanx~.class.sources := classes/MSP/tanx.c $(ssic)
thresh~.class.sources := classes/MSP/thresh.c $(ssic)  # NEW CLASS!
train~.class.sources := classes/MSP/train2.c $(ssic)
trapezoid~.class.sources := classes/MSP/trapezoid.c $(ssic)
triangle~.class.sources := classes/MSP/triangle.c $(ssic)
vectral~.class.sources := classes/MSP/vectral.c $(ssic)
wave~.class.sources := classes/MSP/wave.c $(sarsic)
zerox~.class.sources := classes/MSP/zerox.c $(ssic)

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

