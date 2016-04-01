# Makefile for pure data externals in lib cyclone

lib.name = cyclone

# for the MINGW which has the timespec struct defined twice
cflags = -Ishared -DHAVE_STRUCT_TIMESPEC

uname := $(shell uname -s)
ifeq (MINGW,$(findstring MINGW,$(uname)))
    ldlibs += -lpthread
endif

################################################################################
### hammer #####################################################################
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


# hammer classes
accum.class.sources := hammer/accum.c $(hplain)
acos.class.sources := hammer/acos.c $(hplain)
active.class.sources := hammer/active.c $(hgui)
anal.class.sources := hammer/anal.c $(hloud)
Append.class.sources := hammer/Append.c $(hgrowfitter)
asin.class.sources := hammer/asin.c $(hplain)
atodb.class.sources := hammer/atodb.c
bangbang.class.sources := hammer/bangbang.c $(hfitter)
bondo.class.sources := hammer/bondo.c $(hgrow)
Borax.class.sources := hammer/Borax.c $(hloud)
Bucket.class.sources := hammer/Bucket.c $(hplain)
buddy.class.sources := hammer/buddy.c $(hgrow)
capture.class.sources := hammer/capture.c $(hfile)
cartopol.class.sources := hammer/cartopol.c $(hplain)
Clip.class.sources := hammer/Clip.c $(hgrow)
coll.class.sources := hammer/coll.c $(hfile)
comment.class.sources := hammer/comment.c $(hforky)
cosh.class.sources := hammer/cosh.c $(hplain)
counter.class.sources := hammer/counter.c $(hfitter)
cycle.class.sources := hammer/cycle.c $(hfitter)
dbtoa.class.sources := hammer/dbtoa.c
decide.class.sources := hammer/decide.c $(hloud)
Decode.class.sources := hammer/Decode.c $(hfitter)
drunk.class.sources := hammer/drunk.c $(hrand)
flush.class.sources := hammer/flush.c $(hplain)
forward.class.sources := hammer/forward.c $(hplain)
fromsymbol.class.sources := hammer/fromsymbol.c $(hplain)
funbuff.class.sources := hammer/funbuff.c $(htreefilevefl)
funnel.class.sources := hammer/funnel.c $(hgrow)
gate.class.sources := hammer/gate.c $(hfitter)
grab.class.sources := hammer/grab.c $(hfragile)
Histo.class.sources := hammer/Histo.c $(hloud)
iter.class.sources := hammer/iter.c $(hgrow)
loadmess.class.sources := hammer/loadmess.c $(hfile)
match.class.sources := hammer/match.c $(hgrow)
maximum.class.sources := hammer/maximum.c $(hfitter)
mean.class.sources := hammer/mean.c $(hplain)
midiflush.class.sources := hammer/midiflush.c $(hplain)
midiformat.class.sources := hammer/midiformat.c $(hplain)
midiparse.class.sources := hammer/midiparse.c $(hplain)
minimum.class.sources := hammer/minimum.c $(hfitter)
mousefilter.class.sources := hammer/mousefilter.c $(hgui)
MouseState.class.sources := hammer/MouseState.c $(hgui)
mtr.class.sources := hammer/mtr.c $(hfile)
next.class.sources := hammer/next.c $(hplain)
offer.class.sources := hammer/offer.c $(htree)
onebang.class.sources := hammer/onebang.c $(hplain)
past.class.sources := hammer/past.c $(hgrowfitter)
Peak.class.sources := hammer/Peak.c $(hplain)
poltocar.class.sources := hammer/poltocar.c $(hplain)
pong.class.sources := hammer/pong.c 
prepend.class.sources := hammer/prepend.c $(hgrowfitter)
prob.class.sources := hammer/prob.c $(hrandfile)
pv.class.sources := hammer/pv.c $(hgrow)
round.class.sources := hammer/round.c 
scale.class.sources := hammer/scale.c
seq.class.sources := hammer/seq.c $(hseq)
sinh.class.sources := hammer/sinh.c $(hplain)
speedlim.class.sources := hammer/speedlim.c $(hgrow)
spell.class.sources := hammer/spell.c $(hloud)
split.class.sources := hammer/split.c $(hplain)
spray.class.sources := hammer/spray.c $(hloud)
sprintf.class.sources := hammer/sprintf.c $(hloud)
substitute.class.sources := hammer/substitute.c $(hgrow)
sustain.class.sources := hammer/sustain.c $(hplain)
switch.class.sources := hammer/switch.c $(hfitter)
Table.class.sources := hammer/Table.c $(hrandgrowfile)
tanh.class.sources := hammer/tanh.c $(hplain)
testmess.class.sources := hammer/testmess.c $(hfragile)
thresh.class.sources := hammer/thresh.c $(hgrow)
TogEdge.class.sources := hammer/TogEdge.c $(hloud)
tosymbol.class.sources := hammer/tosymbol.c $(hgrow)
Trough.class.sources := hammer/Trough.c $(hplain)
universal.class.sources := hammer/universal.c $(hfragile)
urn.class.sources := hammer/urn.c $(hrandgrow)
Uzi.class.sources := hammer/Uzi.c $(hplain)
xbendin2.class.sources := hammer/xbendin2.c $(hplain)
xbendin.class.sources := hammer/xbendin.c $(hplain)
xbendout2.class.sources := hammer/xbendout2.c $(hplain)
xbendout.class.sources := hammer/xbendout.c $(hplain)
xnotein.class.sources := hammer/xnotein.c $(hplain)
xnoteout.class.sources := hammer/xnoteout.c $(hplain)
zl.class.sources := hammer/zl.c $(hgrow)


################################################################################
### sickle #####################################################################
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


# sickle classes

abs~.class.sources := sickle/abs.c $(ssic)
acos~.class.sources := sickle/acos.c $(ssic)
acosh~.class.sources := sickle/acosh.c $(ssic)
allpass~.class.sources := sickle/allpass.c $(ssic)
asin~.class.sources := sickle/asin.c $(ssic)
asinh~.class.sources := sickle/asinh.c $(ssic)
atan2~.class.sources := sickle/atan2.c $(ssic)
atan~.class.sources := sickle/atan.c $(ssic)
atanh~.class.sources := sickle/atanh.c $(ssic)
atodb~.class.sources := sickle/atodb.c
average~.class.sources := sickle/average.c $(ssic)
avg~.class.sources := sickle/avg.c $(ssic)
bitand~.class.sources := sickle/bitand.c $(sforky)
bitnot~.class.sources := sickle/bitnot.c $(ssic)
bitor~.class.sources := sickle/bitor.c $(sforky)
bitshift~.class.sources := sickle/bitshift.c $(ssic)
bitxor~.class.sources := sickle/bitxor.c $(sforky)
buffir~.class.sources := sickle/buffir.c $(sarsicfitter)
capture~.class.sources := sickle/capture.c $(sfile)
cartopol~.class.sources := sickle/cartopol.c $(sfragile)
change~.class.sources := sickle/change.c $(ssic)
click~.class.sources := sickle/click.c $(sgrow)
Clip~.class.sources := sickle/Clip.c $(ssic)
comb~.class.sources := sickle/comb.c $(ssic)
cosh~.class.sources := sickle/cosh.c $(ssic)
cosx~.class.sources := sickle/cosx.c $(ssic)
count~.class.sources := sickle/count.c $(ssic)
curve~.class.sources := sickle/curve.c $(sgrowclc)
cycle~.class.sources := sickle/cycle.c $(svefl)
dbtoa~.class.sources := sickle/dbtoa.c
delay~.class.sources := sickle/delay.c $(ssic)
delta~.class.sources := sickle/delta.c $(ssic)
deltaclip~.class.sources := sickle/deltaclip.c $(ssic)
edge~.class.sources := sickle/edge.c $(ssic)
equals~.class.sources := sickle/equals.c $(ssic)
frameaccum~.class.sources := sickle/frameaccum.c $(sgrow)
framedelta~.class.sources := sickle/framedelta.c $(sgrow)
greaterthan~.class.sources := sickle/greaterthan.c $(ssic)
greaterthaneq~.class.sources := sickle/greaterthaneq.c $(ssic)
index~.class.sources := sickle/index.c $(sarsic)
kink~.class.sources := sickle/kink.c $(ssic)
lessthan~.class.sources := sickle/lessthan.c $(ssic)
lessthaneq~.class.sources := sickle/lessthaneq.c $(ssic)
Line~.class.sources := sickle/Line.c $(sgrow)
linedrive.class.sources := sickle/linedrive.c $(splainnotilde)
log~.class.sources := sickle/log.c $(ssic)
lookup~.class.sources := sickle/lookup.c $(sarsic)
lores~.class.sources := sickle/lores.c $(ssic)
matrix~.class.sources := sickle/matrix.c $(sfragilefitter)
maximum~.class.sources := sickle/maximum.c $(ssic)
minimum~.class.sources := sickle/minimum.c $(ssic)
minmax~.class.sources := sickle/minmax.c $(ssic)
modulo~.class.sources := sickle/modulo.c $(ssic)
mstosamps~.class.sources := sickle/mstosamps.c $(ssic)
notequals~.class.sources := sickle/notequals.c $(ssic)
onepole~.class.sources := sickle/onepole.c $(ssic)
overdrive~.class.sources := sickle/overdrive.c $(ssic)
peakamp~.class.sources := sickle/peakamp.c $(ssic)
peek~.class.sources := sickle/peek.c $(sarsic)
phasewrap~.class.sources := sickle/phasewrap.c $(ssic)
pink~.class.sources := sickle/pink.c $(ssic)
play~.class.sources := sickle/play.c $(sarsic)
plusequals~.class.sources := sickle/plusequals.c $(ssic)
poke~.class.sources := sickle/poke.c $(sarsic)
poltocar~.class.sources := sickle/poltocar.c $(sfragile)
pong~.class.sources := sickle/pong.c
pow~.class.sources := sickle/pow.c $(ssic)
rampsmooth~.class.sources := sickle/rampsmooth.c $(ssic)
rand~.class.sources := sickle/rand.c $(ssic)
rdiv~.class.sources := sickle/rdiv.c $(ssic)
rminus~.class.sources := sickle/rminus.c $(ssic)
record~.class.sources := sickle/record.c $(sarsic)
reson~.class.sources := sickle/reson.c $(ssic)
round~.class.sources := sickle/round.c 
sah~.class.sources := sickle/sah.c $(ssic)
sampstoms~.class.sources := sickle/sampstoms.c $(ssic)
Scope~.class.sources := sickle/Scope.c $(sgrowforky)
sinh~.class.sources := sickle/sinh.c $(ssic)
sinx~.class.sources := sickle/sinx.c $(ssic)
slide~.class.sources := sickle/slide.c $(ssic)
Snapshot~.class.sources := sickle/Snapshot.c $(ssic)
spike~.class.sources := sickle/spike.c $(ssic)
svf~.class.sources := sickle/svf.c $(ssic)
tanh~.class.sources := sickle/tanh.c $(ssic)
tanx~.class.sources := sickle/tanx.c $(ssic)
train~.class.sources := sickle/train.c $(ssic)
trapezoid~.class.sources := sickle/trapezoid.c $(ssic)
triangle~.class.sources := sickle/triangle.c $(ssic)
trunc~.class.sources := sickle/trunc.c
vectral~.class.sources := sickle/vectral.c $(ssic)
wave~.class.sources := sickle/wave.c $(sarsic)
zerox~.class.sources := sickle/zerox.c $(ssic)

nettles.class.sources := shadow/nettles.c $(ssic)


datafiles = \
$(wildcard help/*-help.pd) \
help/dspSwitch~.pd \
help/output~.pd \
help/test.mid \
help/voice.wav \
LICENSE.txt \
README.md \
cyclone-meta.pd \
$(wildcard abstractions/*.pd)


################################################################################
### pdlibbuilder ###############################################################
################################################################################


# Include Makefile.pdlibbuilder from this directory, or else from central
# externals directory in pd-extended configuration.

externalsdir = ../..

include $(firstword $(wildcard Makefile.pdlibbuilder \
  $(externalsdir)/Makefile.pdlibbuilder))


################################################################################
### cyclone extra targets ######################################################
################################################################################

install: install-aliases

# on Linux, add symbolic links for lower case aliases
install-aliases: all
ifeq ($(uname), Linux)
	$(INSTALL_DIR) -v $(installpath)
	cd $(installpath); \
        ln -s -f Append.$(extension) append.$(extension); \
        ln -s -f Append-help.pd append-help.pd; \
        ln -s -f Borax.$(extension) borax.$(extension); \
        ln -s -f Borax-help.pd borax-help.pd; \
        ln -s -f Bucket.$(extension) bucket.$(extension); \
        ln -s -f Bucket-help.pd bucket-help.pd; \
        ln -s -f Clip.$(extension) clip.$(extension); \
        ln -s -f Clip-help.pd clip-help.pd; \
        ln -s -f Decode.$(extension) decode.$(extension); \
        ln -s -f Decode-help.pd decode-help.pd; \
        ln -s -f Histo.$(extension) histo.$(extension); \
        ln -s -f Histo-help.pd histo-help.pd; \
        ln -s -f MouseState.$(extension) mousestate.$(extension); \
        ln -s -f MouseState-help.pd mousestate-help.pd; \
        ln -s -f Peak.$(extension) peak.$(extension); \
        ln -s -f Peak-help.pd peak-help.pd; \
        ln -s -f Table.$(extension) table.$(extension); \
        ln -s -f Table-help.pd table-help.pd; \
        ln -s -f TogEdge.$(extension) togedge.$(extension); \
        ln -s -f TogEdge-help.pd togedge-help.pd; \
        ln -s -f Trough.$(extension) trough.$(extension); \
        ln -s -f Trough-help.pd trough-help.pd; \
        ln -s -f Uzi.$(extension) uzi.$(extension); \
        ln -s -f Uzi-help.pd uzi-help.pd; \
        ln -s -f Clip~.$(extension) clip~.$(extension); \
        ln -s -f Clip~-help.pd clip~-help.pd; \
        ln -s -f Line~.$(extension) line~.$(extension); \
        ln -s -f Line~-help.pd line~-help.pd; \
        ln -s -f Scope~.$(extension) scope~.$(extension); \
        ln -s -f Scope~-help.pd scope~-help.pd; \
        ln -s -f Snapshot~.$(extension) snapshot~.$(extension); \
        ln -s -f Snapshot~-help.pd snapshot~-help.pd
endif

