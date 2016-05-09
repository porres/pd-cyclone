/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

/* LATER select the mode fitter-optionally */

#define SLIDE_DEFNUP    0.
#define SLIDE_DEFNDOWN  0.

typedef struct _slide
{
    t_sic    x_sic;
    int      x_nup;
    int      x_ndown;
    double   x_upcoef;
    double   x_downcoef;
    t_float  x_last;
} t_slide;

static t_class *slide_class;


static t_int *slide_perform(t_int *w)

// The formula is y(n) = y(n-1) + ((x(n) - y(n-1)) / slide).

{
    t_slide *x = (t_slide *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_float last = x->x_last;
    while (nblock--)
    {
    	t_float f = *in++;
	if (f > last)
	{
	    if (x->x_nup > 1)
	    {
		*out++ = (last += (f - last) * x->x_upcoef);
		continue;
	    }
	}
	else if (f < last)
	{
	    if (x->x_ndown > 1)
	    {
		*out++ = (last += (f - last) * x->x_downcoef);
		continue;
	    }
	}
	*out++ = last = f;
    }
    x->x_last = (PD_BIGORSMALL(last) ? 0. : last);
    return (w + 5);
}

static void slide_dsp(t_slide *x, t_signal **sp)
{
    dsp_add(slide_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}


static void slide_reset(t_slide *x)
{
x->x_last = 0.;
// Sets the current output sample value to 0(the next incoming value will smoothly transition from that 0).
}

static void slide_slide_up(t_slide *x, t_floatarg f)
{
    int i = (int)f;
    if (i > 1)  /* CHECKME if 1 and 0 differ in any way */
    {
	x->x_nup = i;
	x->x_upcoef = 1. / (float)i;
    }
    else
    {
	x->x_nup = 0;
	x->x_upcoef = 0.;
    }
}

static void slide_slide_down(t_slide *x, t_floatarg f)
{
    int i = (int)f;
    if (i > 1)  /* CHECKME if 1 and 0 differ in any way */
    {
	x->x_ndown = i;
	x->x_downcoef = 1. / (float)i;
    }
    else
    {
	x->x_ndown = 0;
	x->x_downcoef = 0.;
    }
}

static void *slide_new(t_symbol *s, int ac, t_atom *av)
{
    t_slide *x = (t_slide *)pd_new(slide_class);
    
    float f1 = SLIDE_DEFNUP;
    float f2 = SLIDE_DEFNDOWN;
    if (ac && av->a_type == A_FLOAT)
    {
	f1 = av->a_w.w_float;
	ac--; av++;
	if (ac && av->a_type == A_FLOAT)
	    f2 = av->a_w.w_float;
    }
    slide_slide_up(x, f1);
    slide_slide_down(x, f2);
    x->x_last = 0.;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("slide_up"));
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("slide_down"));
    outlet_new((t_object *)x, &s_signal);
    return (x);
}


void slide_tilde_setup(void)
{
    slide_class = class_new(gensym("slide~"),
				 (t_newmethod)slide_new, 0,
				 sizeof(t_slide), 0, A_GIMME, 0);
    sic_setup(slide_class, slide_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(slide_class, (t_method)slide_slide_up,
		    gensym("slide_up"), A_FLOAT, 0);
    class_addmethod(slide_class, (t_method)slide_slide_down,
		    gensym("slide_down"), A_FLOAT, 0);
    class_addmethod(slide_class, (t_method)slide_slide_down,
                    gensym("reset"), 0);
}
