/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
// #include "sickle/sic.h"

typedef struct _plusequals
{
    t_object x_obj;
    t_float  x_sum;
    t_inlet  *x_triglet;
} t_plusequals;

static t_class *plusequals_class;


/* / ---------------------------------------------------
// Functions signature
// ---------------------------------------------------
void * trunc_new(void);// Constructor
void trunc_destroy(t_trunc *x); //Destructor
static t_int * trunc_perform(t_int *w); //Perform function
static void trunc_dsp(t_trunc *x, t_signal **sp); //DSP function */


static t_int *plusequals_perform(t_int *w)
{
    t_plusequals *x = (t_plusequals *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float sum = x->x_sum;
    while (nblock--)
    {
        float x1 = *in1++;
        float x2 = *in2++;
        sum = sum * (x2 == 0);
        *out++ = sum += x1;
    }
    x->x_sum = sum;
    return (w + 6);
}

static void plusequals_dsp(t_plusequals *x, t_signal **sp)
{
    dsp_add(plusequals_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}


static void plusequals_bang(t_plusequals *x)
{
    x->x_sum = 0;
}

static void plusequals_set(t_plusequals *x, t_floatarg f)
{
    x->x_sum = f;
}

static void *plusequals_free(t_plusequals *x)
{
    inlet_free(x->x_triglet);
    return (void *)x;
}

static void *plusequals_new(t_floatarg f)
{
    t_plusequals *x = (t_plusequals *)pd_new(plusequals_class);
    x->x_sum = f;
    x->x_triglet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void plusequals_tilde_setup(void)
{
    plusequals_class = class_new(gensym("plusequals~"),
			  (t_newmethod)plusequals_new, 0,
			  sizeof(t_plusequals), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(plusequals_class, nullfn, gensym("signal"), 0);
    class_addmethod(plusequals_class, (t_method) plusequals_dsp, gensym("dsp"), 0);
    class_addbang(plusequals_class, plusequals_bang);
    class_addmethod(plusequals_class, (t_method)plusequals_set,
                    gensym("set"), A_FLOAT, 0);
}


