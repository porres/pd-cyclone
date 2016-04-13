/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER use hasfeeders */

#include "m_pd.h"
#include "sickle/sic.h"

#define RDIV_DEFRHS  0.  /* CHECKED */

typedef t_sic t_rdiv;
static t_class *rdiv_class;

static t_int *rdiv_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
    /* CHECKED incompatible: c74 outputs NaNs.
    The line below is consistent with Pd's /~, LATER rethink. */
    /* LATER multiply by reciprocal if in1 has no signal feeders */
        *out++ = (f1 == 0. ? 0. : *in2++ / f1);
    }
    return (w + 5);
}

static void rdiv_dsp(t_rdiv *x, t_signal **sp)
{
    dsp_add(rdiv_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *rdiv_new(t_symbol *s, int ac, t_atom *av)
{
    t_rdiv *x = (t_rdiv *)pd_new(rdiv_class);
    sic_inlet((t_sic *)x, 1, RDIV_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void rdiv_tilde_setup(void)
{
    rdiv_class = class_new(gensym("rdiv~"),
			      (t_newmethod)rdiv_new, 0,
			      sizeof(t_rdiv), 0, A_GIMME, 0);
    sic_setup(rdiv_class, rdiv_dsp, SIC_FLOATTOSIGNAL);
}
