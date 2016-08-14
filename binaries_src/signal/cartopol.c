/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "unstable/fragile.h"
#include "unstable/forky.h"

typedef struct _cartopol
{
    t_object x_obj;
    t_inlet *cartopol;
    t_outlet  *x_out2;
    t_glist  *x_glist;
} t_cartopol;

static t_class *cartopol_class;

static t_int *cartopol_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    t_float *out2 = (t_float *)(w[5]);
    while (nblock--)
    {
        float re = *in1++, im = *in2++;
        *out1++ = hypotf(re, im);
        *out2++ = atan2f(im, re);
    }
    return (w + 6);
}

static t_int *cartopol_perform_nophase(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    while (nblock--)
    {
        float re = *in1++, im = *in2++;
        *out1++ = hypotf(re, im);
    }
    return (w + 5);
}

static t_int *cartopol_perform_no_in(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *out1 = (t_float *)(w[4]);
    t_float *out2 = (t_float *)(w[5]);
    while (nblock--)
    {
        *out1++ = *out2++ = 0;
    }
    return (w + 6);
}

static void cartopol_dsp(t_cartopol *x, t_signal **sp)
{
    if (forky_hasfeeders((t_object *)x, x->x_glist, 0, &s_signal))
        {if (fragile_outlet_connections(x->x_out2))
        dsp_add(cartopol_perform, 5, sp[0]->s_n, sp[0]->s_vec,
                sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
        else
        dsp_add(cartopol_perform_nophase, 4, sp[0]->s_n, sp[0]->s_vec,
                sp[1]->s_vec, sp[2]->s_vec);
        }
    else dsp_add(cartopol_perform_no_in, 5, sp[0]->s_n, sp[0]->s_vec,
                sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *cartopol_new(void)
{
    t_cartopol *x = (t_cartopol *)pd_new(cartopol_class);
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_glist = canvas_getcurrent();
    x->x_out2 = outlet_new((t_object *)x, &s_signal);
    return (x);
}

void cartopol_tilde_setup(void)
{
    cartopol_class = class_new(gensym("cartopol~"), (t_newmethod)cartopol_new, 0,
            sizeof(t_cartopol), 0, 0);
    class_addmethod(cartopol_class, nullfn, gensym("signal"), 0);
    class_addmethod(cartopol_class, (t_method) cartopol_dsp, gensym("dsp"), 0);
}