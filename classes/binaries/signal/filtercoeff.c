// Porres 2016

#include "m_pd.h"
#include <math.h>

#define PI M_PI

typedef struct _filtercoeff {
    t_object    x_obj;
    t_inlet    *x_inlet_gain;
    t_inlet    *x_inlet_q;
    t_outlet   *x_out_a0;
    t_outlet   *x_out_a1;
    t_outlet   *x_out_a2;
    t_outlet   *x_out_b1;
    t_outlet   *x_out_b2;
    t_float     x_nyq;
    t_float     x_lastq;
    } t_filtercoeff;

static t_class *filtercoeff_class;


static t_int *filtercoeff_perform(t_int *w)
{
    t_filtercoeff *x = (t_filtercoeff *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out1 = (t_float *)(w[6]);
    t_float *out2 = (t_float *)(w[7]);
    t_float *out3 = (t_float *)(w[8]);
    t_float *out4 = (t_float *)(w[9]);
    t_float *out5 = (t_float *)(w[10]);
    t_float nyq = x->x_nyq;
    t_float lastq = x->x_lastq;
    while (nblock--)
    {
        float omega, alphaQ, a0, a1, a2, b0, b1, b2, f = *in1++, q = *in3++;
        if (q < 0) q = lastq;
        lastq = q;
        if (f < 0) f = 0;
        if (f > nyq) f = nyq;
        omega = f * PI/nyq;
        // ALLPASS
        alphaQ = sinf(omega) / (2*q);
        b0 = alphaQ + 1;
        a0 = (1 - alphaQ) / b0;
        a1 = -2*cosf(omega) / b0;
        a2 = 1;
        b1 = a1;
        b2 = a0;
        *out1++ = a0;
        *out2++ = a1;
        *out3++ = a2;
        *out4++ = b1;
        *out5++ = b2;
    }
    x->x_lastq = lastq;
    return (w + 11);
}

static void filtercoeff_dsp(t_filtercoeff *x, t_signal **sp)
{
    x->x_nyq = sp[0]->s_sr / 2;
    dsp_add(filtercoeff_perform, 10, x, sp[0]->s_n, sp[0]->s_vec,sp[1]->s_vec, sp[2]->s_vec,
            sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec);
}

static void *filtercoeff_new(t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
    t_filtercoeff *x = (t_filtercoeff *)pd_new(filtercoeff_class);
    x->x_inlet_gain = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_gain, f2);
    x->x_inlet_q = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_q, f3);
    x->x_out_a0 = outlet_new((t_object *)x, &s_signal);
    x->x_out_a1 = outlet_new((t_object *)x, &s_signal);
    x->x_out_a2 = outlet_new((t_object *)x, &s_signal);
    x->x_out_b1 = outlet_new((t_object *)x, &s_signal);
    x->x_out_b2 = outlet_new((t_object *)x, &s_signal);
    x->x_lastq = 1;
    return (x);
}

void filtercoeff_tilde_setup(void)
{
    filtercoeff_class = class_new(gensym("filtercoeff~"), (t_newmethod)filtercoeff_new, 0,
        sizeof(t_filtercoeff), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(filtercoeff_class, (t_method)filtercoeff_dsp, gensym("dsp"), 0);
    class_addmethod(filtercoeff_class, nullfn, gensym("signal"), 0);
}
