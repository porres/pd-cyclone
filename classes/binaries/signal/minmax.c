/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

typedef struct _minmax
{
    t_object x_obj;
    t_inlet  *x_inlet;
    t_float    x_min;
    t_float    x_max;
    t_outlet  *x_minout;
    t_outlet  *x_maxout;
} t_minmax;

static t_class *minmax_class;

static void minmax_bang(t_minmax *x)
{
    outlet_float(x->x_maxout, x->x_max);
    outlet_float(x->x_minout, x->x_min);
}

static void minmax_reset(t_minmax *x)
{
    x->x_min = x->x_max = 0;
}

static t_int *minmax_perform(t_int *w)
{
    t_minmax *x = (t_minmax *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *outmin = (t_float *)(w[5]);
    t_float *outmax = (t_float *)(w[6]);
    t_float fmin = x->x_min;
    t_float fmax = x->x_max;
    while (nblock--)
    {
	t_float f = *in1++;
    t_float reset = *in2++;
    if (reset != 0) {fmin = fmax = 0;} // CHECK ME
    if (f < fmin) fmin = f * (reset == 0);
	else if (f > fmax) fmax = f * (reset == 0);
	*outmin++ = fmin;
	*outmax++ = fmax;
    }
    x->x_min = fmin;
    x->x_max = fmax;
    return (w + 7);
}

static void minmax_dsp(t_minmax *x, t_signal **sp)
{
    dsp_add(minmax_perform, 6, x, sp[0]->s_n, sp[0]->s_vec,
	    sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *minmax_new(void)
{
    t_minmax *x = (t_minmax *)pd_new(minmax_class);
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_minout = outlet_new((t_object *)x, &s_float);
    x->x_maxout = outlet_new((t_object *)x, &s_float);
    minmax_reset(x);
    return (x);
}

void minmax_tilde_setup(void)
{
    minmax_class = class_new(gensym("minmax~"),
        (t_newmethod)minmax_new, 0, sizeof(t_minmax), 0, 0);
    class_addmethod(minmax_class, nullfn, gensym("signal"), 0);
    class_addmethod(minmax_class, (t_method)minmax_dsp, gensym("dsp"), A_CANT, 0);
    class_addbang(minmax_class, minmax_bang);
    class_addmethod(minmax_class, (t_method)minmax_reset, gensym("reset"), 0);
}
