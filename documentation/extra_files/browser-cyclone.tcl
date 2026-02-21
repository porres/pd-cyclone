# Gener# Generate menu tree for native objects for the canvas right click popup
# Code by Porres and Seb Shader

package require pd_menus

namespace eval category_cyclone_menu {}

proc menu_send_cyclone_obj {w x y item} {
    if {$item eq "cyclone"} {  
        pdsend "$w obj $x $y $item"
    } else {
        pdsend "$w obj $x $y cyclone/$item"
        set abslist {buffer~ number~}
        foreach abstraction $abslist {
            if {$item eq $abstraction} {  
                pdsend "pd-$item.pd loadbang"
                break
            }
        }
    } 
}

# set nested list
proc category_cyclone_menu::load_menutree {} {
    set menutree { 
        {max
            {math
                {trig
                    {acos acosh asin asinh atanh cosh sinh tanh}}
                {accum atodb cartopol clip counter dbtoa maximum mean minimum poltocar pong rdiv rminus round scale}}
            {midi
                {borax flush midiflush midiformat midiparse seq sustain xnotein xnoteout}}
            {random
                {decide drunk prob urn}}
            {data\ storage
                {coll funbuff histo mtr table}}
            {data\ management
                {list
                    {iter join listfunnel unjoin zl zl.change zl.compare zl.delace zl.ecils zl.filter zl.group zl.indexmap zl.iter zl.join zl.lace zl.len zl.lookup zl.median zl.mth zl.nth zl.queue zl.reg zl.rev zl.rot zl.scramble zl.sect zl.slice zl.sort zl.stack zl.stream zl.sub zl.sum zl.swap zo.thin zl.union}}
                {anal bangbang bondo bucket buddy capture cycle decode fromsymbol funnel gate grab loadmess match next offer onebang pak past peak prepend pv speedlim split spray substitute switch thresh togedge tosymbol trough  uzi}}
            {others
                {active linedrive mousefilter mousestate spell sprintf universal}}
        }
        {msp
            {math
                {trig
                    {acos~ acosh~ asin~ asinh~ atan~ atan2~ atanh~ cosh~ cosx~ sinh~ sinx~ tanh~ tanx~}}
                {operators
                    {bitwise
                        {bitand~ bitnot~ bitor~ bitsafe~ bitshift~ bitxor~}}
                    {equals~ greaterthan~ greaterthaneq~ lessthan~ lessthaneq~ modulo~ notequals~ plusequals~ rdiv~ rminus~}}
                {atodb~ average~ avg~ cartopol~ dbtoa~ delta~ mstosamps~ poltocar~ pong~ round~ sampstoms~ scale~ trunc~}}
            {fx
                {degrade~ delay~ downsamp~ overdrive~}}
            {filters
                {allpass~ buffir~ comb~ cross~ deltaclip~ lores~ onepole~ phaseshift~ rampsmooth~ reson~ slide~ svf~ teeth~}}
            {synthesis
                {cycle~ curve~ kink~ line~ lookup~ pink~ rand~ train~ trapezoid~ triangle~ wave~}}
            {play/buffer
                {buffer~ index~ peek~ play~ poke~ record~}}
            {analysis
                {capture~ change~ edge~ minmax~ peakamp~ sah~ snapshot~ spike~ thresh~ zerox~}}
            {others
                {click~ count~ frameaccum~ framedelta~ gate~ matrix~ phasewrap~ scope~ selector~ vectral~}}
        }
        {lib
            {cyclone}}
        {deprecated
            {append clip~ comment forward maximum~ minimum~ number~ pow~ xbendin xbendin2 xbendout xbendout2}}
    }
#   ::pdwindow::post "===== RAW MENUTREE =====\n"
#    set index 0
#    foreach item $menutree {
#        ::pdwindow::post "Item $index: $item\n"
#        incr index
#    }
    return $menutree
}

proc category_cyclone_menu::build_menu {parent_menu node x y} {
    set name [lindex $node 0]
    
#    ::pdwindow::post "DEBUG: node has [llength $node] elements\n"
#    for {set i 0} {$i < [llength $node]} {incr i} {
#        ::pdwindow::post "  node index $i = [lindex $node $i]\n"
#    }
    
#    ::pdwindow::post "\n"
#    ::pdwindow::post "=== BUILDING MENU ===\n"
#    ::pdwindow::post "Node name: $name\n"
#    ::pdwindow::post "Raw node: $node\n"
    
    # Create menu for this node
    set current_menu $parent_menu.$name
#    ::pdwindow::post "Creating menu ---- \n"
    menu $current_menu
    $parent_menu add cascade -label $name -menu $current_menu
    
    # Process each remaining element directly (these are submenus or item lists)
    set remaining [lrange $node 1 end]
#    ::pdwindow::post "Remaining elements to process: $remaining\n"
#    ::pdwindow::post "Number of remaining elements: [llength $remaining]\n"
    
    set count 0
    foreach element $remaining {
#        ::pdwindow::post "\n  Processing remaining element $count:\n"
#        ::pdwindow::post "  Element: $element\n"
#        ::pdwindow::post "  Element length: [llength $element]\n"
        
        # Debug: show what the element looks like as a list
#        ::pdwindow::post "  Element as list: [list $element]\n"
        
        # Debug: check each part of the element
#        for {set j 0} {$j < [llength $element]} {incr j} {
#            ::pdwindow::post "    element part $j = [lindex $element $j]\n"
#            ::pdwindow::post "    part $j length = [llength [lindex $element $j]]\n"
#        }
        
        # Debug: check if second element exists and is a list
        set second [lindex $element 1]
#        ::pdwindow::post "  second element: $second\n"
#        ::pdwindow::post "  second element length: [llength $second]\n"
#        ::pdwindow::post "  is second element a list? [expr {[llength $second] > 1}]\n"
        
        # Debug: check the structure more deeply
#        if {[llength $element] > 1} {
#            ::pdwindow::post "  first of second: [lindex $second 0]\n"
#        }
        
        # Determine if this element is a submenu or item list
        # A submenu has: first part is a name (string), remaining parts are lists
        set is_submenu 0
        if {[llength $element] >= 2} {
            set is_submenu 1
            for {set j 1} {$j < [llength $element]} {incr j} {
                if {[llength [lindex $element $j]] <= 1} {
                    set is_submenu 0
                    break
                }
            }
        }
        
        if {$is_submenu} {
#            ::pdwindow::post "  -> Element is a submenu (has name + sublists)\n"
            build_menu $current_menu $element $x $y
        } elseif {[llength $element] > 1} {
            # Element is a list of items - add each as command
#            ::pdwindow::post "  -> Element is an item list\n"
            foreach item $element {
#                ::pdwindow::post "    Adding item: $item\n"
                $current_menu add command \
                    -label [regsub -all {^\-$} $item {−}] \
                    -command "menu_send_cyclone_obj \$::focused_window $x $y {$item}"
            }
        } else {
            # Element is a single item
#            ::pdwindow::post "  -> Adding as command: $element\n"
            $current_menu add command \
                -label [regsub -all {^\-$} $element {−}] \
                -command "menu_send_cyclone_obj \$::focused_window $x $y {$element}"
        }
        incr count
    }
#    ::pdwindow::post "=== DONE WITH $name ===\n\n"
}

proc category_cyclone_menu::create {cmdstring code result op} {
    if {!$::category_cyclone_menu::enabled} { return }
    
    set mymenu [lindex $cmdstring 1]
    set x [lindex $cmdstring 3]
    set y [lindex $cmdstring 4]
    set menutree [load_menutree]
    
    $mymenu add separator
    
    # Create the main cyclone menu
    set category "cyclone"
    menu $mymenu.$category
    $mymenu add cascade -label $category -menu $mymenu.$category
    
    # Process each top-level item
    foreach item $menutree {
        build_menu $mymenu.$category $item $x $y
    }
}

proc category_cyclone_menu::read_browser_cfg {} {
    set ::category_cyclone_menu::enabled [::pd_guiprefs::read cyclone_browser_enabled]
    if {$::category_cyclone_menu::enabled eq ""} {
        set ::category_cyclone_menu::enabled 1 
    }
    if {$::category_cyclone_menu::enabled == 1} {
        ::pdwindow::post "Cyclone browser plugin is Enabled\n"
    } else {
        ::pdwindow::post "Cyclone browser plugin is Disabled\n"
    }
}

proc category_cyclone_menu::write_config {{filename browser.cfg}} {
    ::pd_guiprefs::write cyclone_browser_enabled $::category_cyclone_menu::enabled
}

trace add execution ::pdtk_canvas::create_popup leave category_cyclone_menu::create

proc category_cyclone_menu::add_menu_entry {} {
    .preferences add separator
    .preferences add checkbutton \
        -label [_ "Enable Cyclone Browser"] \
        -variable ::category_cyclone_menu::enabled \
        -command {category_cyclone_menu::write_config}
}

category_cyclone_menu::read_browser_cfg
category_cyclone_menu::add_menu_entry
