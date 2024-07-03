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
            {max\ trig
                {acos acosh asin asinh atanh cosh sinh tanh}}   
            {max\ math
                {accum atodb cartopol clip counter dbtoa maximum mean minimum poltocar pong rdiv rminus round scale}}
            {max\ midi
                {borax flush midiflush midiformat midiparse seq sustain xnotein xnoteout}}
            {max\ random
                {decide drunk prob urn}}
            {max\ data\ storage
                {coll funbuff histo mtr table}}
            {max\ data\ management
                {anal bangbang bondo bucket buddy capture cycle decode fromsymbol funnel gate grab iter join listfunnel loadmess match next offer onebang pak past peak prepend pv speedlim split spray substitute switch thresh togedge tosymbol trough unjoin uzi zl}}
            {max\ others
                {active linedrive mousefilter mousestate spell sprintf universal}}
            {msp\ trig
                {acos~ acosh~ asin~ asinh~ atan~ atan2~ atanh~ cosh~ cosx~ sinh~ sinx~ tanh~ tanx~}}
            {msp\ math
                {atodb~ average~ avg~ bitand~ bitnot~ bitor~ bitsafe~ bitshift~ bitxor~ cartopol~ dbtoa~ delta~ equals~ greaterthan~ greaterthaneq~ lessthan~ lessthaneq~ modulo~ mstosamps~ notequals~ plusequals~ poltocar~ pong~ rdiv~ rminus~ round~ sampstoms~ scale~trunc~}}
            {msp\ fx
                {degrade~ delay~ downsamp~ overdrive~}}
            {msp\ filters
                {allpass~ buffir~ comb~ cross~ deltaclip~ lores~ onepole~ phaseshift~ rampsmooth~ reson~ slide~ svf~ teeth~}}
            {msp\ synthesis
                {cycle~ curve~ kink~ line~ lookup~ pink~ rand~ train~ trapezoid~ triangle~ wave~}}
            {msp\ play\ buffer
                {buffer~ index~ peek~ play~ poke~ record~}}
            {msp\ analysis
                {capture~ change~ edge~ minmax~ peakamp~ sah~ snapshot~ spike~ thresh~ zerox~}}
            {msp\ others
                {click~ count~ frameaccum~ framedelta~ gate~ matrix~ phasewrap~ scope~ selector~ vectral~}}
            {deprecated
                {append clip~ comment forward maximum~ minimum~ number~ pow~ xbendin xbendin2 xbendout xbendout2}}
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
