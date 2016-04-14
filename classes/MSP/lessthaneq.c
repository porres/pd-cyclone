/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER use hasfeeders */

#include "m_pd.h"
#include "sickle/sic.h"

#define LESSTHANEQ_DEFRHS  0.  /* CHECKED */


typedef t_sic t_lessthaneq;
static t_class *lessthaneq_class;

static t_int *lessthaneq_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
    *out++ = (f1 < f2); /* the great code */
    }
    return (w + 5);
}

static void lessthaneq_dsp(t_lessthaneq *x, t_signal **sp)
{
    dsp_add(lessthaneq_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *lessthaneq_new(t_symbol *s, int ac, t_atom *av)
{
    t_lessthaneq *x = (t_lessthaneq *)pd_new(lessthaneq_class);
    sic_inlet((t_sic *)x, 1, LESSTHANEQ_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void lessthaneq_tilde_setup(void)
{
    lessthaneq_class = class_new(gensym("lessthaneq~"),
			      (t_newmethod)lessthaneq_new, 0,
			      sizeof(t_lessthaneq), 0, A_GIMME, 0);
    sic_setup(lessthaneq_class, lessthaneq_dsp, SIC_FLOATTOSIGNAL);
}
