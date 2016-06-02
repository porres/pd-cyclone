// Porres 2016

#include "m_pd.h"

typedef struct _cross {
    t_object    x_obj;
    t_float     x_input;    //dummy var
    t_inlet    *x_inlet_y;  // main 1st inlet
    t_inlet    *x_inlet_x;  // 2nd inlet
    t_outlet   *x_out1;
    t_outlet   *x_out2;
} t_cross;

static t_class *cross_class;

static t_int *cross_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    t_float *out1 = (t_float *)(w[5]);
    while (nblock--)
    {
	float f1 = *in1++;
	float f2 = *in2++;
	*out1++ = f1 + f2;
    *out2++ = f1 - f2;
    }
    return (w + 6);
}

static void cross_dsp(t_cross *x, t_signal **sp)
{
    dsp_add(cross_perform, 5, sp[0]->s_n, sp[0]->s_vec,
            sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *cross_new(t_floatarg f)
{
    t_cross *x = (t_cross *)pd_new(cross_class);
    x->x_inlet_x = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_x, f);
    x->x_out1 = outlet_new((t_object *)x, &s_signal);
    x->x_out2 = outlet_new((t_object *)x, &s_signal);
    return (x);
}

void cross_tilde_setup(void)
{
    cross_class = class_new(gensym("cross~"), (t_newmethod)cross_new, 0,
        sizeof(t_cross), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(cross_class, (t_method)cross_dsp, gensym("dsp"), 0);
    CLASS_MAINSIGNALIN(cross_class, t_cross, x_input); // no main signal in...
}
