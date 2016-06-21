// Porres 2016

#include "m_pd.h"

typedef struct _freqshift {
    t_object    x_obj;
    t_inlet    *x_inlet_freq;
    t_outlet   *x_out;
    t_float     x_r1x1;
    t_float     x_r1x2;
    t_float     x_r1y1;
    t_float     x_r1y2;
    
    t_float     x_r2x1;
    t_float     x_r2x2;
    t_float     x_r2y1;
    t_float     x_r2y2;
    } t_freqshift;

static t_class *freqshift_class;

void freqshift_clear(t_freqshift *x)
{
    x->x_r1x1 = x->x_r1x2 = x->x_r1y1 = x->x_r1y2 = 0.;
    x->x_r2x1 = x->x_r2x2 = x->x_r2y1 = x->x_r2y2 = 0.;
}

static t_int *freqshift_perform(t_int *w)
{
    t_freqshift *x = (t_freqshift *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float r1x1 = x->x_r1x1;
    t_float r1x2 = x->x_r1x2;
    t_float r1y1 = x->x_r1y1;
    t_float r1y2 = x->x_r1y2;
    
    t_float r2x1 = x->x_r2x1;
    t_float r2x2 = x->x_r2x2;
    t_float r2y1 = x->x_r2y1;
    t_float r2y2 = x->x_r2y2;
    while (nblock--)
    {
        float r2xn, r2yn, r1yn, r1xn = *in1++, f = *in2++;
//        r1yn = 0.94657*r1xn - 1.94632*r1x1 + r1x2 + 1.94632*r1y1 - 0.94657*r1y2;
        r1yn = -0.260502*r1xn + 0.02569*r1x1 + r1x2 - 0.02569*r1y1 + 0.260502*r1y2;

        r1x2 = r1x1;
        r1x1 = r1xn;
        r1y2 = r1y1;
        r1y1 = r1yn;
        r2xn = r1yn;
//        r2yn = 0.06338*r2xn - 0.83774*r2x1 + r2x2 + 0.83774*r2y1 - 0.06338*r2y2;
        r2yn = 0.870686*r2xn - 1.8685*r2x1 + r2x2 + 1.8685*r2y1 - 0.870686*r2y2;
        r2x2 = r2x1;
        r2x1 = r2xn;
        r2y2 = r2y1;
        r2y1 = r2yn;
        *out++ = r2yn;
    }
    x->x_r1x1 = r1x1;
    x->x_r1x2 = r1x2;
    x->x_r1y1 = r1y1;
    x->x_r1y2 = r1y2;
    
    x->x_r2x1 = r2x1;
    x->x_r2x2 = r2x2;
    x->x_r2y1 = r2y1;
    x->x_r2y2 = r2y2;
    return (w + 6);
}

static void freqshift_dsp(t_freqshift *x, t_signal **sp)
{
    dsp_add(freqshift_perform, 5, x, sp[0]->s_n, sp[0]->s_vec,
        sp[1]->s_vec, sp[2]->s_vec);
}

static void *freqshift_new(t_floatarg f)
{
    t_freqshift *x = (t_freqshift *)pd_new(freqshift_class);
    x->x_inlet_freq = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_freq, f);
    x->x_out = outlet_new((t_object *)x, &s_signal);
    x->x_r1x1 = 0;
    x->x_r1y1 = 0;
    x->x_r1x2 = 0;
    x->x_r1y2 = 0;
    
    x->x_r2x1 = 0;
    x->x_r2y1 = 0;
    x->x_r2x2 = 0;
    x->x_r2y2 = 0;
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