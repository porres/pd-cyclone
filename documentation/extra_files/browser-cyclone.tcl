# generate menu tree for native objects for the canvas right click popup
# code by Porres and Seb Shader

package require pd_menus

namespace eval category_cyclone_menu {
}

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
        {cyclone
            {lib
                {cyclone}}
            {max
                {accum acos acosh active anal append asin asinh atanh atodb bangbang bondo borax bucket buddy capture cartopol clip coll comment cosh counter cycle dbtoa decide decode drunk flush forward fromsymbol funbuff funnel gate grab histo iter join linedrive listfunnel loadmess match maximum mean midiflush midiformat midiparse minimum mousefilter mousestate mtr next offer onebang pak past peak poltocar pong prepend prob pv rdiv rminus round scale seq sinh speedlim spell split spray sprintf substitute sustain switch table tanh thresh togedge tosymbol trough universal unjoin urn uzi xbendin xbendin2 xbendout xbendout2 xnotein xnoteout zl}}
            {msp
                {acos~ acosh~ allpass~ asin~ asinh~ atan~ atan2~ atanh~ atodb~ average~ avg~ bitand~ bitnot~ bitor~ bitsafe~ bitshift~ bitxor~ buffer~ buffir~ capture~ cartopol~ change~ click~ clip~ comb~ cosh~ cosx~ count~ cross~ curve~ cycle~ dbtoa~ degrade~ delay~ delta~ deltaclip~ downsamp~ edge~ equals~ frameaccum~ framedelta~ gate~ greaterthan~ greaterthaneq~ index~ kink~ lessthan~ lessthaneq~ line~ lookup~ lores~ matrix~ maximum~ minimum~ minmax~ modulo~ mstosamps~ notequals~ number~ onepole~ overdrive~ peakamp~ peek~ phaseshift~ phasewrap~ pink~ play~ plusequals~ poke~ poltocar~ pong~ pow~ rampsmooth~ rand~ rdiv~ record~ reson~ rminus~ round~ sah~ sampstoms~ scale~ scope_dialog~ scope~ selector~ sinh~ sinx~ slide~ snapshot~ spike~ svf~ tanh~ tanx~ teeth~ thresh~ train~ trapezoid~ triangle~ trunc~ vectral~ wave~ zerox~}}
        }
    }
    return $menutree
}

proc category_cyclone_menu::create {cmdstring code result op} {
    set mymenu [lindex $cmdstring 1]
    set x [lindex $cmdstring 3]
    set y [lindex $cmdstring 4]
    set menutree [load_menutree]
    $mymenu add separator
    foreach categorylist $menutree {
        set category [lindex $categorylist 0]
        menu $mymenu.$category
        $mymenu add cascade -label $category -menu $mymenu.$category
        foreach subcategorylist [lrange $categorylist 1 end] {
            set subcategory [lindex $subcategorylist 0]
            menu $mymenu.$category.$subcategory
            $mymenu.$category add cascade -label $subcategory -menu $mymenu.$category.$subcategory
            foreach item [lindex $subcategorylist end] {
                # replace the normal dash with a Unicode minus so that Tcl does not
                # interpret the dash in the -label to make it a separator
                $mymenu.$category.$subcategory add command \
                    -label [regsub -all {^\-$} $item {−}] \
                    -command "menu_send_cyclone_obj \$::focused_window $x $y {$item}"
            }
        }
    }
}

trace add execution ::pdtk_canvas::create_popup leave category_cyclone_menu::create
