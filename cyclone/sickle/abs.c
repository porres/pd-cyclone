/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

typedef t_sic t_abs;
static t_class *abs_class;

static t_int *abs_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
    	*out++ = (f >= 0 ? f : -f);
    }
    return (w + 4);
}

static void abs_dsp(t_abs *x, t_signal **sp)
{
    dsp_add(abs_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *abs_new(void)
{
    t_abs *x = (t_abs *)pd_new(abs_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void abs_tilde_setup(void)
{
    abs_class = class_new(gensym("abs~"),
			  (t_newmethod)abs_new, 0,
			  sizeof(t_abs), 0, 0);
    sic_setup(abs_class, abs_dsp, SIC_FLOATTOSIGNAL);
}
