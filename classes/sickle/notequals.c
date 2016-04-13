/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER use hasfeeders */

#include "m_pd.h"
#include "sickle/sic.h"

#define NOTEQUALS_DEFRHS  0.  /* CHECKED */

typedef t_sic t_notequals;
static t_class *notequals_class;

static t_int *notequals_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
    *out++ = (f1 != f2);
    }
    return (w + 5);
}

static void notequals_dsp(t_notequals *x, t_signal **sp)
{
    dsp_add(notequals_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *notequals_new(t_symbol *s, int ac, t_atom *av)
{
    t_notequals *x = (t_notequals *)pd_new(notequals_class);
    sic_inlet((t_sic *)x, 1, NOTEQUALS_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void notequals_tilde_setup(void)
{
    notequals_class = class_new(gensym("notequals~"),
			      (t_newmethod)notequals_new, 0,
			      sizeof(t_notequals), 0, A_GIMME, 0);
    sic_setup(notequals_class, notequals_dsp, SIC_FLOATTOSIGNAL);
}
