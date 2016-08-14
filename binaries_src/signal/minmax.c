/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <math.h>
#include "unstable/forky.h"

typedef struct _minmax
{
    t_object x_obj;
    t_inlet  *x_inlet;
    t_glist  *x_glist;
    t_float    x_min;
    t_float    x_max;
    t_int      x_feederflag;
    t_outlet  *x_minout;
    t_outlet  *x_maxout;
} t_minmax;

static t_class *minmax_class;

static void minmax_bang(t_minmax *x)
{
    if (x->x_feederflag) {
        outlet_float(x->x_maxout, x->x_max);
        outlet_float(x->x_minout, x->x_min);
        }
    else {
        outlet_float(x->x_maxout, 0);
        outlet_float(x->x_minout, 0);
    }
}

static void minmax_reset(t_minmax *x)
{
    x->x_min = INFINITY;
    x->x_max = -INFINITY;
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
        if (reset != 0) fmin = fmax = f;
        else {
            if (f < fmin) fmin = f;
            if (f > fmax) fmax = f;
            }
        *outmin++ = fmin;
        *outmax++ = fmax;
    }
    x->x_min = fmin;
    x->x_max = fmax;
    return (w + 7);
}

static t_int *minmax_perform_no_in(t_int *w)
{
    t_minmax *x = (t_minmax *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *outmin = (t_float *)(w[5]);
    t_float *outmax = (t_float *)(w[6]);
    while (nblock--)
    {
        *outmin++ = *outmax++ = 0;
    }
    return (w + 7);
}

static void minmax_dsp(t_minmax *x, t_signal **sp)
{
    x->x_feederflag = forky_hasfeeders((t_object *)x, x->x_glist, 0, &s_signal);
    if (x->x_feederflag) dsp_add(minmax_perform, 6, x, sp[0]->s_n,
        sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
    else dsp_add(minmax_perform_no_in, 6, x, sp[0]->s_n, sp[0]->s_vec,
        sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *minmax_new(void)
{
    t_minmax *x = (t_minmax *)pd_new(minmax_class);
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_glist = canvas_getcurrent();
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
