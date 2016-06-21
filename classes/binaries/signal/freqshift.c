// Porres 2016

#include "m_pd.h"
#include <math.h>

#define PI M_PI

typedef struct _freqshift {
    t_object    x_obj;
    t_inlet    *x_inlet_freq;
    t_outlet   *x_out;
    t_float     x_nyq;
    t_float     x_x1;
    t_float     x_x2;
    t_float     x_y1;
    t_float     x_y2;
    } t_freqshift;

static t_class *freqshift_class;

void freqshift_clear(t_freqshift *x)
{
    x->x_x1 = x->x_x2 = x->x_y1 = x->x_y2 = 0.;
}

static t_int *freqshift_perform(t_int *w)
{
    t_freqshift *x = (t_freqshift *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float x1 = x->x_x1;
    t_float x2 = x->x_x2;
    t_float y1 = x->x_y1;
    t_float y2 = x->x_y2;
    while (nblock--)
    {
        float b0, a0, a1, yn, xn = *in1++, f = *in2++;
        a0 = 0.94657;
        a1 = -1.94632;
        yn = a0*xn + a1*x1 + x2 - a1*y1 - a0*y2;
        x2 = x1;
        x1 = xn;
        y2 = y1;
        y1 = yn;
        *out++ = yn;
    }
    x->x_x1 = x1;
    x->x_x2 = x2;
    x->x_y1 = y1;
    x->x_y2 = y2;
    return (w + 6);
}

static void freqshift_dsp(t_freqshift *x, t_signal **sp)
{
    x->x_nyq = sp[0]->s_sr / 2;
    dsp_add(freqshift_perform, 5, x, sp[0]->s_n, sp[0]->s_vec,
        sp[1]->s_vec, sp[2]->s_vec);
}

static void *freqshift_new(t_floatarg f)
{
    t_freqshift *x = (t_freqshift *)pd_new(freqshift_class);
    x->x_inlet_freq = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_freq, f);
    x->x_out = outlet_new((t_object *)x, &s_signal);
    x->x_x1 = 0;
    x->x_y1 = 0;
    x->x_x2 = 0;
    x->x_y2 = 0;
    return (x);
}

void freqshift_tilde_setup(void)
{
    freqshift_class = class_new(gensym("freqshift~"), (t_newmethod)freqshift_new, 0,
        sizeof(t_freqshift), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(freqshift_class, (t_method)freqshift_dsp, gensym("dsp"), 0);
    class_addmethod(freqshift_class, nullfn, gensym("signal"), 0);
    class_addmethod(freqshift_class, (t_method) freqshift_clear, gensym("clear"), 0);
}