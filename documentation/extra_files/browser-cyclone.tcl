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
            {max\ math
                {accum acos acosh asin asinh atanh atodb cartopol clip cosh dbtoa maximum mean minimum poltocar pong rdiv rminus round scale sinh tanh}}
            {max\ midi
                {borax flush midiflush midiformat midiparse seq sustain xbendin xbendin2 xbendout xbendout2 xnotein xnoteout}}
            {max\ others
                {active anal append bangbang bondo bucket buddy capture coll comment counter cycle decide decode drunk forward fromsymbol funbuff funnel gate grab histo iter join linedrive listfunnel loadmess match mousefilter mousestate mtr next offer onebang pak past peak prepend prob pv speedlim spell split spray sprintf substitute switch table thresh togedge tosymbol trough universal unjoin urn uzi zl}}
            {msp\ math
                {acos~ acosh~ asin~ asinh~ atan~ atan2~ atanh~ atodb~ average~ avg~ bitand~ bitnot~ bitor~ bitsafe~ bitshift~ bitxor~ cartopol~ clip~ cosh~ cosx~ dbtoa~ delta~ equals~ greaterthan~ greaterthaneq~ lessthan~ lessthaneq~ maximum~ minimum~ modulo~ mstosamps~ notequals~ plusequals~ poltocar~ pong~ pow~ rdiv~ rminus~ round~ sampstoms~ scale~ sinh~ sinx~ tanh~ tanx~ trunc~}}
            {msp\ filters
                {allpass~ buffir~ comb~ cross~ deltaclip~ lores~ onepole~ phaseshift~ rampsmooth~ reson~ slide~ svf~ teeth~}}
            {msp\ others
                {capture~ change~ click~ count~ curve~ cycle~ degrade~ delay~ downsamp~ edge~ frameaccum~ framedelta~ gate~ index~ kink~ line~ lookup~ matrix~ minmax~ number~ overdrive~ peakamp~ peek~ phasewrap~ pink~ play~ poke~ rand~ record~ sah~ scope~ selector~ snapshot~ spike~ thresh~ train~ trapezoid~ triangle~ vectral~ wave~ zerox~}}
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
