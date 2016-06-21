// Porres 2016

#include "m_pd.h"

typedef struct _freqshift {
    t_object    x_obj;
    t_inlet    *x_inlet_freq;
    t_outlet   *x_out1;
    t_outlet   *x_out2;
    t_float     x_re_ap1_x1;
    t_float     x_re_ap1_x2;
    t_float     x_re_ap1_y1;
    t_float     x_re_ap1_y2;
    t_float     x_re_ap2_x1;
    t_float     x_re_ap2_x2;
    t_float     x_re_ap2_y1;
    t_float     x_re_ap2_y2;
    t_float     x_im_ap1_x1;
    t_float     x_im_ap1_x2;
    t_float     x_im_ap1_y1;
    t_float     x_im_ap1_y2;
    t_float     x_im_ap2_x1;
    t_float     x_im_ap2_x2;
    t_float     x_im_ap2_y1;
    t_float     x_im_ap2_y2;
    } t_freqshift;

static t_class *freqshift_class;

void freqshift_clear(t_freqshift *x)
{
}

static t_int *freqshift_perform(t_int *w)
{
    t_freqshift *x = (t_freqshift *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out1 = (t_float *)(w[5]);
    t_float *out2 = (t_float *)(w[6]);
    t_float r1_x1 = x->x_re_ap1_x1;
    t_float r1_x2 = x->x_re_ap1_x2;
    t_float r1_y1 = x->x_re_ap1_y1;
    t_float r1_y2 = x->x_re_ap1_y2;
    t_float r2_x1 = x->x_re_ap2_x1;
    t_float r2_x2 = x->x_re_ap2_x2;
    t_float r2_y1 = x->x_re_ap2_y1;
    t_float r2_y2 = x->x_re_ap2_y2;
    t_float i1_x1 = x->x_im_ap1_x1;
    t_float i1_x2 = x->x_im_ap1_x2;
    t_float i1_y1 = x->x_im_ap1_y1;
    t_float i1_y2 = x->x_im_ap1_y2;
    t_float i2_x1 = x->x_im_ap2_x1;
    t_float i2_x2 = x->x_im_ap2_x2;
    t_float i2_y1 = x->x_im_ap2_y1;
    t_float i2_y2 = x->x_im_ap2_y2;
    while (nblock--)
    {
        float r1_yn, r1_xn = *in1++;
        float i1_yn, i1_xn = *in1++;
        float r2_xn, r2_yn, i2_xn, i2_yn;
        float re, im;
        r2_xn = r1_yn = 0.94657*r1_xn - 1.94632*r1_x1 + r1_x2 + 1.94632*r1_y1 - 0.94657*r1_y2;
        r1_x2 = r1_x1;
        r1_x1 = r1_xn;
        r1_y2 = r1_y1;
        r1_y1 = r1_yn;
        re = r2_yn = 0.06338*r2_xn - 0.83774*r2_x1 + r2_x2 + 0.83774*r2_y1 - 0.06338*r2_y2;
        r2_x2 = r2_x1;
        r2_x1 = r2_xn;
        r2_y2 = r2_y1;
        r2_y1 = r2_yn;
        i2_xn = i1_yn = -0.260502*i1_xn + 0.02569*i1_x1 + i1_x2 - 0.02569*i1_y1 + 0.260502*i1_y2;
        i1_x2 = i1_x1;
        i1_x1 = i1_xn;
        i1_y2 = i1_y1;
        i1_y1 = i1_yn;
        im = i2_yn = 0.870686*i2_xn - 1.8685*i2_x1 + i2_x2 + 1.8685*i2_y1 - 0.870686*i2_y2;
        i2_x2 = i2_x1;
        i2_x1 = i2_xn;
        i2_y2 = i2_y1;
        i2_y1 = i2_yn;
        *out1++ = re;
        *out2++ = im;
    }
    x->x_re_ap1_x1 = r1_x1;
    x->x_re_ap1_x2 = r1_x2;
    x->x_re_ap1_y1 = r1_y1;
    x->x_re_ap1_y2 = r1_y2;
    x->x_re_ap2_x1 = r2_x1;
    x->x_re_ap2_x2 = r2_x2;
    x->x_re_ap2_y1 = r2_y1;
    x->x_re_ap2_y2 = r2_y2;
    x->x_im_ap1_x1 = i1_x1;
    x->x_im_ap1_x2 = i1_x2;
    x->x_im_ap1_y1 = i1_y1;
    x->x_im_ap1_y2 = i1_y2;
    x->x_im_ap2_x1 = i2_x1;
    x->x_im_ap2_x2 = i2_x2;
    x->x_im_ap2_y1 = i2_y1;
    x->x_im_ap2_y2 = i2_y2;
    return (w + 7);
}

static void freqshift_dsp(t_freqshift *x, t_signal **sp)
{
    dsp_add(freqshift_perform, 6, x, sp[0]->s_n, sp[0]->s_vec,
            sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *freqshift_new(t_floatarg f)
{
    t_freqshift *x = (t_freqshift *)pd_new(freqshift_class);
    x->x_inlet_freq = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_freq, f);
    x->x_out1 = outlet_new((t_object *)x, &s_signal);
    x->x_out2 = outlet_new((t_object *)x, &s_signal);
    x->x_re_ap1_x1 = 0.;
    x->x_re_ap1_x2 = 0.;
    x->x_re_ap1_y1 = 0.;
    x->x_re_ap1_y2 = 0.;
    x->x_re_ap2_x1 = 0.;
    x->x_re_ap2_x2 = 0.;
    x->x_re_ap2_y1 = 0.;
    x->x_re_ap2_y2 = 0.;
    x->x_im_ap1_x1 = 0.;
    x->x_im_ap1_x2 = 0.;
    x->x_im_ap1_y1 = 0.;
    x->x_im_ap1_y2 = 0.;
    x->x_im_ap2_x1 = 0.;
    x->x_im_ap2_x2 = 0.;
    x->x_im_ap2_y1 = 0.;
    x->x_im_ap2_y2 = 0.;
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
