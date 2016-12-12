/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "hammer/gui.h"
#include "g_canvas.h"

//adding mousestate_proxy and related methods

//borrowing IOhannes zmoelnig's receivecanvas_proxy  from iemguts' receivecanvas
//for gui message interception - DK

//this is the receivecanvas_proxy


typedef struct _mousestate_proxy
{
    //was going to be in hammer/gui but found it difficult, which is why i chose the g prefix
    //instead of p as in receivecanvas
    t_object    g_obj;
    t_symbol    *g_sym;
    struct _mousestate      *g_owner; 
    //copied from the receivecanvas_proxy code
    //guessing this assures clock is freed indp of parent obj?
    t_clock     *g_clock;

} t_mousestate_proxy;

static t_class *mousestate_proxy_class;


typedef struct _mousestate
{
    t_object   x_ob;
    int        x_ispolling02; //polling in mode 0 or mode 2
    int        x_ispolling1; //polling in mode 1, since it's handled differently for now at least
    int        x_wasbanged;
    int        x_waszeroed;
    int        x_hlast;
    int        x_vlast;
    int        x_hzero;
    int        x_vzero;
    int        x_mode; //0-screen, 1-object window, 2-active window
    t_outlet  *x_hposout;
    t_outlet  *x_vposout;
    t_outlet  *x_hdiffout;
    t_outlet  *x_vdiffout;

    t_mousestate_proxy *x_icpt;
} t_mousestate;

static t_class *mousestate_class;



//mousestate_proxy methods, again lifted from IOhannes Zmoelnig's receivecanvas from iemguts


static void mousestate_proxy_actualfree(t_mousestate_proxy *g){

    //in receivecanvas_proxy, the actual free method, fitured this makes it a little easier
    if(g->g_sym){
        pd_unbind(&g->g_obj.ob_pd, g->g_sym);
    };
    g->g_sym = NULL;

    clock_free(g->g_clock);
    g->g_clock = NULL;

    g->g_owner = NULL;
    pd_free(&g->g_obj.ob_pd);
    g = NULL;
}



static t_mousestate_proxy * mousestate_proxy_new(t_mousestate * owner, t_symbol *s){
    t_mousestate_proxy *g = NULL;
    
    if(!owner){
        return g;
    };
    g = (t_mousestate_proxy *)pd_new(mousestate_proxy_class);

    g->g_owner = owner;
    g->g_sym = s;

    if(g->g_sym){
        pd_bind(&g->g_obj.ob_pd, g->g_sym);
    };
   
    g->g_clock = clock_new(g, (t_method)mousestate_proxy_actualfree);

    return g;
}

static void mousestate_proxy_free(t_mousestate_proxy *g){
    //clock_delay called directly in receivecanvas, figured this makes it a little more transparent
    clock_delay(g->g_clock, 0);
}

//========MOUSESTATE_METHODS

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
    if (x->x_wasbanged || x->x_ispolling1)
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

//================= MOUSESTATE_PROXY_METHOD
void mousestate_proxy_anything(t_mousestate_proxy *g, t_symbol *s, int argc, t_atom *argv){
    if(g->g_owner){
        //unlike receivecanvas, filter out only motion messages
        if(strcmp(s->s_name, "motion") == 0){
            mousestate_objwin(g->g_owner, argc, argv);
            };
    };
    
}

//================= BACK TO MOUSESTATE

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
    if (x->x_ispolling02)
    {
	x->x_wasbanged = 1;
	mousestate_dobang(x, f1, f2);
    }
}

static void mousestate_bang(t_mousestate *x)
{
    switch(x->x_mode){
        case 0:
            hammergui_screenmousexy(gensym("_bang"));
            break;
        case 1:
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
    if(mode != 1){
        if (!x->x_ispolling02)
        {
            x->x_ispolling02 = 1;
                hammergui_startpolling((t_pd *)x);

        };
    }
    else{
        x->x_ispolling1 = 1;
    };
}

static void mousestate_nopoll(t_mousestate *x)
{
    int mode = x->x_mode;
    if(mode != 1){
        if (x->x_ispolling02)
        {
            x->x_ispolling02 = 0;
            hammergui_stoppolling((t_pd *)x);
        }
    }
    else{
        x->x_ispolling1 = 0;
    };
}

static void mousestate_zero(t_mousestate *x)
{
    switch(x->x_mode){
        case 0:
            hammergui_screenmousexy(gensym("_zero"));
            break;
        case 1:
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
    mousestate_proxy_free(x->x_icpt);
}

static void mousestate_mode(t_mousestate *x, t_floatarg f){
    int mode = (int) f;
    if(mode < 0){
        mode = 0;
    }
    else if(mode > 2){
        mode = 2;
    };

    //must take care of polling if switching b/w mode 1 and any other mode (and vice versa)
    if(
        (mode == 1 && x->x_ispolling02 && x->x_mode != 1) ||
      (mode != 1 && x->x_ispolling1 && x->x_mode == 1)
            ){
        mousestate_nopoll(x);
        x->x_mode = mode;
        mousestate_poll(x);
    }
    else{
        x->x_mode = mode;
    };
}

static void *mousestate_new(void)
{
    t_mousestate *x = (t_mousestate *)pd_new(mousestate_class);
    x->x_ispolling02 = x->x_wasbanged = x->x_waszeroed = 0;
    x->x_ispolling1 = 0;
    outlet_new((t_object *)x, &s_float);
    x->x_hposout = outlet_new((t_object *)x, &s_float);
    x->x_vposout = outlet_new((t_object *)x, &s_float);
    x->x_hdiffout = outlet_new((t_object *)x, &s_float);
    x->x_vdiffout = outlet_new((t_object *)x, &s_float);
    x->x_mode = 0;
    hammergui_bindmouse((t_pd *)x);
    hammergui_willpoll();
    mousestate_reset(x);

    //now dealing with mousestate_proxy
    //mostly copied from iemguts receivecanvas except for depth and new function pointer
   t_glist *glist=(t_glist *)canvas_getcurrent();
   t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);

   x->x_icpt = NULL;
    if(canvas) {
            char buf[MAXPDSTRING];
            snprintf(buf, MAXPDSTRING-1, ".x%lx", (t_int)canvas);
            buf[MAXPDSTRING-1]=0;
            x->x_icpt=mousestate_proxy_new(x, gensym(buf));
    }
    return (x);
}

void mousestate_setup(void)
{
    mousestate_class = class_new(gensym("mousestate"),
				 (t_newmethod)mousestate_new,
				 (t_method)mousestate_free,
				 sizeof(t_mousestate), 0, 0);
    mousestate_proxy_class = class_new(0, 0, 0, sizeof(t_mousestate_proxy), CLASS_NOINLET | CLASS_PD, 0);

    class_addanything(mousestate_proxy_class, mousestate_proxy_anything);
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
