/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

#define TRAIN_DEFPERIOD  1000
#define TRAIN_DEFWIDTH      0.5
#define TRAIN_DEFOFFSET     0

typedef struct _train
{
    t_sic      x_sic;
    double     x_phase;
	 double		x_prev;
    float      x_rcpksr;
    t_outlet  *x_bangout;
    t_clock   *x_clock;
} t_train;

static t_class *train_class;

static void train_tick(t_train *x)
{
    outlet_bang(x->x_bangout);
}

static t_int *train_perform(t_int *w)
{
    t_train *x = (t_train *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    float rcpksr = x->x_rcpksr;
    double phase = x->x_phase;
	 double prev = x->x_prev;
    //double tfph = phase + SHARED_UNITBIT32;
    t_shared_wrappy wrappy;
    int32_t normhipart;
    unsigned int edge = 0;

    wrappy.w_d = SHARED_UNITBIT32;
    normhipart = wrappy.w_i[SHARED_HIOFFSET];

   while (nblock--)
   {
		double width, offset, offsetphase;
		double next = 0.;
		float period = *in1++;
	
		float in2val = *in2++;
		if (in2val < 0.) in2val = 0.;
		if (in2val >= 1.)
		{
			width = 1.;
			wrappy.w_d = offset + phase + (rcpksr / period) + SHARED_UNITBIT32;
			wrappy.w_i[SHARED_HIOFFSET] = normhipart;
			next = wrappy.w_d - SHARED_UNITBIT32;
		}
		else
		{
			wrappy.w_d = in2val + SHARED_UNITBIT32;
    		wrappy.w_i[SHARED_HIOFFSET] = normhipart;
			width = wrappy.w_d - SHARED_UNITBIT32;
		}

		//offset
		wrappy.w_d = *in3++ + SHARED_UNITBIT32;
    	wrappy.w_i[SHARED_HIOFFSET] = normhipart;
		offset = wrappy.w_d - SHARED_UNITBIT32;

		wrappy.w_d = offset + phase + SHARED_UNITBIT32;
    	wrappy.w_i[SHARED_HIOFFSET] = normhipart;
		offsetphase = wrappy.w_d - SHARED_UNITBIT32;

		//printf("width: %f, offsetphase: %f, prev: %f\n",width, offsetphase, prev);

		if (0 == width && offsetphase < prev) *out++ = 1.;
		else if (1 == width && offsetphase > next) *out++ = 0.; 
		else *out++ = (offsetphase <= width) ? 1. : 0.;

		if (offsetphase < prev)
		{
			clock_delay(x->x_clock, 0);
		}

		prev = offsetphase;
		if (period > rcpksr)  /* LATER rethink */
			phase += rcpksr / period;  /* LATER revisit (profiling?) */
    }
    x->x_phase = phase;
	 x->x_prev = prev;
    return (w + 7);
}

static void train_dsp(t_train *x, t_signal **sp)
{
    x->x_rcpksr = 1000. / sp[0]->s_sr;
    dsp_add(train_perform, 6, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void train_free(t_train *x)
{
    if (x->x_clock) clock_free(x->x_clock);
}

static void *train_new(t_symbol *s, int ac, t_atom *av)
{
    t_train *x = (t_train *)pd_new(train_class);
    x->x_phase = 0;
	 x->x_prev = 2.;
    sic_inlet((t_sic *)x, 0, TRAIN_DEFPERIOD, 0, ac, av);
    sic_inlet((t_sic *)x, 1, TRAIN_DEFWIDTH, 1, ac, av);
    sic_inlet((t_sic *)x, 2, TRAIN_DEFOFFSET, 2, ac, av);
    outlet_new((t_object *)x, &s_signal);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    x->x_clock = clock_new(x, (t_method)train_tick);
    return (x);
}

void train_tilde_setup(void)
{
    train_class = class_new(gensym("train~"),
			    (t_newmethod)train_new,
			    (t_method)train_free,
			    sizeof(t_train), 0, A_GIMME, 0);
    sic_setup(train_class, train_dsp, SIC_FLOATTOSIGNAL);
}
