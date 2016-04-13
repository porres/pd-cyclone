/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "hammer/gui.h"

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
    if (x->x_wasbanged)
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
    if (x->x_ispolling)
    {
	x->x_wasbanged = 1;
	mousestate_dobang(x, f1, f2);
    }
}

static void mousestate_bang(t_mousestate *x)
{
    hammergui_mousexy(gensym("_bang"));
    x->x_wasbanged = 1;
}

static void mousestate_poll(t_mousestate *x)
{
    if (!x->x_ispolling)
    {
	x->x_ispolling = 1;
	hammergui_startpolling((t_pd *)x);
    }
}

static void mousestate_nopoll(t_mousestate *x)
{
    if (x->x_ispolling)
    {
	x->x_ispolling = 0;
	hammergui_stoppolling((t_pd *)x);
    }
}

static void mousestate_zero(t_mousestate *x)
{
    hammergui_mousexy(gensym("_zero"));
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

static void *mousestate_new(void)
{
    t_mousestate *x = (t_mousestate *)pd_new(mousestate_class);
    x->x_ispolling = x->x_wasbanged = x->x_waszeroed = 0;
    outlet_new((t_object *)x, &s_float);
    x->x_hposout = outlet_new((t_object *)x, &s_float);
    x->x_vposout = outlet_new((t_object *)x, &s_float);
    x->x_hdiffout = outlet_new((t_object *)x, &s_float);
    x->x_vdiffout = outlet_new((t_object *)x, &s_float);
    hammergui_bindmouse((t_pd *)x);
    hammergui_willpoll();
    mousestate_reset(x);
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
}

void MouseState_setup(void)
{
    mousestate_setup();
}
