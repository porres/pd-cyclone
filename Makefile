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
asin.class.sources := cyclone_src/binaries/control/asin.c
bucket.class.sources := cyclone_src/binaries/control/bucket.c
cartopol.class.sources := cyclone_src/binaries/control/cartopol.c
cosh.class.sources := cyclone_src/binaries/control/cosh.c
flush.class.sources := cyclone_src/binaries/control/flush.c
forward.class.sources := cyclone_src/binaries/control/forward.c
fromsymbol.class.sources := cyclone_src/binaries/control/fromsymbol.c
funnel.class.sources := cyclone_src/binaries/control/funnel.c
histo.class.sources := cyclone_src/binaries/control/histo.c
listfunnel.class.sources := cyclone_src/binaries/control/listfunnel.c
mean.class.sources := cyclone_src/binaries/control/mean.c
midiflush.class.sources := cyclone_src/binaries/control/midiflush.c
midiformat.class.sources := cyclone_src/binaries/control/midiformat.c
midiparse.class.sources := cyclone_src/binaries/control/midiparse.c
next.class.sources := cyclone_src/binaries/control/next.c
onebang.class.sources := cyclone_src/binaries/control/onebang.c
past.class.sources := cyclone_src/binaries/control/past.c
peak.class.sources := cyclone_src/binaries/control/peak.c
poltocar.class.sources := cyclone_src/binaries/control/poltocar.c
sinh.class.sources := cyclone_src/binaries/control/sinh.c
split.class.sources := cyclone_src/binaries/control/split.c
spray.class.sources := cyclone_src/binaries/control/spray.c
sustain.class.sources := cyclone_src/binaries/control/sustain.c
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

hfile := \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/common/fitter.c \
shared/unstable/forky.c

hfitter := \
shared/common/loud.c \
shared/common/fitter.c


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

htreefile := \
shared/hammer/tree.c \
shared/hammer/file.c \
shared/common/os.c \
shared/unstable/forky.c 

hloud := shared/common/loud.c

# hfitter classes
bangbang.class.sources := cyclone_src/binaries/control/bangbang.c $(hfitter)
counter.class.sources := cyclone_src/binaries/control/counter.c $(hfitter)
cycle.class.sources := cyclone_src/binaries/control/cycle.c $(hfitter)
decode.class.sources := cyclone_src/binaries/control/decode.c $(hfitter)
gate.class.sources := cyclone_src/binaries/control/gate.c $(hfitter)
linedrive.class.sources := cyclone_src/binaries/control/linedrive.c $(hfitter)
maximum.class.sources := cyclone_src/binaries/control/maximum.c $(hfitter)
minimum.class.sources := cyclone_src/binaries/control/minimum.c $(hfitter)
switch.class.sources := cyclone_src/binaries/control/switch.c $(hfitter)

# hfile classes
capture.class.sources := cyclone_src/binaries/control/capture.c $(hfile)
coll.class.sources := cyclone_src/binaries/control/coll.c $(hfile)
mtr.class.sources := cyclone_src/binaries/control/mtr.c $(hfile)

# hgui classes
active.class.sources := cyclone_src/binaries/control/active.c $(hgui)
mousefilter.class.sources := cyclone_src/binaries/control/mousefilter.c $(hgui)
mousestate.class.sources := cyclone_src/binaries/control/mousestate.c $(hgui)

# hgrow classes
bondo.class.sources := cyclone_src/binaries/control/bondo.c $(hgrow)
buddy.class.sources := cyclone_src/binaries/control/buddy.c $(hgrow)
clip.class.sources := cyclone_src/binaries/control/clip.c $(hgrow)
iter.class.sources := cyclone_src/binaries/control/iter.c $(hgrow)
match.class.sources := cyclone_src/binaries/control/match.c $(hgrow)
speedlim.class.sources := cyclone_src/binaries/control/speedlim.c $(hgrow)
substitute.class.sources := cyclone_src/binaries/control/substitute.c $(hgrow)
thresh.class.sources := cyclone_src/binaries/control/thresh.c $(hgrow)
tosymbol.class.sources := cyclone_src/binaries/control/tosymbol.c $(hgrow)
pv.class.sources := cyclone_src/binaries/control/pv.c $(hgrow)

# hgrowfitter classes
append.class.sources := cyclone_src/binaries/control/append.c $(hgrowfitter)
prepend.class.sources := cyclone_src/binaries/control/prepend.c $(hgrowfitter)

# hloud classes
anal.class.sources := cyclone_src/binaries/control/anal.c $(hloud)
borax.class.sources := cyclone_src/binaries/control/borax.c $(hloud)
decide.class.sources := cyclone_src/binaries/control/decide.c $(hloud)
spell.class.sources := cyclone_src/binaries/control/spell.c $(hloud)
sprintf.class.sources := cyclone_src/binaries/control/sprintf.c $(hloud)
togedge.class.sources := cyclone_src/binaries/control/togedge.c $(hloud)
# GUI:
comment.class.sources := cyclone_src/binaries/control/comment.c $(hloud)

# Single cases:

drunk.class.sources := cyclone_src/binaries/control/drunk.c $(hrand)

prob.class.sources := cyclone_src/binaries/control/prob.c $(hrandfile)

urn.class.sources := cyclone_src/binaries/control/urn.c $(hrandgrow)

table.class.sources := cyclone_src/binaries/control/table.c $(hrandgrowfile)

seq.class.sources := cyclone_src/binaries/control/seq.c $(hseq)

offer.class.sources := cyclone_src/binaries/control/offer.c $(htree)

funbuff.class.sources := cyclone_src/binaries/control/funbuff.c $(htreefile)


# Signal: #################################################################

sgrow := shared/common/grow.c
    frameaccum~.class.sources := cyclone_src/binaries/signal/frameaccum.c $(sgrow)
    framedelta~.class.sources := cyclone_src/binaries/signal/framedelta.c $(sgrow)

sfile := \
shared/hammer/file.c \
shared/common/os.c
    capture~.class.sources := cyclone_src/binaries/signal/capture.c $(sfile)

# New Dependencies: #################################################################

#GUI
sfittermagic := \
shared/magicbit.c \
shared/common/fitter.c
    scope~.class.sources := cyclone_src/binaries/signal/scope.c $(sfittermagic)

magicbit := shared/magicbit.c
    grab.class.sources := cyclone_src/binaries/control/grab.c $(magicbit) # only "fragile"
    cartopol~.class.sources := cyclone_src/binaries/signal/cartopol.c $(magicbit) # only one "fragile" + "forky"
    delay~.class.sources := cyclone_src/binaries/signal/delay.c $(magicbit)
    plusequals~.class.sources := cyclone_src/binaries/signal/plusequals.c $(magicbit)
    minmax~.class.sources := cyclone_src/binaries/signal/minmax.c $(magicbit)
    poltocar~.class.sources := cyclone_src/binaries/signal/poltocar.c $(magicbit)
    matrix~.class.sources := cyclone_src/binaries/signal/matrix.c $(magicbit)
    sah~.class.sources := cyclone_src/binaries/signal/sah.c $(magicbit)
    gate~.class.sources := cyclone_src/binaries/signal/gate.c $(magicbit)
    selector~.class.sources := cyclone_src/binaries/signal/selector.c $(magicbit)
    kink~.class.sources := cyclone_src/binaries/signal/kink.c $(magicbit)
    vectral~.class.sources := cyclone_src/binaries/signal/vectral.c $(magicbit) #
    cyclone.class.sources := cyclone_src/binaries/sub_lib_cyclone.c $(magicbit) # cyclone sub-library
    bitand~.class.sources := cyclone_src/binaries/signal/bitand.c $(magicbit)
    bitnot~.class.sources := cyclone_src/binaries/signal/bitnot.c $(magicbit)
    bitor~.class.sources := cyclone_src/binaries/signal/bitor.c $(magicbit)
    bitsafe~.class.sources := cyclone_src/binaries/signal/bitsafe.c $(magicbit)
    bitshift~.class.sources := cyclone_src/binaries/signal/bitshift.c $(magicbit)
    bitxor~.class.sources := cyclone_src/binaries/signal/bitxor.c $(magicbit)

scybuf := shared/cybuf.c
    buffir~.class.sources := cyclone_src/binaries/signal/buffir.c $(scybuf)
    lookup~.class.sources := cyclone_src/binaries/signal/lookup.c $(scybuf)
    index~.class.sources := cyclone_src/binaries/signal/index.c $(scybuf)
    peek~.class.sources := cyclone_src/binaries/signal/peek.c $(scybuf)
    poke~.class.sources := cyclone_src/binaries/signal/poke.c $(scybuf)
    record~.class.sources := cyclone_src/binaries/signal/record.c $(scybuf)
    wave~.class.sources := cyclone_src/binaries/signal/wave.c $(scybuf)

magicscybuff := \
shared/magicbit.c \
shared/cybuf.c
    play~.class.sources := cyclone_src/binaries/signal/play.c $(magicscybuff)

#######################################################################
                        ## END OF CYCLONE CLASSES ##
#######################################################################

### datafiles #########################################################

# ifdef PDL2ORK
# datafiles = \
# $(wildcard documentation/purr_help/*.pd) \ # instead of documentation/help_files/*.pd
# $(wildcard documentation/purr_help/abs_GUI/*.pd) \ # instead of cyclone_src/abstractions/*.pd
# else

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
