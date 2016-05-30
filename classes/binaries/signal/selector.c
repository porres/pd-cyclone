// Porres 2016

#include "m_pd.h"

typedef struct _selector
{
    t_object x_obj;
    t_float  x_main_in;
    t_float  x_inputs;
} t_selector;

static t_class *selector_class;

static t_int *selector_perform(t_int *w)
{
    t_selector *x = (t_selector *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *chn1 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    while (nblock--)
    {
        float input = *in++;
        *out++ = input == 1 ? *chn1++ : 0;
    }
    return (w + 6);
}

static void selector_dsp(t_selector *x, t_signal **sp)
{
    dsp_add(selector_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *selector_new(t_floatarg f)
{
    t_selector *x = (t_selector *)pd_new(selector_class);
    x->x_inputs = f;
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void selector_tilde_setup(void)
{
    selector_class = class_new(gensym("selector~"), (t_newmethod)selector_new, 0,
            sizeof(t_selector), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(selector_class, (t_method)selector_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(selector_class, t_selector, x_main_in);
}
