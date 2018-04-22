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
accum.class.sources := cyclone_src/binaries/control/accum.c
acos.class.sources := cyclone_src/binaries/control/acos.c
anal.class.sources := cyclone_src/binaries/control/anal.c
asin.class.sources := cyclone_src/binaries/control/asin.c
bangbang.class.sources := cyclone_src/binaries/control/bangbang.c
borax.class.sources := cyclone_src/binaries/control/borax.c
bucket.class.sources := cyclone_src/binaries/control/bucket.c
cartopol.class.sources := cyclone_src/binaries/control/cartopol.c
counter.class.sources := cyclone_src/binaries/control/counter.c
cosh.class.sources := cyclone_src/binaries/control/cosh.c
cycle.class.sources := cyclone_src/binaries/control/cycle.c
decide.class.sources := cyclone_src/binaries/control/decide.c
decode.class.sources := cyclone_src/binaries/control/decode.c
flush.class.sources := cyclone_src/binaries/control/flush.c
forward.class.sources := cyclone_src/binaries/control/forward.c
fromsymbol.class.sources := cyclone_src/binaries/control/fromsymbol.c
funnel.class.sources := cyclone_src/binaries/control/funnel.c
gate.class.sources := cyclone_src/binaries/control/gate.c
histo.class.sources := cyclone_src/binaries/control/histo.c
listfunnel.class.sources := cyclone_src/binaries/control/listfunnel.c
linedrive.class.sources := cyclone_src/binaries/control/linedrive.c
maximum.class.sources := cyclone_src/binaries/control/maximum.
mean.class.sources := cyclone_src/binaries/control/mean.c
midiflush.class.sources := cyclone_src/binaries/control/midiflush.c
midiformat.class.sources := cyclone_src/binaries/control/midiformat.c
midiparse.class.sources := cyclone_src/binaries/control/midiparse.c
minimum.class.sources := cyclone_src/binaries/control/minimum.c
next.class.sources := cyclone_src/binaries/control/next.c
onebang.class.sources := cyclone_src/binaries/control/onebang.c
past.class.sources := cyclone_src/binaries/control/past.c
peak.class.sources := cyclone_src/binaries/control/peak.c
poltocar.class.sources := cyclone_src/binaries/control/poltocar.c
sinh.class.sources := cyclone_src/binaries/control/sinh.c
spell.class.sources := cyclone_src/binaries/control/spell.c
split.class.sources := cyclone_src/binaries/control/split.c
spray.class.sources := cyclone_src/binaries/control/spray.c
sprintf.class.sources := cyclone_src/binaries/control/sprintf.c
sustain.class.sources := cyclone_src/binaries/control/sustain.c
switch.class.sources := cyclone_src/binaries/control/switch.c
tanh.class.sources := cyclone_src/binaries/control/tanh.c
trough.class.sources := cyclone_src/binaries/control/trough.c
universal.class.sources := cyclone_src/binaries/control/universal.c
unjoin.class.sources := cyclone_src/binaries/control/unjoin.c
uzi.class.sources := cyclone_src/binaries/control/uzi.c
xbendin.class.sources := cyclone_src/binaries/control/xbendin.c
xbendin2.class.sources := cyclone_src/binaries/control/xbendin2.c
xbendout.class.sources := cyclone_src/binaries/control/xbendout.c
xbendout2.class.sources := cyclone_src/binaries/control/xbendout2.c
xnotein.class.sources := cyclone_src/binaries/control/xnotein.c
xnoteout.class.sources := cyclone_src/binaries/control/xnoteout.c
zl.class.sources := cyclone_src/binaries/control/zl.c
# NEW ones in cyclone0.3:
acosh.class.sources := cyclone_src/binaries/control/acosh.c
asinh.class.sources := cyclone_src/binaries/control/asinh.c
atanh.class.sources := cyclone_src/binaries/control/atanh.c
atodb.class.sources := cyclone_src/binaries/control/atodb.c
dbtoa.class.sources := cyclone_src/binaries/control/dbtoa.c
join.class.sources := cyclone_src/binaries/control/join.c
loadmess.class.sources := cyclone_src/binaries/control/loadmess.c
pong.class.sources := cyclone_src/binaries/control/pong.c
pak.class.sources := cyclone_src/binaries/control/pak.c
rdiv.class.sources := cyclone_src/binaries/control/rdiv.c
rminus.class.sources := cyclone_src/binaries/control/rminus.c
round.class.sources := cyclone_src/binaries/control/round.c
scale.class.sources := cyclone_src/binaries/control/scale.c

# SIGNAL CLASSES:
acos~.class.sources := cyclone_src/binaries/signal/acos.c
acosh~.class.sources := cyclone_src/binaries/signal/acosh.c
allpass~.class.sources := cyclone_src/binaries/signal/allpass.c
asinh~.class.sources := cyclone_src/binaries/signal/asinh.c
asin~.class.sources := cyclone_src/binaries/signal/asin.c
atan~.class.sources := cyclone_src/binaries/signal/atan.c
atan2~.class.sources := cyclone_src/binaries/signal/atan2.c
atanh~.class.sources := cyclone_src/binaries/signal/atanh.c
average~.class.sources := cyclone_src/binaries/signal/average.c
avg~.class.sources := cyclone_src/binaries/signal/avg.c
cascade~.class.sources := cyclone_src/binaries/signal/cascade.c
change~.class.sources := cyclone_src/binaries/signal/change.c
click~.class.sources := cyclone_src/binaries/signal/click.c
clip~.class.sources := cyclone_src/binaries/signal/clip.c
cosh~.class.sources := cyclone_src/binaries/signal/cosh.c
cosx~.class.sources := cyclone_src/binaries/signal/cosx.c
count~.class.sources := cyclone_src/binaries/signal/count.c
comb~.class.sources := cyclone_src/binaries/signal/comb.c
curve~.class.sources := cyclone_src/binaries/signal/curve.c
cycle~.class.sources := cyclone_src/binaries/signal/cycle.c
delta~.class.sources := cyclone_src/binaries/signal/delta.c
deltaclip~.class.sources := cyclone_src/binaries/signal/deltaclip.c
edge~.class.sources := cyclone_src/binaries/signal/edge.c
line~.class.sources := cyclone_src/binaries/signal/line.c
lores~.class.sources := cyclone_src/binaries/signal/lores.c
maximum~.class.sources := cyclone_src/binaries/signal/maximum.c
minimum~.class.sources := cyclone_src/binaries/signal/minimum.c
mstosamps~.class.sources := cyclone_src/binaries/signal/mstosamps.c
onepole~.class.sources := cyclone_src/binaries/signal/onepole.c
overdrive~.class.sources := cyclone_src/binaries/signal/overdrive.c
peakamp~.class.sources := cyclone_src/binaries/signal/peakamp.c
phasewrap~.class.sources := cyclone_src/binaries/signal/phasewrap.c
pink~.class.sources := cyclone_src/binaries/signal/pink.c
pong~.class.sources := cyclone_src/binaries/signal/pong.c
pow~.class.sources := cyclone_src/binaries/signal/pow.c
rampsmooth~.class.sources := cyclone_src/binaries/signal/rampsmooth.c
rand~.class.sources := cyclone_src/binaries/signal/rand.c
reson~.class.sources := cyclone_src/binaries/signal/reson.c
sampstoms~.class.sources := cyclone_src/binaries/signal/sampstoms.c
sinh~.class.sources := cyclone_src/binaries/signal/sinh.c
sinx~.class.sources := cyclone_src/binaries/signal/sinx.c
slide~.class.sources := cyclone_src/binaries/signal/slide.c
snapshot~.class.sources := cyclone_src/binaries/signal/snapshot.c
spike~.class.sources := cyclone_src/binaries/signal/spike.c
svf~.class.sources := cyclone_src/binaries/signal/svf.c
tanh~.class.sources := cyclone_src/binaries/signal/tanh.c
tanx~.class.sources := cyclone_src/binaries/signal/tanx.c
teeth~.class.sources := cyclone_src/binaries/signal/teeth.c
togedge.class.sources := cyclone_src/binaries/control/togedge.c
train~.class.sources := cyclone_src/binaries/signal/train.c
trapezoid~.class.sources := cyclone_src/binaries/signal/trapezoid.c
triangle~.class.sources := cyclone_src/binaries/signal/triangle.c
zerox~.class.sources := cyclone_src/binaries/signal/zerox.c
# NEW ones in cyclone0.3:
atodb~.class.sources := cyclone_src/binaries/signal/atodb.c
biquad~.class.sources := cyclone_src/binaries/signal/biquad.c
cross~.class.sources := cyclone_src/binaries/signal/cross.c
dbtoa~.class.sources := cyclone_src/binaries/signal/dbtoa.c
degrade~.class.sources := cyclone_src/binaries/signal/degrade.c
downsamp~.class.sources := cyclone_src/binaries/signal/downsamp.c
equals~.class.sources := cyclone_src/binaries/signal/equals.c
filtercoeff~.class.sources := cyclone_src/binaries/signal/filtercoeff.c
freqshift~.class.sources := cyclone_src/binaries/signal/freqshift.c
greaterthan~.class.sources := cyclone_src/binaries/signal/greaterthan.c
greaterthaneq~.class.sources := cyclone_src/binaries/signal/greaterthaneq.c
Hilbert~.class.sources := cyclone_src/binaries/signal/Hilbert.c
lessthan~.class.sources := cyclone_src/binaries/signal/lessthan.c
lessthaneq~.class.sources := cyclone_src/binaries/signal/lessthaneq.c
modulo~.class.sources := cyclone_src/binaries/signal/modulo.c
notequals~.class.sources := cyclone_src/binaries/signal/notequals.c
phaseshift~.class.sources := cyclone_src/binaries/signal/phaseshift.c
rdiv~.class.sources := cyclone_src/binaries/signal/rdiv.c
rminus~.class.sources := cyclone_src/binaries/signal/rminus.c
round~.class.sources := cyclone_src/binaries/signal/round.c
scale~.class.sources := cyclone_src/binaries/signal/scale.c
thresh~.class.sources := cyclone_src/binaries/signal/thresh.c
trunc~.class.sources := cyclone_src/binaries/signal/trunc.c

##################### CLASSES WITH DEPENDENCIES ##################################

# Control classes: ###############################################

hfile := shared/common/file.c
capture.class.sources := cyclone_src/binaries/control/capture.c $(hfile)
mtr.class.sources := cyclone_src/binaries/control/mtr.c $(hfile)

hgrow := shared/common/grow.c
tosymbol.class.sources := cyclone_src/binaries/control/tosymbol.c $(hgrow)
append.class.sources := cyclone_src/binaries/control/append.c $(hgrow)
clip.class.sources := cyclone_src/binaries/control/clip.c $(hgrow)
prepend.class.sources := cyclone_src/binaries/control/prepend.c $(hgrow)
thresh.class.sources := cyclone_src/binaries/control/thresh.c $(hgrow)
substitute.class.sources := cyclone_src/binaries/control/substitute.c $(hgrow)
speedlim.class.sources := cyclone_src/binaries/control/speedlim.c $(hgrow)
match.class.sources := cyclone_src/binaries/control/match.c $(hgrow)
iter.class.sources := cyclone_src/binaries/control/iter.c $(hgrow)
buddy.class.sources := cyclone_src/binaries/control/buddy.c $(hgrow)
bondo.class.sources := cyclone_src/binaries/control/bondo.c $(hgrow)
pv.class.sources := cyclone_src/binaries/control/pv.c $(hgrow)

hrandfile := \
shared/control/rand.c \
shared/common/file.c
prob.class.sources := cyclone_src/binaries/control/prob.c $(hrandfile)

hgui := shared/control/gui.c
active.class.sources := cyclone_src/binaries/control/active.c $(hgui)
mousefilter.class.sources := cyclone_src/binaries/control/mousefilter.c $(hgui)
mousestate.class.sources := cyclone_src/binaries/control/mousestate.c $(hgui)

htree := shared/control/tree.c
offer.class.sources := cyclone_src/binaries/control/offer.c $(htree)

htreefile := \
shared/control/tree.c \
shared/common/file.c
funbuff.class.sources := cyclone_src/binaries/control/funbuff.c $(htreefile)

hrand := shared/control/rand.c
drunk.class.sources := cyclone_src/binaries/control/drunk.c $(hrand)

hrandgrow := \
shared/control/rand.c \
shared/common/grow.c
urn.class.sources := cyclone_src/binaries/control/urn.c $(hrandgrow)

hrandgrowfile := \
shared/control/rand.c \
shared/common/grow.c \
shared/common/file.c
table.class.sources := cyclone_src/binaries/control/table.c $(hrandgrowfile)

hseq := \
shared/control/mifi.c \
shared/common/file.c \
shared/common/grow.c
seq.class.sources := cyclone_src/binaries/control/seq.c $(hseq)

# still with loud

hfileloud := \
shared/common/file.c \
shared/common/loud.c
coll.class.sources := cyclone_src/binaries/control/coll.c $(hfileloud)

# GUI:
hloud := shared/common/loud.c
comment.class.sources := cyclone_src/binaries/control/comment.c $(hloud)

# New Dependency:

hmagicbit := shared/common/magicbit.c
    grab.class.sources := cyclone_src/binaries/control/grab.c $(hmagicbit)

# Signal: #################################################################

sgrow := shared/common/grow.c
    frameaccum~.class.sources := cyclone_src/binaries/signal/frameaccum.c $(sgrow)
    framedelta~.class.sources := cyclone_src/binaries/signal/framedelta.c $(sgrow)

sfile := shared/common/file.c
    capture~.class.sources := cyclone_src/binaries/signal/capture.c $(sfile)

# New Dependencies:

smagicbit := shared/common/magicbit.c
    cartopol~.class.sources := cyclone_src/binaries/signal/cartopol.c $(smagicbit)
    delay~.class.sources := cyclone_src/binaries/signal/delay.c $(smagicbit)
    plusequals~.class.sources := cyclone_src/binaries/signal/plusequals.c $(smagicbit)
    minmax~.class.sources := cyclone_src/binaries/signal/minmax.c $(smagicbit)
    poltocar~.class.sources := cyclone_src/binaries/signal/poltocar.c $(smagicbit)
    matrix~.class.sources := cyclone_src/binaries/signal/matrix.c $(smagicbit)
    sah~.class.sources := cyclone_src/binaries/signal/sah.c $(smagicbit)
    gate~.class.sources := cyclone_src/binaries/signal/gate.c $(smagicbit)
    selector~.class.sources := cyclone_src/binaries/signal/selector.c $(smagicbit)
    kink~.class.sources := cyclone_src/binaries/signal/kink.c $(smagicbit)
    vectral~.class.sources := cyclone_src/binaries/signal/vectral.c $(smagicbit)
    bitand~.class.sources := cyclone_src/binaries/signal/bitand.c $(smagicbit)
    bitnot~.class.sources := cyclone_src/binaries/signal/bitnot.c $(smagicbit)
    bitor~.class.sources := cyclone_src/binaries/signal/bitor.c $(smagicbit)
    bitsafe~.class.sources := cyclone_src/binaries/signal/bitsafe.c $(smagicbit)
    bitshift~.class.sources := cyclone_src/binaries/signal/bitshift.c $(smagicbit)
    bitxor~.class.sources := cyclone_src/binaries/signal/bitxor.c $(smagicbit)
# GUI
    scope~.class.sources := cyclone_src/binaries/signal/scope.c $(smagicbit)

scybuf := shared/signal/cybuf.c
    buffir~.class.sources := cyclone_src/binaries/signal/buffir.c $(scybuf)
    lookup~.class.sources := cyclone_src/binaries/signal/lookup.c $(scybuf)
    index~.class.sources := cyclone_src/binaries/signal/index.c $(scybuf)
    peek~.class.sources := cyclone_src/binaries/signal/peek.c $(scybuf)
    poke~.class.sources := cyclone_src/binaries/signal/poke.c $(scybuf)
    record~.class.sources := cyclone_src/binaries/signal/record.c $(scybuf)
    wave~.class.sources := cyclone_src/binaries/signal/wave.c $(scybuf)

smagicscybuff := \
shared/common/magicbit.c \
shared/signal/cybuf.c
    play~.class.sources := cyclone_src/binaries/signal/play.c $(smagicscybuff)

# Cyclone Library: ################################################

magicbit := shared/common/magicbit.c
    cyclone.class.sources := cyclone_src/binaries/sub_lib_cyclone.c $(magicbit)

#######################################################################
                        ## END OF CYCLONE CLASSES ##
#######################################################################

### datafiles #########################################################

datafiles = \
$(wildcard cyclone_src/abstractions/*.pd) \
$(wildcard documentation/help_files/*.pd) \
$(wildcard documentation/extra_files/*.*) \
LICENSE.txt \
README.pdf \

# pthreadGC-3.dll is required for Windows installation. It can be found in
# the MinGW directory (usually C:\MinGW\bin) directory and should be
# copied to the current directory before installation or packaging.

ifeq (MINGW,$(findstring MINGW,$(uname)))
datafiles += maintenance/windows_dll/pthreadGC-3.dll
datafiles += maintenance/windows_dll/libgcc_s_dw2-1.dll
endif

### pd-lib-builder ######################################################

include pd-lib-builder/Makefile.pdlibbuilder

### Install UPPER case aliases for Linux ###############################

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
