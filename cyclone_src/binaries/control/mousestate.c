/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "hammer/gui.h"
#include "g_canvas.h"

/* DK
- adding mousestate_proxy and related methods
*/



typedef struct _mousestate
{
    t_object   x_ob;
    int        x_ispolling; 
    int        x_wasbanged;
    int        x_waszeroed;
    int        x_hlast;
    int        x_vlast;
    int        x_hzero;
    int        x_vzero;
    int        x_mode; //0-screen, 1-object window, 2-active window
    
    int         x_wx; 
    int         x_wy;
    int         x_ww;
    int         x_wh;
    t_outlet  *x_hposout;
    t_outlet  *x_vposout;
    t_outlet  *x_hdiffout;
    t_outlet  *x_vdiffout;

} t_mousestate;

static t_class *mousestate_class;





static void mousestate_anything(t_mousestate *x,
				t_symbol *s, int ac, t_atom *av)
{
    /* dummy method, filtering out those messages from gui,
       which are not handled explicitly */
}

static void mousestate_doup(t_mousestate *x, t_floatarg f)
{
    outlet_float(((t_object *)x)->ob_outlet, ((int)f ? 0 : 1));
}


static void mousestate_dobang(t_mousestate *x, t_floatarg f1, t_floatarg f2)
{
    if (x->x_wasbanged || x->x_ispolling)
    {
	int h = (int)f1, v = (int)f2;
	outlet_float(x->x_vdiffout, v - x->x_vlast);
	outlet_float(x->x_hdiffout, h - x->x_hlast);
	outlet_float(x->x_vposout, v - x->x_vzero);
	outlet_float(x->x_hposout, h - x->x_hzero);
	x->x_hlast = h;
	x->x_vlast = v;
	x->x_wasbanged = 0;
    }
}

static void mousestate_objwin(t_mousestate *x, int argc, t_atom * argv){
    t_float objx, objy;
    if(argc >=2 && x->x_mode == 1){
        objx = atom_getfloatarg(0, argc, argv);
        objy = atom_getfloatarg(1, argc, argv);
        mousestate_dobang(x, objx, objy); 
    };
}



static void mousestate_dozero(t_mousestate *x, t_floatarg f1, t_floatarg f2)
{
    if (x->x_waszeroed)
    {
	int h = (int)f1, v = (int)f2;
	x->x_hzero = h;
	x->x_vzero = v;
	x->x_waszeroed = 0;
    }
}

static void mousestate_dopoll(t_mousestate *x, t_floatarg f1, t_floatarg f2)
{
	x->x_wasbanged = 1;
	mousestate_dobang(x, f1, f2);
}

static void mousestate_bang(t_mousestate *x)
{
    switch(x->x_mode){
        case 0:
            hammergui_screenmousexy(gensym("_bang"));
            break;
        case 1:
            hammergui_localmousexy(gensym("_bang"), x->x_wx, x->x_wy, x->x_ww, x->x_wh);
            break;
        case 2:
            hammergui_focusmousexy(gensym("_bang"));
            break;
        default:
            break;
    };
    x->x_wasbanged = 1;
}

static void mousestate_poll(t_mousestate *x)
{
    int mode = x->x_mode;
    //pollmode: mode + 1 : mode0 -> 1, mode1 -> 2, mode2-> 3
    int pollmode = mode + 1;
    hammergui_startpolling((t_pd *)x, pollmode, x->x_wx, x->x_wy, x->x_ww, x->x_wh);
    x->x_ispolling = 1;
}

static void mousestate_nopoll(t_mousestate *x)
{
    hammergui_stoppolling((t_pd *)x, x->x_wx, x->x_wy, x->x_ww, x->x_wh);
    x->x_ispolling = 0;
}

static void mousestate_zero(t_mousestate *x)
{
    switch(x->x_mode){
        case 0:
            hammergui_screenmousexy(gensym("_zero"));
            break;
        case 1:
            hammergui_localmousexy( gensym("_zero"), x->x_wx, x->x_wy, x->x_ww, x->x_wh);
            break;
        case 2:
            hammergui_focusmousexy(gensym("_zero"));
            break;
        default:
            break;
    };
    x->x_waszeroed = 1;
}

static void mousestate_reset(t_mousestate *x)
{
    x->x_hzero = x->x_vzero = 0;
}

static void mousestate_free(t_mousestate *x)
{
    mousestate_nopoll(x);
    hammergui_unbindmouse((t_pd *)x);
}

static void mousestate_mode(t_mousestate *x, t_floatarg f){
    int mode = (int) f;
    int polling = x->x_ispolling;
    if(mode < 0){
        mode = 0;
    }
    else if(mode > 2){
        mode = 2;
    };
    if(polling)
    {
        mousestate_nopoll(x);
        x->x_mode = mode;
        mousestate_poll(x);
    }
    else x->x_mode = mode;
}

static void *mousestate_new(void)
{
    int x1, x2, y1, y2;
    t_mousestate *x = (t_mousestate *)pd_new(mousestate_class);
    x->x_ispolling = x->x_wasbanged = x->x_waszeroed = 0;
    outlet_new((t_object *)x, &s_float);
    x->x_hposout = outlet_new((t_object *)x, &s_float);
    x->x_vposout = outlet_new((t_object *)x, &s_float);
    x->x_hdiffout = outlet_new((t_object *)x, &s_float);
    x->x_vdiffout = outlet_new((t_object *)x, &s_float);
    x->x_mode = 0;
    hammergui_bindmouse((t_pd *)x);
    hammergui_willpoll();
    mousestate_reset(x);
    t_glist *g_list = (t_glist *)canvas_getcurrent();

    x1 =  g_list->gl_screenx1;
    y1 = g_list->gl_screeny1;
    x2 = g_list->gl_screenx2;
    y2 = g_list->gl_screeny2;

    x->x_wx = x1;
    x->x_wy = y1;
    x->x_ww = x2 - x1;
    x->x_wh = y2 - y1;
//     post("%d %d %d %d", x->x_x1, x->x_y1, x->x_x2, x->x_y2);

    return (x);
}

void mousestate_setup(void)
{
    mousestate_class = class_new(gensym("mousestate"),
				 (t_newmethod)mousestate_new,
				 (t_method)mousestate_free,
				 sizeof(t_mousestate), 0, 0);

    class_addcreator((t_newmethod)mousestate_new, gensym("MouseState"), 0, 0);
    class_addcreator((t_newmethod)mousestate_new, gensym("cyclone/MouseState"), 0, 0);
    class_addanything(mousestate_class, mousestate_anything);
    class_addmethod(mousestate_class, (t_method)mousestate_doup,
		    gensym("_up"), A_FLOAT, 0);
    class_addmethod(mousestate_class, (t_method)mousestate_dobang,
		    gensym("_bang"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(mousestate_class, (t_method)mousestate_dozero,
		    gensym("_zero"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(mousestate_class, (t_method)mousestate_dopoll,
		    gensym("_poll"), A_FLOAT, A_FLOAT, 0);
    class_addbang(mousestate_class, mousestate_bang);
    class_addmethod(mousestate_class, (t_method)mousestate_poll,
		    gensym("poll"), 0);
    class_addmethod(mousestate_class, (t_method)mousestate_nopoll,
		    gensym("nopoll"), 0);
    class_addmethod(mousestate_class, (t_method)mousestate_zero,
		    gensym("zero"), 0);
    class_addmethod(mousestate_class, (t_method)mousestate_reset,
		    gensym("reset"), 0);
    class_addmethod(mousestate_class, (t_method)mousestate_mode,
		    gensym("mode"), A_FLOAT, 0);

}

void MouseState_setup(void)
{
    mousestate_setup();
}
