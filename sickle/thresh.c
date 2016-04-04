/* Copyright (c) 2016 Porres
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

#define THRESH_DEFLO  0.
#define THRESH_DEFHI  0.

typedef struct _thresh
{
    t_sic    x_sic;
    t_float  x_last;
} t_thresh;

static t_class *thresh_class;

static t_int *thresh_perform(t_int *w)
{
    t_thresh *x = (t_thresh *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    t_float last = x->x_last;
    while (nblock--)
    {
        float in = *in1++;
        float lo = *in2++;
        float hi = *in3++;
        last = (in > lo && (in >= hi || last));
        *out++ = last;
    }
    x->x_last = last;
    return (w + 7);
}

static void thresh_dsp(t_thresh *x, t_signal **sp)
{
    dsp_add(thresh_perform, 6, x, sp[0]->s_n,
            sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *thresh_new(t_symbol *s, int ac, t_atom *av)
{
    t_thresh *x = (t_thresh *)pd_new(thresh_class);
    sic_inlet((t_sic *)x, 1, THRESH_DEFLO, 0, ac, av);
    sic_inlet((t_sic *)x, 2, THRESH_DEFHI, 1, ac, av);
    outlet_new((t_object *)x, &s_signal);
    x->x_last = 0;
    return (x);
}

void thresh_tilde_setup(void)
{
    thresh_class = class_new(gensym("thresh~"),
                                (t_newmethod)thresh_new, 0,
                                sizeof(t_thresh), 0, A_GIMME, 0);
    sic_setup(thresh_class, thresh_dsp, SIC_FLOATTOSIGNAL);
}