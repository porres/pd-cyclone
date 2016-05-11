/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"

typedef struct _asin {
    t_object x_obj;
    t_inlet *asin;
    t_outlet *x_outlet;
} t_asin;

void *asin_new(void);
static t_int * asin_perform(t_int *w);
static void asin_dsp(t_asin *x, t_signal **sp);

static t_class *asin_class;

static t_int *asin_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
        float f = *in++;
        *out++ = asinf(f);  /* CHECKED no protection against NaNs */
    }
    return (w + 4);
}

static void asin_dsp(t_asin *x, t_signal **sp)
{
    dsp_add(asin_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

void *asin_new(void)
{
    t_asin *x = (t_asin *)pd_new(asin_class);
    x->x_outlet = outlet_new(&x->x_obj, &s_signal);
    return (x);
}

void asin_tilde_setup(void)
{
    asin_class = class_new(gensym("asin~"), (t_newmethod)asin_new, 0,
                           sizeof(t_asin), CLASS_DEFAULT, 0);
    class_addmethod(asin_class, nullfn, gensym("signal"), 0);
    class_addmethod(asin_class, (t_method) asin_dsp, gensym("dsp"), 0);
}