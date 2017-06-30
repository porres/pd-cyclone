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

##################### CLASSES WITH DEPENDENCIES ##################################

# Control classes: ###############################################

hfile := \
shared/hammer/file.c \
shared/common/loud.c \
shared/common/os.c \
shared/common/fitter.c \
shared/unstable/forky.c

# hfile classes
coll.class.sources := cyclone_src/binaries/control/coll.c $(hfile)

#######################################################################
                        ## END OF CYCLONE CLASSES ##
#######################################################################

datafiles = \
$(wildcard cyclone_src/abstractions/*.pd) \
$(wildcard documentation/help_files/*.pd) \
$(wildcard documentation/extra_files/*.*) \
LICENSE.txt \
README.txt \

# pthreadGC-3.dll is required for Windows installation. It can be found in
# the MinGW directory (usually C:\MinGW\bin) directory and should be
# copied to the current directory before installation or packaging.

ifeq (MINGW,$(findstring MINGW,$(uname)))
datafiles += maintenance/windows_dll/pthreadGC-3.dll
datafiles += maintenance/windows_dll/libgcc_s_dw2-1.dll
endif

### pd-lib-builder ######################################################

include pd-lib-builder/Makefile.pdlibbuilder
