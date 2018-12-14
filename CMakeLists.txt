cmake_minimum_required (VERSION 2.8.11)
project (cyclone C)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

option(LIBPD_ROOT_PATH "Path to libpd library sources")
if (NOT LIBPD_ROOT_PATH)
    message(FATAL_ERROR "Please provide a path to the libpd library source files.")
endif()

option(LIBPD_LIB_PATH "Path to libpd library")
if (NOT LIBPD_LIB_PATH)
    message(FATAL_ERROR "Please provide a path to the libpd library.")
endif()


if (WIN32)
    option(CMAKE_THREAD_LIBS_INIT "Path to pthreads library")
    option(PTHREADS_INCLUDE_DIR "Path to the pthreads library header files")
    # We need pthreads.
    # Please provide the path to the pthreads library include path and
    # the path to the pthread library by specifying the following arguments
    # on the CMake command-line:
    # -DCMAKE_THREAD_LIBS_INIT:PATH=<path to library, either MSVC (for example pthreadVC2.lib) or GNUC version>
    # -DPTHREADS_INCLUDE_DIR:PATH=<path to the pthread header files>
    if (NOT CMAKE_THREAD_LIBS_INIT OR NOT PTHREADS_INCLUDE_DIR)
        message(FATAL_ERROR "Please provide a path to the pthreads library and its headers.")
    endif()
    set(CMAKE_THREAD_PREFER_PTHREAD ON)
endif()

if(WIN32)
    # Use Windows APIs compatible with most versions
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWINVER=0x502 -DWIN32 -D_WIN32")
endif()
if (MSVC)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DHAVE_STRUCT_TIMESPEC")
    add_definitions("/D_CRT_SECURE_NO_WARNINGS /wd4091 /wd4996")
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        # Select appropriate long int type on 64-bit Windows
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DPD_LONGINTTYPE=\"long long\"")
    endif()
else()
    compile_definitions(HAVE_UNISTD_H=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -ffast-math -funroll-loops -fomit-frame-pointer -O3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g -O0")
    if(NOT APPLE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
    endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MACOSX_RPATH ON)

foreach (_BUILD_TYPE RELEASE DEBUG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${PROJECT_SOURCE_DIR}/libs)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${PROJECT_SOURCE_DIR}/libs)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${PROJECT_SOURCE_DIR}/libs)
endforeach()

include_directories(${LIBPD_ROOT_PATH}/pure-data/src)
include_directories(${PTHREADS_INCLUDE_DIR})
include_directories(shared)

find_package(Threads REQUIRED)

add_library(cyclone_shared STATIC
    shared/common/file.h
    shared/common/file.c
    shared/common/grow.h
    shared/common/grow.c
    shared/common/magicbit.h
    shared/common/magicbit.c
    shared/control/gui.h
    shared/control/gui.c
    shared/control/mifi.h
    shared/control/mifi.c
    shared/control/rand.h
    shared/control/rand.c
    shared/control/tree.h
    shared/control/tree.c
    shared/signal/cybuf.h
    shared/signal/cybuf.c
)

link_libraries(${LIBPD_LIB_PATH})
link_libraries(${CMAKE_THREAD_LIBS_INIT})
link_libraries(cyclone_shared)

set(CONTROL_CLASSES
    accum
    acos
    anal
    asin
    bangbang
    borax
    bucket
    cartopol
    counter
    cosh
    cycle
    decide
    decode
    flush
    forward
    fromsymbol
    funnel
    gate
    histo
    listfunnel
    linedrive
    maximum
    mean
    midiflush
    midiformat
    midiparse
    minimum
    next
    onebang
    past
    peak
    poltocar
    sinh
    spell
    split
    spray
    sprintf
    sustain
    switch
    tanh
    togedge
    trough
    universal
    unjoin
    uzi
    xbendin
    xbendin2
    xbendout
    xbendout2
    xnotein
    xnoteout
    zl
    # new ones in cyclone 0.3
    acosh
    asinh
    atanh
    atodb
    dbtoa
    join
    # FIXME loadmess needs sys_noloadbang
    pong
    pak
    rdiv
    rminus
    round
    scale

    #GUI
    comment

    # classes with dependencies
    capture
    mtr
    coll
    tosymbol
    append
    clip
    prepend
    thresh
    substitute
    speedlim
    match
    iter
    buddy
    bondo
    pv
    prob
    active
    mousefilter
    mousestate
    offer
    funbuff
    drunk
    urn
    table
    seq
    grab
)

if (MSVC)
    # Remove controls that are not compatible with MSVC
    list(REMOVE_ITEM CONTROL_CLASSES
    funbuff # needs POSIX libgen.h
    round # needs alloca
    pong # needs alloca
    )
endif(MSVC)

set(SIGNAL_CLASSES
    acos
    acosh
    allpass
    asin
    asinh
    atan
    atan2
    atanh
    average
    avg
    change
    click
    clip
    cosh
    cosx
    count
    comb
    curve
    cycle
    delta
    deltaclip
    edge
    line
    lores
    maximum
    minimum
    mstosamps
    onepole
    overdrive
    peakamp
    phasewrap
    pink
    pong
    pow
    rampsmooth
    rand
    reson
    sampstoms
    sinh
    sinx
    slide
    snapshot
    spike
    svf
    tanh
    tanx
    teeth
    train
    trapezoid
    triangle
    zerox
    # new in cyclone 0.3
    atodb
    cross
    dbtoa
    degrade
    downsamp
    equals
    greaterthan
    greaterthaneq
    lessthan
    lessthaneq
    modulo
    notequals
    phaseshift
    rdiv
    rminus
    round
    scale
    thresh
    trunc

    # classes with dependencies
    frameaccum
    framedelta
    capture
    cartopol
    delay
    plusequals
    minmax
    poltocar
    matrix
    sah
    gate
    selector
    kink
    vectral
    bitand
    bitnot
    bitor
    bitsafe
    bitshift
    bitxor
    # GUI
    scope
    buffir
    lookup
    index
    peek
    poke
    record
    wave
    play
)

add_library(cyclone cyclone_objects/binaries/cyclone_lib.c)

foreach(NAME ${CONTROL_CLASSES})
    add_library(${NAME} cyclone_objects/binaries/control/${NAME}.c)
endforeach(NAME)

foreach(NAME ${SIGNAL_CLASSES})
    add_library(signal_${NAME} cyclone_objects/binaries/audio/${NAME}.c)
    set_target_properties(signal_${NAME} PROPERTIES OUTPUT_NAME ${NAME}~)
endforeach(NAME)

