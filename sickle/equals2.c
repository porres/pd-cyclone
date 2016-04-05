/* Copyright (c) 2016 Porres. check "LICENSE.txt" in this distribution For
 * information on usage and redistribution, plus for a DISCLAIMER OF ALL WARRANTIES */


/* version to be renamed to ==~ after compiled */

#include "m_pd.h"
#include "sickle/sic.h"

#define EQUALS_DEFRHS  0.

typedef t_sic t_equals;

static t_class *equals_class;
static t_int *equals_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
    *out++ = (f1 == f2);
    }
    return (w + 5);
}

static void equals_dsp(t_equals *x, t_signal **sp)
{
    dsp_add(equals_perform, 4, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *equals_new(t_symbol *s, int ac, t_atom *av)
{
    t_equals *x = (t_equals *)pd_new(equals_class);
    sic_inlet((t_sic *)x, 1, EQUALS_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void setup_0x3d0x3d0x7e(void)
{
    equals_class = class_new(gensym("==~"),
			      (t_newmethod)equals_new, 0,
			      sizeof(t_equals), 0, A_GIMME, 0);
    sic_setup(equals_class, equals_dsp, SIC_FLOATTOSIGNAL);
}

void setup(void){
    setup_0x3d0x3d0x7e();
}
