/* Copyright (c) 2016 Porres
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

#define THRESH_DEFLO  0.
#define THRESH_DEFHI  0.

/*typedef t_sic t_thresh;*/

typedef struct _thresh
{
    t_sic    x_sic;
    t_float  x_lastout;
} t_thresh;


static t_class *thresh_class;

static t_int *thresh_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *in3 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float lastout = x->x_lastout;
    while (nblock--)
    {
    	float in = *in1++;
    	float lo = *in2++;
    	float hi = *in3++;
        lastout = (in > lo && (in >= hi || lastout));
        in1++;
        *out++ = lastout;
    }
    x->x_lastout = lastout;
    return (w + 6);
}

static void thresh_dsp(t_thresh *x, t_signal **sp)
{
    dsp_add(thresh_perform, 5, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *thresh_new(t_symbol *s, int ac, t_atom *av)
{
    t_thresh *x = (t_thresh *)pd_new(thresh_class);
    x->x_lastout = 0;
    sic_inlet((t_sic *)x, 1, THRESH_DEFLO, 0, ac, av);
    sic_inlet((t_sic *)x, 2, THRESH_DEFHI, 1, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void thresh_tilde_setup(void)
{
    thresh_class = class_new(gensym("thresh~"),
			   (t_newmethod)thresh_new, 0,
			   sizeof(t_thresh), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)thresh_new, gensym("thresh~"), A_GIMME, 0);
    class_addcreator((t_newmethod)thresh_new, gensym("cyclone/thresh~"), A_GIMME, 0);
    sic_setup(thresh_class, thresh_dsp, SIC_FLOATTOSIGNAL);
}