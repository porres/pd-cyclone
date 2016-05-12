/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

// fixed and rewritten by Porres

#include "m_pd.h"
// #include "math.h" // get it back?
// #include "shared.h"// get it back?
#include "sickle/sic.h"

#define TRAIN_DEFPERIOD  1000
#define TRAIN_DEFWIDTH   0.5
#define TRAIN_DEFOFFSET  0

typedef struct _train
{
    t_sic      x_sic;
    float      x_phase;
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
    float phase = x->x_phase;
    
   while (nblock--)
   {
        float next_phase;
		float period = *in1++;
        float phase_step = rcpksr / period;
        if (period <= 0) phase_step = 0;
        if (phase_step > 0.5) phase_step = 0.5; // smallest period corresponds to nyquist
        float width = *in2++;
        if (width < 0.) width = 0.;
        if (width > 1.) width = 1.;
        float phase_offset = *in3++;
        if (phase_offset < 0.) phase_offset = 0.;
        if (phase_offset > 1.) phase_offset = 1.;
        float wrap_low = phase_offset;
        float wrap_high = phase_offset + 1;
        float wrapped_phase;
       {
           wrapped_phase = (phase >= wrap_high) ? (wrap_low + phase - wrap_high) : phase;
           next_phase = wrapped_phase + phase_step;
           
           if (phase_step = 0) *out++ = 0;
           else if (phase >= wrap_high)
           {
               *out++ = 1.;
               clock_delay(x->x_clock, 0);
           }
           else if (phase < wrap_low) *out++ = 0.;
           else if(next_phase >= wrap_high) *out++ = 0.;
           else  *out++ = phase < (width + phase_offset) ? 1. : 0.;
        }
		phase = next_phase;
        }
    x->x_phase = phase;
    return (w + 7);
}

static void train_dsp(t_train *x, t_signal **sp)
{
    x->x_rcpksr = 1000. / sp[0]->s_sr; // reciprocal sample rate in Khz
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
