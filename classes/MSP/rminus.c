/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER use hasfeeders */

#include "m_pd.h"
#include "sickle/sic.h"

#define RMINUS_DEFRHS  0.  /* CHECKED */

typedef t_sic t_rminus;
static t_class *rminus_class;

static t_int *rminus_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) *out++ = *in2++ - *in1++;
    return (w + 5);
}

static void rminus_dsp(t_rminus *x, t_signal **sp)
{
    dsp_add(rminus_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *rminus_new(t_symbol *s, int ac, t_atom *av)
{
    t_rminus *x = (t_rminus *)pd_new(rminus_class);
    sic_inlet((t_sic *)x, 1, RMINUS_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void rminus_tilde_setup(void)
{
    rminus_class = class_new(gensym("rminus~"),
			      (t_newmethod)rminus_new, 0,
			      sizeof(t_rminus), 0, A_GIMME, 0);
    sic_setup(rminus_class, rminus_dsp, SIC_FLOATTOSIGNAL);
}
