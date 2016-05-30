// Porres 2016

#include "m_pd.h"

typedef struct _selector
{
    t_object   x_obj;
    t_float    x_main_in;
    t_float    x_inputs;
    t_float  **x_ivecs; // copying from matrix
    t_float  **x_ovecs; // copying from matrix
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
        *out++ = (int)*in++ == 1 ? *chn1++ : 0;
    }
    return (w + 6);
}

static void selector_dsp(t_selector *x, t_signal **sp)
{
    dsp_add(selector_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

/* static void selector_dsp(t_selector *x, t_signal **sp) // trying to copy from matrix...
{
    int i;
    t_float **vecp = x->x_ivecs;
    t_signal **sigp = sp;
    for (i = 0; i < x->x_inputs; i++)
    {
    *vecp++ = (*sigp++)->s_vec;
    }
    vecp = x->x_ovecs;
    for (i = 0; i < 1; i++) *vecp++ = (*sigp++)->s_vec;
    dsp_add(selector_perform, 2, x, sp[0]->s_n);
} */

static void *selector_new(t_symbol *s, int argc, t_atom *argv)
{
    t_selector *x = (t_selector *)pd_new(selector_class);
    x->x_inputs = 2;
    int i;
    int argnum = 0;
    while(argc > 0)
    {
        if(argv -> a_type == A_FLOAT)
        {
            t_float argval = atom_getfloatarg(0, argc, argv);
            switch(argnum)
            {
                case 0:
                    if(argval < 1) x->x_inputs = 2;
                    else x->x_inputs = (int)argval + 1;
                    break;
                default:
                    break;
            };
            argc--;
            argv++;
            argnum++;
        }
    }
    for (i = 1; i < x->x_inputs; i++)
    {
        pd_float((t_pd *)inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal), 0.);
    };
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void selector_tilde_setup(void)
{
    selector_class = class_new(gensym("selector~"), (t_newmethod)selector_new, 0,
            sizeof(t_selector), CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(selector_class, (t_method)selector_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(selector_class, t_selector, x_main_in);
}
