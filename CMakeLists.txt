cmake_minimum_required (VERSION 3.0)
project (cyclone C)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_SINGLE_LIBRARY "Compile everything into one single library." OFF)

set(LIBPD_ROOT_PATH "" CACHE PATH "Path to libpd library sources")
if (NOT LIBPD_ROOT_PATH)
    message(FATAL_ERROR "Please provide a path to the libpd library source files.")
endif()

set(LIBPD_LIB_PATH "" CACHE PATH "Path to libpd library")
if (NOT LIBPD_LIB_PATH)
    message(FATAL_ERROR "Please provide a path to the libpd library.")
endif()

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/libs CACHE PATH "Installation path")

if (WIN32)
    set(CMAKE_THREAD_LIBS_INIT CACHE PATH "Path to pthreads library")
    set(PTHREADS_INCLUDE_DIR CACHE PATH "Path to the pthreads library header files")
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
    add_definitions(-DHAVE_UNISTD_H=1)
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
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${CMAKE_INSTALL_PREFIX})
endforeach()

include_directories(${LIBPD_ROOT_PATH}/pure-data/src)
include_directories(${PTHREADS_INCLUDE_DIR})
include_directories(shared)

find_package(Threads REQUIRED)

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

# Add main cyclone library

set(SETUP_DECLS "")
set(SETUP_CALLS "")
if(BUILD_SINGLE_LIBRARY)
    string(APPEND SETUP_DECLS "// control classes\n\n")
    string(APPEND SETUP_CALLS "    // control classes\n\n")
    foreach(NAME ${CONTROL_CLASSES})
        string(APPEND SETUP_DECLS "void ${NAME}_setup();\n")
        string(APPEND SETUP_CALLS "    ${NAME}_setup();\n")
    endforeach(NAME)
    string(APPEND SETUP_DECLS "\n// signal classes\n\n")
    string(APPEND SETUP_CALLS "\n    // signal classes\n\n")
    foreach(NAME ${SIGNAL_CLASSES})
        string(APPEND SETUP_DECLS "void ${NAME}_tilde_setup();\n")
        string(APPEND SETUP_CALLS "    ${NAME}_tilde_setup();\n")
    endforeach(NAME)
endif(BUILD_SINGLE_LIBRARY)

configure_file(cyclone_objects/binaries/single_lib.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/cyclone_objects/binaries/single_lib.c)

add_library(cyclone cyclone_objects/binaries/cyclone_lib.c
    ${CMAKE_CURRENT_BINARY_DIR}/cyclone_objects/binaries/single_lib.c
)

# Add libraries for objects

if(BUILD_SHARED_LIBS AND NOT BUILD_SINGLE_LIBRARY)
    set(LIB_BUILD_MODE SHARED)
else()
    set(LIB_BUILD_MODE STATIC)
endif()

if(BUILD_SINGLE_LIBRARY)
    add_definitions(-DCYCLONE_SINGLE_LIBRARY=1)
endif()

foreach(NAME ${CONTROL_CLASSES})
    add_library(${NAME} ${LIB_BUILD_MODE} cyclone_objects/binaries/control/${NAME}.c)
endforeach(NAME)

foreach(NAME ${SIGNAL_CLASSES})
    add_library(signal_${NAME} ${LIB_BUILD_MODE} cyclone_objects/binaries/audio/${NAME}.c)
    set_target_properties(signal_${NAME} PROPERTIES OUTPUT_NAME ${NAME}~)
endforeach(NAME)

# Link objects to main library if building a single library

if(BUILD_SINGLE_LIBRARY)
    foreach(NAME ${CONTROL_CLASSES})
        target_link_libraries(cyclone ${NAME})
    endforeach(NAME)
    foreach(NAME ${SIGNAL_CLASSES})
        target_link_libraries(cyclone signal_${NAME})
    endforeach(NAME)
endif(BUILD_SINGLE_LIBRARY)

# Install

install(DIRECTORY
    cyclone_objects/abstractions/
    documentation/help_files/
    documentation/extra_files/
    DESTINATION .
    FILES_MATCHING PATTERN "*.pd"
)

install(FILES
    LICENSE.txt
    README.pdf
    DESTINATION .
)

if (MINGW)
    # pthreadGC-3.dll is required for Windows installation. It can be found in
    # the MinGW directory (usually C:\MinGW\bin) directory and should be
    # copied to the current directory before installation or packaging.

    install(FILES
        maintenance/windows_dll/pthreadGC-3.dll
        maintenance/windows_dll/libgcc_s_dw2-1.dll
        DESTINATION .
    )
endif(MINGW)

# Install upper case aliases on Linux
if(UNIX AND NOT APPLE)
    if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
        set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}")
    endif()

    macro(create_symlinks old new)
        if (NOT BUILD_SINGLE_LIBRARY)
            install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \
                ${CMAKE_INSTALL_PREFIX}/lib${old}${CMAKE_DEBUG_POSTFIX}.so \
                ${CMAKE_INSTALL_PREFIX}/lib${new}${CMAKE_DEBUG_POSTFIX}.so)")
        endif()
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \
            ${CMAKE_INSTALL_PREFIX}/${old}-help.pd ${CMAKE_INSTALL_PREFIX}/${new}-help.pd)")
    endmacro()

    create_symlinks(append Append)
    create_symlinks(bucket Bucket)
    create_symlinks(clip Clip)
    create_symlinks(decode Decode)
    create_symlinks(histo Histo)
    create_symlinks(mousestate MouseState)
    create_symlinks(peak Peak)
    create_symlinks(table Table)
    create_symlinks(togedge TogEdge)
    create_symlinks(trough Trough)
    create_symlinks(uzi Uzi)

    create_symlinks(clip~ Clip~)
    create_symlinks(line~ Line~)
    create_symlinks(scope~ Scope~)
    create_symlinks(snapshot~ Snapshot~)
endif(UNIX AND NOT APPLE)
