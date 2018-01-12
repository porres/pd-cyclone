// Porres 2016

#include "m_pd.h"

typedef struct _hilbert {
    t_object    x_obj;
    t_outlet   *x_out1;
    t_outlet   *x_out2;
    t_float     x_r1x1;
    t_float     x_r1x2;
    t_float     x_r1y1;
    t_float     x_r1y2;
    t_float     x_r2x1;
    t_float     x_r2x2;
    t_float     x_r2y1;
    t_float     x_r2y2;
    t_float     x_i1x1;
    t_float     x_i1x2;
    t_float     x_i1y1;
    t_float     x_i1y2;
    t_float     x_i2x1;
    t_float     x_i2x2;
    t_float     x_i2y1;
    t_float     x_i2y2;
    } t_hilbert;

static t_class *hilbert_class;

void hilbert_clear(t_hilbert *x)
{
    x->x_r1x1 = x->x_r1x2 = x->x_r1y1 = x->x_r1y2 = 0.;
    x->x_r2x1 = x->x_r2x2 = x->x_r2y1 = x->x_r2y2 = 0.;
    x->x_i1x1 = x->x_i1x2 = x->x_i1y1 = x->x_i1y2 = 0.;
    x->x_i2x1 = x->x_i2x2 = x->x_i2y1 = x->x_i2y2 = 0.;
}

static t_int *hilbert_perform(t_int *w)
{
    t_hilbert *x = (t_hilbert *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    t_float *out2 = (t_float *)(w[5]);
    t_float r1x1 = x->x_r1x1;
    t_float r1x2 = x->x_r1x2;
    t_float r1y1 = x->x_r1y1;
    t_float r1y2 = x->x_r1y2;
    t_float r2x1 = x->x_r2x1;
    t_float r2x2 = x->x_r2x2;
    t_float r2y1 = x->x_r2y1;
    t_float r2y2 = x->x_r2y2;
    t_float i1x1 = x->x_i1x1;
    t_float i1x2 = x->x_i1x2;
    t_float i1y1 = x->x_i1y1;
    t_float i1y2 = x->x_i1y2;
    t_float i2x1 = x->x_i2x1;
    t_float i2x2 = x->x_i2x2;
    t_float i2y1 = x->x_i2y1;
    t_float i2y2 = x->x_i2y2;
    while (nblock--)
    {
        float r1xn, r1yn, r2xn, r2yn, i1xn, i1yn, i2xn, i2yn;
        r1xn = i1xn = *in1++;
        r1yn = -0.260502*r1xn + 0.02569*r1x1 + r1x2 - 0.02569*r1y1 + 0.260502*r1y2;
        r1x2 = r1x1;
        r1x1 = r1xn;
        r1y2 = r1y1;
        r1y1 = r1yn;
        r2xn = r1yn;
        r2yn = 0.870686*r2xn - 1.8685*r2x1 + r2x2 + 1.8685*r2y1 - 0.870686*r2y2;
        r2x2 = r2x1;
        r2x1 = r2xn;
        r2y2 = r2y1;
        r2y1 = r2yn;
        float re = r2yn;
       i1yn = 0.94657*i1xn - 1.94632*i1x1 + i1x2 + 1.94632*i1y1 - 0.94657*i1y2;
        i1x2 = i1x1;
        i1x1 = i1xn;
        i1y2 = i1y1;
        i1y1 = i1yn;
        i2xn = i1yn;
        i2yn = 0.06338*i2xn - 0.83774*i2x1 + i2x2 + 0.83774*i2y1 - 0.06338*i2y2;
        i2x2 = i2x1;
        i2x1 = i2xn;
        i2y2 = i2y1;
        i2y1 = i2yn;
        float im = i2yn;
        *out1++ = re;
        *out2++ = im;
    }
    x->x_r1x1 = r1x1;
    x->x_r1x2 = r1x2;
    x->x_r1y1 = r1y1;
    x->x_r1y2 = r1y2;
    x->x_r2x1 = r2x1;
    x->x_r2x2 = r2x2;
    x->x_r2y1 = r2y1;
    x->x_r2y2 = r2y2;
    x->x_i1x1 = i1x1;
    x->x_i1x2 = i1x2;
    x->x_i1y1 = i1y1;
    x->x_i1y2 = i1y2;
    x->x_i2x1 = i2x1;
    x->x_i2x2 = i2x2;
    x->x_i2y1 = i2y1;
    x->x_i2y2 = i2y2;
    return (w + 6);
}

static void hilbert_dsp(t_hilbert *x, t_signal **sp)
{
    dsp_add(hilbert_perform, 5, x, sp[0]->s_n, sp[0]->s_vec,
        sp[1]->s_vec, sp[2]->s_vec);
}

static void *hilbert_new(void){
    t_hilbert *x = (t_hilbert *)pd_new(hilbert_class);
    x->x_out1 = outlet_new((t_object *)x, &s_signal);
    x->x_out2 = outlet_new((t_object *)x, &s_signal);
    x->x_r1x1 = x->x_r1x2 = x->x_r1y1 = x->x_r1y2 = 0.;
    x->x_r2x1 = x->x_r2x2 = x->x_r2y1 = x->x_r2y2 = 0.;
    x->x_i1x1 = x->x_i1x2 = x->x_i1y1 = x->x_i1y2 = 0.;
    x->x_i2x1 = x->x_i2x2 = x->x_i2y1 = x->x_i2y2 = 0.;
    return (x);
}

void hilbert_tilde_setup(void)
{
    hilbert_class = class_new(gensym("cyclone/hilbert~"), (t_newmethod)hilbert_new, 0,
        sizeof(t_hilbert), CLASS_DEFAULT, 0);
    class_addcreator((t_newmethod)hilbert_new, gensym("hilbert~"), 0);
    class_addmethod(hilbert_class, (t_method)hilbert_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(hilbert_class, nullfn, gensym("signal"), 0);
    class_addmethod(hilbert_class, (t_method) hilbert_clear, gensym("clear"), 0);
    class_sethelpsymbol(hilbert_class, gensym("hilbert~"));
}
