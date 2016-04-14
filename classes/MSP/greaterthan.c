/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER use hasfeeders */

#include "m_pd.h"
#include "sickle/sic.h"

#define GREATERTHAN_DEFRHS  0.  /* CHECKED */


typedef t_sic t_greaterthan;
static t_class *greaterthan_class;

static t_int *greaterthan_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
	/*out++ = (f1 > f2 ? f1 : f2);*/
    /*out++ = (*in1++ == *in2++);*/
    *out++ = (f1 > f2);
    }
    return (w + 5);
}

static void greaterthan_dsp(t_greaterthan *x, t_signal **sp)
{
    dsp_add(greaterthan_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *greaterthan_new(t_symbol *s, int ac, t_atom *av)
{
    t_greaterthan *x = (t_greaterthan *)pd_new(greaterthan_class);
    sic_inlet((t_sic *)x, 1, GREATERTHAN_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void greaterthan_tilde_setup(void)
{
    greaterthan_class = class_new(gensym("greaterthan~"),
			      (t_newmethod)greaterthan_new, 0,
			      sizeof(t_greaterthan), 0, A_GIMME, 0);
    sic_setup(greaterthan_class, greaterthan_dsp, SIC_FLOATTOSIGNAL);
}
