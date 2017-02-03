/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

static t_class *kink_class;

typedef struct _kink {
    t_object x_obj;
    t_inlet  *x_slope_inlet;
} t_kink;

static t_int *kink_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	float iph = *in1++;
	float slope = *in2++;
    slope = (slope < 0 ? 0 : slope);
	float oph = iph * slope;
	if (oph > .5)
	{
	    slope = 1. / (slope + slope);  // x(y=.5)
	    if (slope == 1.)
		*out++ = 0;
	    else
		*out++ = (iph - slope) / (2. - (slope + slope)) + .5;
	}
	else *out++ = oph;
    }
    return (w + 5);
}

static void kink_dsp(t_kink *x, t_signal **sp)
{
    dsp_add(kink_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *kink_free(t_kink *x)
{
    inlet_free(x->x_slope_inlet);
    return (void *)x;
}

static void *kink_new(t_floatarg f)
{
    t_kink *x = (t_kink *)pd_new(kink_class);
    f = (f < 0 ? 0 : f);
    f = (f == 0 ? 1 : f);
    x->x_slope_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_slope_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void kink_tilde_setup(void)
{
    kink_class = class_new(gensym("kink~"),
			   (t_newmethod)kink_new, 0,
			   sizeof(t_kink), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(kink_class, nullfn, gensym("signal"), 0);
    class_addmethod(kink_class, (t_method)kink_dsp, gensym("dsp"), A_CANT, 0);
}
