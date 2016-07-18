# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "LICENSE.txt," in this distribution.
# Copyright (c) 1997-2009 Miller Puckette.

package provide dialog_scope 0.1

namespace eval ::dialog_scope:: {   
    namespace export pdtk_scope_dialog
}

proc ::dialog_scope::scope_clip_ents {mytoplevel} {
	set vid [string trimleft $mytoplevel .]
	
	#dimensions
	set var_scope_wdt [concat scope_wdt_$vid]
    global $var_scope_wdt
    set var_scope_hgt [concat scope_hgt_$vid]
    global $var_scope_hgt
	set var_scope_wdt_min [concat scope_wdt_min_$vid]
    global $var_scope_wdt_min
    set var_scope_hgt_min [concat scope_hgt_min_$vid]
    global $var_scope_hgt_min
    
    #buffer
    set var_scope_cal [concat scope_cal_$vid]
	global $var_scope_cal
	set var_scope_bfs [concat scope_bfs_$vid]
	global $var_scope_bfs
	set var_scope_cal_min [concat scope_cal_min_$vid]
    global $var_scope_cal_min
    set var_scope_cal_max [concat scope_cal_max_$vid]
    global $var_scope_cal_max
    set var_scope_bfs_min [concat scope_bfs_min_$vid]
    global $var_scope_bfs_min
    set var_scope_bfs_max [concat scope_bfs_max_$vid]
    global $var_scope_bfs_max
    
    #delay
    set var_scope_del [concat scope_del_$vid]
    global $var_scope_del
    set var_scope_del_min [concat scope_del_min_$vid]
    global $var_scope_del_min
    
    
    #dimensions
    if { [eval concat $$var_scope_wdt] < [eval concat $$var_scope_wdt_min] } {
    	set $var_scope_wdt [eval concat $$var_scope_wdt_min] 
    	$mytoplevel.dim.fr.w_ent configure -textvariable $var_scope_wdt
    }
    if { [eval concat $$var_scope_hgt] < [eval concat $$var_scope_hgt_min] } {
    	set $var_scope_hgt [eval concat $$var_scope_hgt_min] 
    	$mytoplevel.dim.fr.h_ent configure -textvariable $var_scope_hgt
    }
    
    #buffer
    if { [eval concat $$var_scope_cal] < [eval concat $$var_scope_cal_min] } {
    	set $var_scope_cal [eval concat $$var_scope_cal_min] 
    	$mytoplevel.buf.fr.cal_ent configure -textvariable $var_scope_cal
    }
    if { [eval concat $$var_scope_cal] > [eval concat $$var_scope_cal_max] } {
    	set $var_scope_cal [eval concat $$var_scope_cal_max] 
    	$mytoplevel.buf.fr.cal_ent configure -textvariable $var_scope_cal
    }
    if { [eval concat $$var_scope_bfs] < [eval concat $$var_scope_bfs_min] } {
    	set $var_scope_bfs [eval concat $$var_scope_bfs_min] 
    	$mytoplevel.buf.fr.bfs_ent configure -textvariable $var_scope_bfs
    }
    if { [eval concat $$var_scope_bfs] > [eval concat $$var_scope_bfs_max] } {
    	set $var_scope_bfs [eval concat $$var_scope_bfs_max] 
    	$mytoplevel.buf.fr.bfs_ent configure -textvariable $var_scope_bfs
    }
    
    #delay
    if { [eval concat $$var_scope_del] < [eval concat $$var_scope_del_min] } {
    	set $var_scope_del [eval concat $$var_scope_del_min] 
    	$mytoplevel.misc.fr.del_ent configure -textvariable $var_scope_del
    }
    
}    

proc ::dialog_scope::scope_check_range {mytoplevel} {
	set vid [string trimleft $mytoplevel .]
	
	set var_scope_min_rng [concat scope_min_rng_$vid]
    global $var_scope_min_rng
    set var_scope_max_rng [concat scope_max_rng_$vid]
    global $var_scope_max_rng
    
    if { [eval concat $$var_scope_min_rng] > [eval concat $$var_scope_max_rng] } {
		set temp [eval concat $$var_scope_min_rng]
		set $var_scope_min_rng [eval concat $$var_scope_max_rng]
		set $var_scope_max_rng [eval concat $temp]
    	$mytoplevel.rng.fr.min_ent configure -textvariable $var_scope_min_rng
    	$mytoplevel.rng.fr.max_ent configure -textvariable $var_scope_max_rng
    }
}

proc ::dialog_scope::scope_empty_ents {mytoplevel} {
	set vid [string trimleft $mytoplevel .]
	set var_scope_wdt [concat scope_wdt_$vid]
    global $var_scope_wdt
    set var_scope_hgt [concat scope_hgt_$vid]
    global $var_scope_hgt
	set var_scope_cal [concat scope_cal_$vid]
	global $var_scope_cal
	set var_scope_bfs [concat scope_bfs_$vid]
	global $var_scope_bfs
    set var_scope_min_rng [concat scope_min_rng_$vid]
    global $var_scope_min_rng
    set var_scope_max_rng [concat scope_max_rng_$vid]
    global $var_scope_max_rng
    set var_scope_del [concat scope_del_$vid]
    global $var_scope_del
    set var_scope_tlv [concat scope_tlv_$vid]
    global $var_scope_tlv
    set var_scope_drs [concat scope_drs_$vid]
    
    set var_scope_wdt_init [concat scope_wdt_init_$vid]
    global $var_scope_wdt_init
    set var_scope_hgt_init [concat scope_hgt_init_$vid]
    global $var_scope_hgt_init
	set var_scope_cal_init [concat scope_cal_init_$vid]
	global $var_scope_cal_init
	set var_scope_bfs_init [concat scope_bfs_init_$vid]
	global $var_scope_bfs_init
    set var_scope_min_rng_init [concat scope_min_rng_init_$vid]
    global $var_scope_min_rng_init
    set var_scope_max_rng_init [concat scope_max_rng_init_$vid]
    global $var_scope_max_rng_init
    set var_scope_del_init [concat scope_del_init_$vid]
    global $var_scope_del_init
    set var_scope_tlv_init [concat scope_tlv_init_$vid]
    global $var_scope_tlv_init
    
    if {[eval concat $$var_scope_wdt] eq ""} {set $var_scope_wdt [eval concat $$var_scope_wdt_init]}
    if {[eval concat $$var_scope_hgt] eq ""} {set $var_scope_hgt [eval concat $$var_scope_hgt_init]}
    if {[eval concat $$var_scope_cal] eq ""} {set $var_scope_cal [eval concat $$var_scope_cal_init]}
    if {[eval concat $$var_scope_bfs] eq ""} {set $var_scope_bfs [eval concat $$var_scope_bfs_init]}
    if {[eval concat $$var_scope_min_rng] eq ""} {set $var_scope_min_rng [eval concat $$var_scope_min_rng_init]}
    if {[eval concat $$var_scope_max_rng] eq ""} {set $var_scope_max_rng [eval concat $$var_scope_max_rng_init]}
    if {[eval concat $$var_scope_del] eq ""} {set $var_scope_del [eval concat $$var_scope_del_init]}
    if {[eval concat $$var_scope_tlv] eq ""} {set $var_scope_tlv [eval concat $$var_scope_tlv_init]}
    
}

proc ::dialog_scope::scope_set_col_example {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_scope_bcol [concat scope_bcol_$vid]
    global $var_scope_bcol
    set var_scope_gcol [concat scope_gcol_$vid]
    global $var_scope_gcol
    set var_scope_fcol [concat scope_fcol_$vid]
    global $var_scope_fcol
    
    $mytoplevel.colors.scopevis.cv itemconfigure "bg" -fill [eval concat $$var_scope_bcol]
    $mytoplevel.colors.scopevis.cv itemconfigure "gr" -fill [eval concat $$var_scope_gcol]
    $mytoplevel.colors.scopevis.cv itemconfigure "fg" -fill [eval concat $$var_scope_fcol]
    
    # for OSX live updates
    if {$::windowingsystem eq "aqua"} {
        ::dialog_scope::apply_and_rebind_return $mytoplevel
    }
}

proc ::dialog_scope::scope_preset_col {mytoplevel presetcol} {
    set vid [string trimleft $mytoplevel .]
    
    set var_scope_f2_g1_b0 [concat scope_f2_g1_b0_$vid]
    global $var_scope_f2_g1_b0
    set var_scope_bcol [concat scope_bcol_$vid]
    global $var_scope_bcol
    set var_scope_gcol [concat scope_gcol_$vid]
    global $var_scope_gcol
    set var_scope_fcol [concat scope_fcol_$vid]
    global $var_scope_fcol
    
    if { [eval concat $$var_scope_f2_g1_b0] == 0 } { set $var_scope_bcol $presetcol }
    if { [eval concat $$var_scope_f2_g1_b0] == 1 } { set $var_scope_gcol $presetcol }
    if { [eval concat $$var_scope_f2_g1_b0] == 2 } { set $var_scope_fcol $presetcol }
    ::dialog_scope::scope_set_col_example $mytoplevel
}

proc ::dialog_scope::scope_choose_col_bkgrfg {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_scope_f2_g1_b0 [concat scope_f2_g1_b0_$vid]
    global $var_scope_f2_g1_b0
    set var_scope_bcol [concat scope_bcol_$vid]
    global $var_scope_bcol
    set var_scope_gcol [concat scope_gcol_$vid]
    global $var_scope_gcol
    set var_scope_fcol [concat scope_fcol_$vid]
    global $var_scope_fcol
    
    if {[eval concat $$var_scope_f2_g1_b0] == 0} {
        set $var_scope_bcol [eval concat $$var_scope_bcol]
        set helpstring [tk_chooseColor -title [_ "Background color"] -initialcolor [eval concat $$var_scope_bcol]]
        if { $helpstring ne "" } {
            set $var_scope_bcol $helpstring }
    }
    if {[eval concat $$var_scope_f2_g1_b0] == 1} {
        set $var_scope_gcol [eval concat $$var_scope_gcol]
        set helpstring [tk_chooseColor -title [_ "Grid color"] -initialcolor [eval concat $$var_scope_gcol]]
        if { $helpstring ne "" } {
            set $var_scope_gcol $helpstring }
    }
    if {[eval concat $$var_scope_f2_g1_b0] == 2} {
    	set $var_scope_fcol [eval concat $$var_scope_fcol]
        set helpstring [tk_chooseColor -title [_ "Phosphor color"] -initialcolor [eval concat $$var_scope_fcol]]
        if { $helpstring ne "" } {
            set $var_scope_fcol $helpstring }
    }
    ::dialog_scope::scope_set_col_example $mytoplevel
}

proc ::dialog_scope::scope_trigger_mode {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_scope_tmd [concat scope_tmd_$vid]
    global $var_scope_tmd
    set var_scope_tmd0 [concat scope_tmd0_$vid]
    global $var_scope_tmd0
    set var_scope_tmd1 [concat scope_tmd1_$vid]
    global $var_scope_tmd1
    set var_scope_tmd2 [concat scope_tmd2_$vid]
    global $var_scope_tmd2

    
    if {[eval concat $$var_scope_tmd] == 0} {
        set $var_scope_tmd 1
        $mytoplevel.trg.tmd.trb configure -text [eval concat $$var_scope_tmd1]
    } elseif {[eval concat $$var_scope_tmd] == 1} {
    	set $var_scope_tmd 2
    	$mytoplevel.trg.tmd.trb configure -text [eval concat $$var_scope_tmd2]
    } else {
        set $var_scope_tmd 0
        $mytoplevel.trg.tmd.trb configure -text [eval concat $$var_scope_tmd0]
    }
}


proc ::dialog_scope::apply {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_scope_wdt [concat scope_wdt_$vid]
    global $var_scope_wdt
    set var_scope_hgt [concat scope_hgt_$vid]
    global $var_scope_hgt
	set var_scope_cal [concat scope_cal_$vid]
	global $var_scope_cal
	set var_scope_bfs [concat scope_bfs_$vid]
	global $var_scope_bfs
    set var_scope_min_rng [concat scope_min_rng_$vid]
    global $var_scope_min_rng
    set var_scope_max_rng [concat scope_max_rng_$vid]
    global $var_scope_max_rng
    set var_scope_del [concat scope_del_$vid]
    global $var_scope_del
    set var_scope_tmd [concat scope_tmd_$vid]
    global $var_scope_tmd
    set var_scope_tlv [concat scope_tlv_$vid]
    global $var_scope_tlv
    set var_scope_drs [concat scope_drs_$vid]
    global $var_scope_drs
    set var_scope_bcol [concat scope_bcol_$vid]
    global $var_scope_bcol
    set var_scope_gcol [concat scope_gcol_$vid]
    global $var_scope_gcol
    set var_scope_fcol [concat scope_fcol_$vid]
    global $var_scope_fcol

	::dialog_scope::scope_empty_ents $mytoplevel
	::dialog_scope::scope_clip_ents $mytoplevel
	::dialog_scope::scope_check_range $mytoplevel
	
	pdsend [concat $mytoplevel dialog \
			[eval concat $$var_scope_wdt] \
			[eval concat $$var_scope_hgt] \
			[eval concat $$var_scope_cal] \
			[eval concat $$var_scope_bfs] \
			[eval concat $$var_scope_min_rng] \
			[eval concat $$var_scope_max_rng] \
			[eval concat $$var_scope_del] \
			[eval concat $$var_scope_drs] \
			[eval concat $$var_scope_tmd] \
			[eval concat $$var_scope_tlv] \
			[eval concat $$var_scope_bcol] \
			[eval concat $$var_scope_gcol] \
			[eval concat $$var_scope_fcol]]

}	

# 	foreach ent {$var_scope_wdt $var_scope_hgt $var_scope_cal $var_scope_bfs
    # make sure the offset boxes have a value
#     if {[eval concat $$var_iemgui_gn_dx] eq ""} {set $var_iemgui_gn_dx 0}
#     if {[eval concat $$var_iemgui_gn_dy] eq ""} {set $var_iemgui_gn_dy 0}

#     pdsend [concat $mytoplevel dialog \
#             [eval concat $$var_iemgui_wdt] \
#             [eval concat $$var_iemgui_hgt] \
#             [eval concat $$var_iemgui_min_rng] \
#             [eval concat $$var_iemgui_max_rng] \
#             [eval concat $$var_iemgui_bcol] \
#             [eval concat $$var_iemgui_fcol] \
#             [eval concat $$var_iemgui_lcol] \
#             [eval concat $$var_iemgui_steady]]





proc ::dialog_scope::cancel {mytoplevel} {
    pdsend "$mytoplevel cancel"
}

proc ::dialog_scope::ok {mytoplevel} {
    ::dialog_scope::apply $mytoplevel
    ::dialog_scope::cancel $mytoplevel
}

proc ::dialog_scope::pdtk_scope_dialog {mytoplevel \
						dim_header wdt wdt_label hgt hgt_label \
                        buf_header cal cal_label bfs bfs_label \
                        rng_header min_rng min_rng_label max_rng max_rng_label \
                        del_header del del_label drs_header drs drs_label\
                        trg_header tmd tmd_label tlv tlv_label \
                        dim_mins wdt_min hgt_min \
                        cal_min_max cal_min cal_max bfs_min_max bfs_min bfs_max \
                        del_mins del_min \
                        bcol gcol fcol} {
    
    set vid [string trimleft $mytoplevel .]

    set var_scope_wdt [concat scope_wdt_$vid]
    global $var_scope_wdt
    set var_scope_hgt [concat scope_hgt_$vid]
    global $var_scope_hgt
	set var_scope_cal [concat scope_cal_$vid]
	global $var_scope_cal
	set var_scope_bfs [concat scope_bfs_$vid]
	global $var_scope_bfs
    set var_scope_min_rng [concat scope_min_rng_$vid]
    global $var_scope_min_rng
    set var_scope_max_rng [concat scope_max_rng_$vid]
    global $var_scope_max_rng
    set var_scope_del [concat scope_del_$vid]
    global $var_scope_del
    set var_scope_tmd [concat scope_tmd_$vid]
    global $var_scope_tmd
    set var_scope_tmd0 [concat scope_tmd0_$vid]
    global $var_scope_tmd0
    set var_scope_tmd1 [concat scope_tmd1_$vid]
    global $var_scope_tmd1
    set var_scope_tmd2 [concat scope_tmd2_$vid]
    global $var_scope_tmd2
    set var_scope_tlv [concat scope_tlv_$vid]
    global $var_scope_tlv
    set var_scope_drs [concat scope_drs_$vid]
    global $var_scope_drs
    set var_scope_f2_g1_b0 [concat scope_f2_g1_b0_$vid]
    global $var_scope_f2_g1_b0
    set var_scope_bcol [concat scope_bcol_$vid]
    global $var_scope_bcol
    set var_scope_gcol [concat scope_gcol_$vid]
    global $var_scope_gcol
    set var_scope_fcol [concat scope_fcol_$vid]
    global $var_scope_fcol
    
    set var_scope_wdt_min [concat scope_wdt_min_$vid]
    global $var_scope_wdt_min
    set var_scope_hgt_min [concat scope_hgt_min_$vid]
    global $var_scope_hgt_min
    set var_scope_cal_min [concat scope_cal_min_$vid]
    global $var_scope_cal_min
    set var_scope_cal_max [concat scope_cal_max_$vid]
    global $var_scope_cal_max
    set var_scope_bfs_min [concat scope_bfs_min_$vid]
    global $var_scope_bfs_min
    set var_scope_bfs_max [concat scope_bfs_max_$vid]
    global $var_scope_bfs_max
    set var_scope_del_min [concat scope_del_min_$vid]
    global $var_scope_del_min
    
    set var_scope_wdt_init [concat scope_wdt_init_$vid]
    global $var_scope_wdt_init
    set var_scope_hgt_init [concat scope_hgt_init_$vid]
    global $var_scope_hgt_init
	set var_scope_cal_init [concat scope_cal_init_$vid]
	global $var_scope_cal_init
	set var_scope_bfs_init [concat scope_bfs_init_$vid]
	global $var_scope_bfs_init
    set var_scope_min_rng_init [concat scope_min_rng_init_$vid]
    global $var_scope_min_rng_init
    set var_scope_max_rng_init [concat scope_max_rng_init_$vid]
    global $var_scope_max_rng_init
    set var_scope_del_init [concat scope_del_init_$vid]
    global $var_scope_del_init
    set var_scope_tlv_init [concat scope_tlv_init_$vid]
    global $var_scope_tlv_init
    
    set $var_scope_wdt $wdt
    set $var_scope_hgt $hgt
	set $var_scope_cal $cal
	set $var_scope_bfs $bfs
    set $var_scope_min_rng $min_rng
    set $var_scope_max_rng $max_rng
	set $var_scope_del $del
	set $var_scope_drs $drs
	set $var_scope_tmd $tmd
	set $var_scope_tlv $tlv  
    set $var_scope_bcol $bcol
    set $var_scope_gcol $gcol
    set $var_scope_fcol $fcol
    set $var_scope_f2_g1_b0 0
    
    set $var_scope_wdt_init $wdt
    set $var_scope_hgt_init $hgt
	set $var_scope_cal_init $cal
	set $var_scope_bfs_init $bfs
    set $var_scope_min_rng_init $min_rng
    set $var_scope_max_rng_init $max_rng
	set $var_scope_del_init $del
	set $var_scope_tlv_init $tlv 
    
    set $var_scope_wdt_min $wdt_min
    set $var_scope_hgt_min $hgt_min
    set $var_scope_cal_min $cal_min
    set $var_scope_cal_max $cal_max
    set $var_scope_bfs_min $bfs_min
    set $var_scope_bfs_max $bfs_max
    set $var_scope_del_min $del_min
    
    
    set wdt_label [_ "Width:"]
    set hgt_label [_ "Height:"]
    set buf_header [_ "Buffer Settings:"]
    set cal_label [_ "Samples Per Point:"]
    set bfs_label [_ "Buffer Size:"]
    set rng_header [_ "Signal Range:"]
    set min_rng_label [_ "Minval:"]
    set max_rng_label [_ "Maxval:"]
    set del_header [_ "Delay:"]
    set del_label [_ "Delay:"]
    set drs_label [_ "Alternate Drawstyle:"]
    set tmd_label [_ "Trigger Mode:"]
    set tmd0_label [_ "None"]
    set tmd1_label [_ "Up"]
    set tmd2_label [_ "Down"]
    set tlv_label [_ "Trigger Level:"]
    
    set $var_scope_tmd0 $tmd0_label
    set $var_scope_tmd1 $tmd1_label
    set $var_scope_tmd2 $tmd2_label
    
    toplevel $mytoplevel -class DialogWindow
    wm title $mytoplevel [format [_ "scope~ Properties"] ]
    wm group $mytoplevel .
    wm resizable $mytoplevel 0 0
    wm transient $mytoplevel $::focused_window
    $mytoplevel configure -menu $::dialog_menubar
    $mytoplevel configure -padx 0 -pady 0
    ::pd_bindings::dialog_bindings $mytoplevel "scope"

    # dimensions
    labelframe $mytoplevel.dim -borderwidth 1 -pady 8 -text [_ "Dimensions:"]
    pack $mytoplevel.dim -side top -fill x -pady 5
    frame $mytoplevel.dim.fr
    label $mytoplevel.dim.fr.w_lab -text [_ $wdt_label]
    entry $mytoplevel.dim.fr.w_ent -textvariable $var_scope_wdt -width 4
    label $mytoplevel.dim.fr.dummy1 -text "" -width 1
    label $mytoplevel.dim.fr.h_lab -text [_ $hgt_label]
    entry $mytoplevel.dim.fr.h_ent -textvariable $var_scope_hgt -width 4
    pack $mytoplevel.dim.fr -side left -expand 1
    pack $mytoplevel.dim.fr.w_lab $mytoplevel.dim.fr.w_ent \
    	$mytoplevel.dim.fr.dummy1 $mytoplevel.dim.fr.h_lab $mytoplevel.dim.fr.h_ent -side left
    
    # calccount & bufsize
    labelframe $mytoplevel.buf -borderwidth 1 -pady 8 -text [_ $buf_header]
    pack $mytoplevel.buf -side top -fill x -pady 5
    frame $mytoplevel.buf.fr
    label $mytoplevel.buf.fr.cal_lab -text [_ $cal_label]
    entry $mytoplevel.buf.fr.cal_ent -textvariable $var_scope_cal -width 4
    label $mytoplevel.buf.fr.dummy1 -text "" -width 1
    label $mytoplevel.buf.fr.bfs_lab -text [_ $bfs_label]
    entry $mytoplevel.buf.fr.bfs_ent -textvariable $var_scope_bfs -width 4
    pack $mytoplevel.buf.fr -side left -expand 1
    pack $mytoplevel.buf.fr.cal_lab $mytoplevel.buf.fr.cal_ent -side left
    pack $mytoplevel.buf.fr.dummy1 $mytoplevel.buf.fr.bfs_lab $mytoplevel.buf.fr.bfs_ent -side left
    
    # range
    labelframe $mytoplevel.rng -borderwidth 1 -pady 8 -text [_ $rng_header]
    pack $mytoplevel.rng -side top -fill x -pady 5
    frame $mytoplevel.rng.fr
    label $mytoplevel.rng.fr.min_lab -text [_ $min_rng_label]
    entry $mytoplevel.rng.fr.min_ent -textvariable $var_scope_min_rng -width 8
    label $mytoplevel.rng.fr.dummy1 -text "" -width 1
    label $mytoplevel.rng.fr.max_lab -text [_ $max_rng_label]
    entry $mytoplevel.rng.fr.max_ent -textvariable $var_scope_max_rng -width 8
    pack $mytoplevel.rng.fr -side left -expand 1
	pack $mytoplevel.rng.fr.min_lab $mytoplevel.rng.fr.min_ent -side left
	pack $mytoplevel.rng.fr.dummy1 $mytoplevel.rng.fr.max_lab $mytoplevel.rng.fr.max_ent -side left
	
	#trigger
	labelframe $mytoplevel.trg 
	pack $mytoplevel.trg -side top -fill x -pady 5
	frame $mytoplevel.trg.tmd
	label $mytoplevel.trg.tmd.lab -text [_ $tmd_label]
	if {[eval concat $$var_scope_tmd] == 0} {
		button $mytoplevel.trg.tmd.trb -text [_ [eval concat $$var_scope_tmd0]]	-width 4 \
			-command "::dialog_scope::scope_trigger_mode $mytoplevel" }
	if {[eval concat $$var_scope_tmd] == 1} {
		button $mytoplevel.trg.tmd.trb -text [_ [eval concat $$var_scope_tmd1]]	-width 4 \
			-command "::dialog_scope::scope_trigger_mode $mytoplevel" }
	if {[eval concat $$var_scope_tmd] == 2} {
		button $mytoplevel.trg.tmd.trb -text [_ [eval concat $$var_scope_tmd2]]	-width 4 \
			-command "::dialog_scope::scope_trigger_mode $mytoplevel" }
	label $mytoplevel.trg.dummy1 -text "" -width 1
	label $mytoplevel.trg.tlv_lab -text [_ $tlv_label]
	entry $mytoplevel.trg.tlv_ent -textvariable $var_scope_tlv -width 8
	$mytoplevel.trg config -borderwidth 1 -padx 5 -pady 5 -text [_ "Trigger Settings:"]
	pack $mytoplevel.trg.tmd
	pack $mytoplevel.trg.tmd.lab -side left
	pack $mytoplevel.trg.tmd.trb -side left
	$mytoplevel.trg config -padx 10
	pack configure $mytoplevel.trg.tmd -side left
	pack $mytoplevel.trg.dummy1 $mytoplevel.trg.tlv_lab $mytoplevel.trg.tlv_ent -side left
	
	#delay
	labelframe $mytoplevel.misc -borderwidth 1 -pady 8
    pack $mytoplevel.misc -side top -pady 5 -fill x
    frame $mytoplevel.misc.fr
    label $mytoplevel.misc.fr.del_lab -text [_ $del_label]
    entry $mytoplevel.misc.fr.del_ent -textvariable $var_scope_del -width 7
    label $mytoplevel.misc.fr.dummy1 -text "" -width 4
    label $mytoplevel.misc.fr.drs_lab -text [_ $drs_label]
    checkbutton $mytoplevel.misc.fr.drs_chk -variable $var_scope_drs
    pack $mytoplevel.misc.fr -side left -expand 1
	pack $mytoplevel.misc.fr.del_lab $mytoplevel.misc.fr.del_ent \
		$mytoplevel.misc.fr.dummy1 $mytoplevel.misc.fr.drs_lab $mytoplevel.misc.fr.drs_chk -side left
	
    
    # colors
    labelframe $mytoplevel.colors -borderwidth 1 -text [_ "Colors:"] -padx 5 -pady 5
    pack $mytoplevel.colors -fill x
    
    frame $mytoplevel.colors.scopevis 
    pack $mytoplevel.colors.scopevis -side top -pady 10
    canvas $mytoplevel.colors.scopevis.cv -width 81 -height 65 \
    	-relief flat -highlightthickness 0
    pack $mytoplevel.colors.scopevis.cv -in $mytoplevel.colors.scopevis
    $mytoplevel.colors.scopevis.cv create rectangle 0 0 80 64 \
    	-fill [eval concat $$var_scope_bcol] -tags {"bg"}
    foreach i {0 10 20 30 40 50 60 70 80} {
    	$mytoplevel.colors.scopevis.cv create line $i 0 $i 64 -fill [eval concat $$var_scope_gcol] \
    	-width 0.5 -tags {"gr"}
    }
    foreach i {0 16 32 48 64} {
    	$mytoplevel.colors.scopevis.cv create line 0 $i 80 $i -fill [eval concat $$var_scope_gcol] \
    	-width 0.5 -tags {"gr"}
    }
    $mytoplevel.colors.scopevis.cv create line \
    	0 32 5 19.754 10 9.367 15 2.436 \
    	20 0 25 2.436 30 9.367 35 19.754 \
    	40 32 45 44.246 50 54.624 55 61.564 \
    	60 64 65 61.564 70 54.624 75 44.246 \
    	80 32 \
    	-smooth bezier -fill [eval concat $$var_scope_fcol] \
    	-width 0.9 -tags {"fg"}
    	
    frame $mytoplevel.colors.dummy
    pack $mytoplevel.colors.dummy -side top -pady 10
    frame $mytoplevel.colors.select
    pack $mytoplevel.colors.select -side top
    radiobutton $mytoplevel.colors.select.radio0 -value 0 -variable \
        $var_scope_f2_g1_b0 -text [_ "Background"] -justify left
    radiobutton $mytoplevel.colors.select.radio1 -value 1 -variable \
        $var_scope_f2_g1_b0 -text [_ "Grid"] -justify left
    radiobutton $mytoplevel.colors.select.radio2 -value 2 -variable \
        $var_scope_f2_g1_b0 -text [_ "Phosphor"] -justify left
    pack $mytoplevel.colors.select.radio0 $mytoplevel.colors.select.radio1 \
        $mytoplevel.colors.select.radio2 -side left
    
    frame $mytoplevel.colors.sections 
    pack $mytoplevel.colors.sections -side top -pady 5
    button $mytoplevel.colors.sections.but -text [_ "Compose color"] \
        -command "::dialog_scope::scope_choose_col_bkgrfg $mytoplevel"
    pack $mytoplevel.colors.sections.but -side left -anchor w -pady 5 \
        -expand yes -fill x

    
    # color scheme by Mary Ann Benedetto http://piR2.org
    foreach r {r1 r2 r3} hexcols {
       { "#FFFFFF" "#DFDFDF" "#BBBBBB" "#FFC7C6" "#FFE3C6" "#FEFFC6" "#C6FFC7" "#C6FEFF" "#C7C6FF" "#E3C6FF" }
       { "#9F9F9F" "#7C7C7C" "#606060" "#FF0400" "#FF8300" "#FAFF00" "#00FF04" "#00FAFF" "#0400FF" "#9C00FF" }
       { "#404040" "#202020" "#000000" "#551312" "#553512" "#535512" "#0F4710" "#0E4345" "#131255" "#2F004D" } } \
    {
       frame $mytoplevel.colors.$r
       pack $mytoplevel.colors.$r -side top
       foreach i { 0 1 2 3 4 5 6 7 8 9} hexcol $hexcols \
           {
               label $mytoplevel.colors.$r.c$i -background $hexcol -activebackground $hexcol -relief ridge -padx 7 -pady 0
               bind $mytoplevel.colors.$r.c$i <Button> "::dialog_scope::scope_preset_col $mytoplevel $hexcol"
           }
       pack $mytoplevel.colors.$r.c0 $mytoplevel.colors.$r.c1 $mytoplevel.colors.$r.c2 $mytoplevel.colors.$r.c3 \
           $mytoplevel.colors.$r.c4 $mytoplevel.colors.$r.c5 $mytoplevel.colors.$r.c6 $mytoplevel.colors.$r.c7 \
           $mytoplevel.colors.$r.c8 $mytoplevel.colors.$r.c9 -side left
    }
    
    #buttons
    frame $mytoplevel.cao -pady 10
    pack $mytoplevel.cao -side top -expand 1 -fill x
    button $mytoplevel.cao.cancel -text [_ "Cancel"] \
        -command "::dialog_scope::cancel $mytoplevel"
    pack $mytoplevel.cao.cancel -side left -padx 10 -expand 1 -fill x
    if {$::windowingsystem ne "aqua"} {
        button $mytoplevel.cao.apply -text [_ "Apply"] \
            -command "::dialog_scope::apply $mytoplevel"
        pack $mytoplevel.cao.apply -side left -padx 10 -expand 1 -fill x
    }
    button $mytoplevel.cao.ok -text [_ "OK"] \
        -command "::dialog_scope::ok $mytoplevel" -default active
    pack $mytoplevel.cao.ok -side left -padx 10 -expand 1 -fill x
    
    $mytoplevel.dim.fr.w_ent select from 0
    $mytoplevel.dim.fr.w_ent select adjust end
    focus $mytoplevel.dim.fr.w_ent

    # live widget updates on OSX in lieu of Apply button
    if {$::windowingsystem eq "aqua"} {

        # call apply on Return in entry boxes that are in focus & rebind Return to ok button
        bind $mytoplevel.dim.fr.w_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"
        bind $mytoplevel.dim.fr.h_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"
        bind $mytoplevel.buf.fr.cal_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"
        bind $mytoplevel.buf.fr.bfs_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"
		bind $mytoplevel.rng.fr.min_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"
		bind $mytoplevel.rng.fr.max_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"
		bind $mytoplevel.trg.tlv_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"
		bind $mytoplevel.misc.fr.del_ent <KeyPress-Return> "::dialog_scope::apply_and_rebind_return $mytoplevel"

        # unbind Return from ok button when an entry takes focus
        $mytoplevel.dim.fr.w_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"
        $mytoplevel.dim.fr.h_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"
        $mytoplevel.buf.fr.cal_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"
        $mytoplevel.buf.fr.bfs_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"
		$mytoplevel.rng.fr.min_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"
		$mytoplevel.rng.fr.max_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"
		$mytoplevel.trg.tlv_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"
		$mytoplevel.misc.fr.del_ent config -validate focusin -vcmd "::dialog_scope::unbind_return $mytoplevel"


        # remove cancel button from focus list since it's not activated on Return
        $mytoplevel.cao.cancel config -takefocus 0

        # can't see focus for buttons, so disable it
        $mytoplevel.trg.tmd.trb config -takefocus 0

        $mytoplevel.colors.select.radio0 config -takefocus 0
        $mytoplevel.colors.select.radio1 config -takefocus 0
        $mytoplevel.colors.select.radio2 config -takefocus 0
        $mytoplevel.colors.sections.but config -takefocus 0

        # show active focus on the ok button as it *is* activated on Return
        $mytoplevel.cao.ok config -default normal
        bind $mytoplevel.cao.ok <FocusIn> "$mytoplevel.cao.ok config -default active"
        bind $mytoplevel.cao.ok <FocusOut> "$mytoplevel.cao.ok config -default normal"
    }
}

# for live widget updates on OSX
proc ::dialog_scope::apply_and_rebind_return {mytoplevel} {
    ::dialog_scope::apply $mytoplevel
    bind $mytoplevel <KeyPress-Return> "::dialog_scope::ok $mytoplevel"
    focus $mytoplevel.cao.ok
    return 0
}

# for live widget updates on OSX
proc ::dialog_scope::unbind_return {mytoplevel} {
    bind $mytoplevel <KeyPress-Return> break
    return 1
}
