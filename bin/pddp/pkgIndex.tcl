proc LoadPddp { version dir } {
    namespace eval ::pddp {}
    set ::pddp::theVersion $version
    set ::pddp::theDir $dir
    source [file join $dir pddpboot.tcl]
}

set version "0.1.0.2"

package ifneeded pddp $version [list LoadPddp $version $dir]
