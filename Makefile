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
accum.class.sources := cyclone_objects/binaries/control/accum.c
acos.class.sources := cyclone_objects/binaries/control/acos.c
anal.class.sources := cyclone_objects/binaries/control/anal.c
asin.class.sources := cyclone_objects/binaries/control/asin.c
bangbang.class.sources := cyclone_objects/binaries/control/bangbang.c
borax.class.sources := cyclone_objects/binaries/control/borax.c
bucket.class.sources := cyclone_objects/binaries/control/bucket.c
cartopol.class.sources := cyclone_objects/binaries/control/cartopol.c
counter.class.sources := cyclone_objects/binaries/control/counter.c
cosh.class.sources := cyclone_objects/binaries/control/cosh.c
cycle.class.sources := cyclone_objects/binaries/control/cycle.c
decide.class.sources := cyclone_objects/binaries/control/decide.c
decode.class.sources := cyclone_objects/binaries/control/decode.c
flush.class.sources := cyclone_objects/binaries/control/flush.c
forward.class.sources := cyclone_objects/binaries/control/forward.c
fromsymbol.class.sources := cyclone_objects/binaries/control/fromsymbol.c
funnel.class.sources := cyclone_objects/binaries/control/funnel.c
gate.class.sources := cyclone_objects/binaries/control/gate.c
histo.class.sources := cyclone_objects/binaries/control/histo.c
listfunnel.class.sources := cyclone_objects/binaries/control/listfunnel.c
linedrive.class.sources := cyclone_objects/binaries/control/linedrive.c
maximum.class.sources := cyclone_objects/binaries/control/maximum.c
mean.class.sources := cyclone_objects/binaries/control/mean.c
midiflush.class.sources := cyclone_objects/binaries/control/midiflush.c
midiformat.class.sources := cyclone_objects/binaries/control/midiformat.c
midiparse.class.sources := cyclone_objects/binaries/control/midiparse.c
minimum.class.sources := cyclone_objects/binaries/control/minimum.c
next.class.sources := cyclone_objects/binaries/control/next.c
onebang.class.sources := cyclone_objects/binaries/control/onebang.c
past.class.sources := cyclone_objects/binaries/control/past.c
peak.class.sources := cyclone_objects/binaries/control/peak.c
poltocar.class.sources := cyclone_objects/binaries/control/poltocar.c
sinh.class.sources := cyclone_objects/binaries/control/sinh.c
spell.class.sources := cyclone_objects/binaries/control/spell.c
split.class.sources := cyclone_objects/binaries/control/split.c
spray.class.sources := cyclone_objects/binaries/control/spray.c
sprintf.class.sources := cyclone_objects/binaries/control/sprintf.c
sustain.class.sources := cyclone_objects/binaries/control/sustain.c
switch.class.sources := cyclone_objects/binaries/control/switch.c
tanh.class.sources := cyclone_objects/binaries/control/tanh.c
trough.class.sources := cyclone_objects/binaries/control/trough.c
universal.class.sources := cyclone_objects/binaries/control/universal.c
unjoin.class.sources := cyclone_objects/binaries/control/unjoin.c
uzi.class.sources := cyclone_objects/binaries/control/uzi.c
xbendin.class.sources := cyclone_objects/binaries/control/xbendin.c
xbendin2.class.sources := cyclone_objects/binaries/control/xbendin2.c
xbendout.class.sources := cyclone_objects/binaries/control/xbendout.c
xbendout2.class.sources := cyclone_objects/binaries/control/xbendout2.c
xnotein.class.sources := cyclone_objects/binaries/control/xnotein.c
xnoteout.class.sources := cyclone_objects/binaries/control/xnoteout.c
zl.class.sources := cyclone_objects/binaries/control/zl.c
# NEW ones in cyclone0.3:
acosh.class.sources := cyclone_objects/binaries/control/acosh.c
asinh.class.sources := cyclone_objects/binaries/control/asinh.c
atanh.class.sources := cyclone_objects/binaries/control/atanh.c
atodb.class.sources := cyclone_objects/binaries/control/atodb.c
dbtoa.class.sources := cyclone_objects/binaries/control/dbtoa.c
join.class.sources := cyclone_objects/binaries/control/join.c
loadmess.class.sources := cyclone_objects/binaries/control/loadmess.c
pong.class.sources := cyclone_objects/binaries/control/pong.c
pak.class.sources := cyclone_objects/binaries/control/pak.c
rdiv.class.sources := cyclone_objects/binaries/control/rdiv.c
rminus.class.sources := cyclone_objects/binaries/control/rminus.c
round.class.sources := cyclone_objects/binaries/control/round.c
scale.class.sources := cyclone_objects/binaries/control/scale.c

# SIGNAL CLASSES:
acos~.class.sources := cyclone_objects/binaries/audio/acos.c
acosh~.class.sources := cyclone_objects/binaries/audio/acosh.c
allpass~.class.sources := cyclone_objects/binaries/audio/allpass.c
asinh~.class.sources := cyclone_objects/binaries/audio/asinh.c
asin~.class.sources := cyclone_objects/binaries/audio/asin.c
atan~.class.sources := cyclone_objects/binaries/audio/atan.c
atan2~.class.sources := cyclone_objects/binaries/audio/atan2.c
atanh~.class.sources := cyclone_objects/binaries/audio/atanh.c
average~.class.sources := cyclone_objects/binaries/audio/average.c
avg~.class.sources := cyclone_objects/binaries/audio/avg.c
change~.class.sources := cyclone_objects/binaries/audio/change.c
click~.class.sources := cyclone_objects/binaries/audio/click.c
clip~.class.sources := cyclone_objects/binaries/audio/clip.c
cosh~.class.sources := cyclone_objects/binaries/audio/cosh.c
cosx~.class.sources := cyclone_objects/binaries/audio/cosx.c
count~.class.sources := cyclone_objects/binaries/audio/count.c
comb~.class.sources := cyclone_objects/binaries/audio/comb.c
curve~.class.sources := cyclone_objects/binaries/audio/curve.c
cycle~.class.sources := cyclone_objects/binaries/audio/cycle.c
delta~.class.sources := cyclone_objects/binaries/audio/delta.c
deltaclip~.class.sources := cyclone_objects/binaries/audio/deltaclip.c
edge~.class.sources := cyclone_objects/binaries/audio/edge.c
line~.class.sources := cyclone_objects/binaries/audio/line.c
lores~.class.sources := cyclone_objects/binaries/audio/lores.c
maximum~.class.sources := cyclone_objects/binaries/audio/maximum.c
minimum~.class.sources := cyclone_objects/binaries/audio/minimum.c
mstosamps~.class.sources := cyclone_objects/binaries/audio/mstosamps.c
onepole~.class.sources := cyclone_objects/binaries/audio/onepole.c
overdrive~.class.sources := cyclone_objects/binaries/audio/overdrive.c
peakamp~.class.sources := cyclone_objects/binaries/audio/peakamp.c
phasewrap~.class.sources := cyclone_objects/binaries/audio/phasewrap.c
pink~.class.sources := cyclone_objects/binaries/audio/pink.c
pong~.class.sources := cyclone_objects/binaries/audio/pong.c
pow~.class.sources := cyclone_objects/binaries/audio/pow.c
rampsmooth~.class.sources := cyclone_objects/binaries/audio/rampsmooth.c
rand~.class.sources := cyclone_objects/binaries/audio/rand.c
reson~.class.sources := cyclone_objects/binaries/audio/reson.c
sampstoms~.class.sources := cyclone_objects/binaries/audio/sampstoms.c
sinh~.class.sources := cyclone_objects/binaries/audio/sinh.c
sinx~.class.sources := cyclone_objects/binaries/audio/sinx.c
slide~.class.sources := cyclone_objects/binaries/audio/slide.c
snapshot~.class.sources := cyclone_objects/binaries/audio/snapshot.c
spike~.class.sources := cyclone_objects/binaries/audio/spike.c
svf~.class.sources := cyclone_objects/binaries/audio/svf.c
tanh~.class.sources := cyclone_objects/binaries/audio/tanh.c
tanx~.class.sources := cyclone_objects/binaries/audio/tanx.c
teeth~.class.sources := cyclone_objects/binaries/audio/teeth.c
togedge.class.sources := cyclone_objects/binaries/control/togedge.c
train~.class.sources := cyclone_objects/binaries/audio/train.c
trapezoid~.class.sources := cyclone_objects/binaries/audio/trapezoid.c
triangle~.class.sources := cyclone_objects/binaries/audio/triangle.c
zerox~.class.sources := cyclone_objects/binaries/audio/zerox.c
# NEW ones in cyclone0.3:
atodb~.class.sources := cyclone_objects/binaries/audio/atodb.c
cross~.class.sources := cyclone_objects/binaries/audio/cross.c
dbtoa~.class.sources := cyclone_objects/binaries/audio/dbtoa.c
degrade~.class.sources := cyclone_objects/binaries/audio/degrade.c
downsamp~.class.sources := cyclone_objects/binaries/audio/downsamp.c
equals~.class.sources := cyclone_objects/binaries/audio/equals.c
greaterthan~.class.sources := cyclone_objects/binaries/audio/greaterthan.c
greaterthaneq~.class.sources := cyclone_objects/binaries/audio/greaterthaneq.c
lessthan~.class.sources := cyclone_objects/binaries/audio/lessthan.c
lessthaneq~.class.sources := cyclone_objects/binaries/audio/lessthaneq.c
modulo~.class.sources := cyclone_objects/binaries/audio/modulo.c
notequals~.class.sources := cyclone_objects/binaries/audio/notequals.c
phaseshift~.class.sources := cyclone_objects/binaries/audio/phaseshift.c
rdiv~.class.sources := cyclone_objects/binaries/audio/rdiv.c
rminus~.class.sources := cyclone_objects/binaries/audio/rminus.c
round~.class.sources := cyclone_objects/binaries/audio/round.c
scale~.class.sources := cyclone_objects/binaries/audio/scale.c
thresh~.class.sources := cyclone_objects/binaries/audio/thresh.c
trunc~.class.sources := cyclone_objects/binaries/audio/trunc.c

# GUI:
comment.class.sources := cyclone_objects/binaries/control/comment.c

##################### CLASSES WITH DEPENDENCIES ##################################

# Control classes: ###############################################

hfile := shared/common/file.c
capture.class.sources := cyclone_objects/binaries/control/capture.c $(hfile)
mtr.class.sources := cyclone_objects/binaries/control/mtr.c $(hfile)
coll.class.sources := cyclone_objects/binaries/control/coll.c $(hfile)

hgrow := shared/common/grow.c
tosymbol.class.sources := cyclone_objects/binaries/control/tosymbol.c $(hgrow)
append.class.sources := cyclone_objects/binaries/control/append.c $(hgrow)
clip.class.sources := cyclone_objects/binaries/control/clip.c $(hgrow)
prepend.class.sources := cyclone_objects/binaries/control/prepend.c $(hgrow)
thresh.class.sources := cyclone_objects/binaries/control/thresh.c $(hgrow)
substitute.class.sources := cyclone_objects/binaries/control/substitute.c $(hgrow)
speedlim.class.sources := cyclone_objects/binaries/control/speedlim.c $(hgrow)
match.class.sources := cyclone_objects/binaries/control/match.c $(hgrow)
iter.class.sources := cyclone_objects/binaries/control/iter.c $(hgrow)
buddy.class.sources := cyclone_objects/binaries/control/buddy.c $(hgrow)
bondo.class.sources := cyclone_objects/binaries/control/bondo.c $(hgrow)
pv.class.sources := cyclone_objects/binaries/control/pv.c $(hgrow)

hrandfile := \
shared/control/rand.c \
shared/common/file.c
prob.class.sources := cyclone_objects/binaries/control/prob.c $(hrandfile)

hgui := shared/control/gui.c
active.class.sources := cyclone_objects/binaries/control/active.c $(hgui)
mousefilter.class.sources := cyclone_objects/binaries/control/mousefilter.c $(hgui)
mousestate.class.sources := cyclone_objects/binaries/control/mousestate.c $(hgui)

htree := shared/control/tree.c
offer.class.sources := cyclone_objects/binaries/control/offer.c $(htree)

htreefile := \
shared/control/tree.c \
shared/common/file.c
funbuff.class.sources := cyclone_objects/binaries/control/funbuff.c $(htreefile)

hrand := shared/control/rand.c
drunk.class.sources := cyclone_objects/binaries/control/drunk.c $(hrand)

hrandgrow := \
shared/control/rand.c \
shared/common/grow.c
urn.class.sources := cyclone_objects/binaries/control/urn.c $(hrandgrow)

hrandgrowfile := \
shared/control/rand.c \
shared/common/grow.c \
shared/common/file.c
table.class.sources := cyclone_objects/binaries/control/table.c $(hrandgrowfile)

hseq := \
shared/control/mifi.c \
shared/common/file.c \
shared/common/grow.c
seq.class.sources := cyclone_objects/binaries/control/seq.c $(hseq)

# New Dependency:

hmagicbit := shared/common/magicbit.c
    grab.class.sources := cyclone_objects/binaries/control/grab.c $(hmagicbit)

# Signal: #################################################################

sgrow := shared/common/grow.c
    frameaccum~.class.sources := cyclone_objects/binaries/audio/frameaccum.c $(sgrow)
    framedelta~.class.sources := cyclone_objects/binaries/audio/framedelta.c $(sgrow)

sfile := shared/common/file.c
    capture~.class.sources := cyclone_objects/binaries/audio/capture.c $(sfile)

# New Dependencies:

smagicbit := shared/common/magicbit.c
    cartopol~.class.sources := cyclone_objects/binaries/audio/cartopol.c $(smagicbit)
    delay~.class.sources := cyclone_objects/binaries/audio/delay.c $(smagicbit)
    plusequals~.class.sources := cyclone_objects/binaries/audio/plusequals.c $(smagicbit)
    minmax~.class.sources := cyclone_objects/binaries/audio/minmax.c $(smagicbit)
    poltocar~.class.sources := cyclone_objects/binaries/audio/poltocar.c $(smagicbit)
    matrix~.class.sources := cyclone_objects/binaries/audio/matrix.c $(smagicbit)
    sah~.class.sources := cyclone_objects/binaries/audio/sah.c $(smagicbit)
    gate~.class.sources := cyclone_objects/binaries/audio/gate.c $(smagicbit)
    selector~.class.sources := cyclone_objects/binaries/audio/selector.c $(smagicbit)
    kink~.class.sources := cyclone_objects/binaries/audio/kink.c $(smagicbit)
    vectral~.class.sources := cyclone_objects/binaries/audio/vectral.c $(smagicbit)
    bitand~.class.sources := cyclone_objects/binaries/audio/bitand.c $(smagicbit)
    bitnot~.class.sources := cyclone_objects/binaries/audio/bitnot.c $(smagicbit)
    bitor~.class.sources := cyclone_objects/binaries/audio/bitor.c $(smagicbit)
    bitsafe~.class.sources := cyclone_objects/binaries/audio/bitsafe.c $(smagicbit)
    bitshift~.class.sources := cyclone_objects/binaries/audio/bitshift.c $(smagicbit)
    bitxor~.class.sources := cyclone_objects/binaries/audio/bitxor.c $(smagicbit)
# GUI
    scope~.class.sources := cyclone_objects/binaries/audio/scope.c $(smagicbit)

scybuf := shared/signal/cybuf.c
    buffir~.class.sources := cyclone_objects/binaries/audio/buffir.c $(scybuf)
    lookup~.class.sources := cyclone_objects/binaries/audio/lookup.c $(scybuf)
    index~.class.sources := cyclone_objects/binaries/audio/index.c $(scybuf)
    peek~.class.sources := cyclone_objects/binaries/audio/peek.c $(scybuf)
    poke~.class.sources := cyclone_objects/binaries/audio/poke.c $(scybuf)
    record~.class.sources := cyclone_objects/binaries/audio/record.c $(scybuf)
    wave~.class.sources := cyclone_objects/binaries/audio/wave.c $(scybuf)

smagicscybuff := \
shared/common/magicbit.c \
shared/signal/cybuf.c
    play~.class.sources := cyclone_objects/binaries/audio/play.c $(smagicscybuff)

# Cyclone Library: ################################################

magicbit := shared/common/magicbit.c
    cyclone.class.sources := cyclone_objects/binaries/cyclone_lib.c $(magicbit)

#######################################################################
                        ## END OF CYCLONE CLASSES ##
#######################################################################

### datafiles #########################################################

datafiles = \
$(wildcard cyclone_objects/abstractions/*.pd) \
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
    ln -s -f append.$(extension) Append.$(extension); \
    ln -s -f append-help.pd Append-help.pd; \
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
