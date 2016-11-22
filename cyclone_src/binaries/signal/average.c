/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKME no reset after changing of a window size? */
/* CHECKME overlap */

/* Derek Kwan 2016
 
   rewrite perform method and average_bipolarsum, average_absolutesum, and average_rmssum
   to parsing of input signal outside of average_ methods and inside perform method

   getting rid of x_clock

   changing float outlet to signal outlet

   reinterpreting phase and resetting of phase

*/

#include <math.h>
#include "m_pd.h"

#define AVERAGE_DEFNPOINTS  100  /* CHECKME */
#define AVERAGE_DEFMODE     AVERAGE_BIPOLAR
enum { AVERAGE_BIPOLAR, AVERAGE_ABSOLUTE, AVERAGE_RMS };

typedef struct _average
{
    t_object    x_obj;
    t_inlet    *x_inlet1;
    int         x_mode;
    float     (*x_sumfn)(t_float, float);
    int         x_phase;
    int         x_npoints;
    float       x_result;
    float       x_accum;
} t_average;

static t_class *average_class;


static float average_bipolarsum(t_float input, float accum)
{
    accum += input;
    return (accum);
}

static float average_absolutesum(t_float input, float accum)
{
    accum += (input >= 0 ? input : -input);
    return (accum);
}

static float average_rmssum(t_float input, float accum)
{
    accum += input * input;
    return (accum);
}

static void average_setmode(t_average *x, int mode)
{
    if (mode == AVERAGE_BIPOLAR)
	x->x_sumfn = average_bipolarsum;
    else if (mode == AVERAGE_ABSOLUTE)
	x->x_sumfn = average_absolutesum;
    else if (mode == AVERAGE_RMS)
	x->x_sumfn = average_rmssum;
    x->x_mode = mode;
    x->x_phase = 0;
    x->x_accum = 0;
}

static void average_float(t_average *x, t_float f)
{
    int i = (int)f;  /* CHECKME noninteger */
    if (i > 0)  /* CHECKME */
    {
	x->x_npoints = i;
	x->x_phase = 0;
	x->x_accum = 0;
    }
}

static void average_bipolar(t_average *x)
{
    average_setmode(x, AVERAGE_BIPOLAR);
}

static void average_absolute(t_average *x)
{
    average_setmode(x, AVERAGE_ABSOLUTE);
}

static void average_rms(t_average *x)
{
    average_setmode(x, AVERAGE_RMS);
}

static t_int *average_perform(t_int *w)
{
    t_average *x = (t_average *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    float (*sumfn)(t_float, float) = x->x_sumfn;
    int phase = x->x_phase;
    int i;

    t_float input;

    for(i=0; i< nblock; i++){
        input = in[i];

        x->x_accum = (*sumfn)(input, x->x_accum);
        phase++;
        if(phase >= x->x_npoints){
            if (x->x_mode == AVERAGE_RMS)
	        /* CHECKME scaling and FIXME */
	        x->x_result = sqrtf(x->x_accum / (t_float)x->x_npoints);
	    else
	        x->x_result = x->x_accum / (t_float)x->x_npoints;
            phase = 0;
            x->x_accum = 0;

        };
        x->x_phase = phase;
        out[i] = x->x_result;
    };
    return (w + 5);
}

static void average_dsp(t_average *x, t_signal **sp)
{
    dsp_add(average_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void average_free(t_average *x)
{
}

static void *average_new(t_symbol *s, t_floatarg f)
{
    t_average *x = (t_average *)pd_new(average_class);
    int i = (int)f;  /* CHECKME noninteger */
    int mode;
    /* CHECKED it looks like memory is allocated for the entire window,
       in tune with the refman's note about ``maximum averaging interval'' --
       needed for dynamic control over window size, or what? LATER rethink */
    x->x_npoints = (i > 0 ?  /* CHECKME */
		    i : AVERAGE_DEFNPOINTS);
    if (s == gensym("bipolar"))
	mode = AVERAGE_BIPOLAR;
    else if (s == gensym("absolute"))
	mode = AVERAGE_ABSOLUTE;
    else if (s == gensym("rms"))
	mode = AVERAGE_RMS;
    else
    {
	mode = AVERAGE_DEFMODE;
	/* CHECKME a warning if (s && s != &s_) */
    }
    average_setmode(x, mode);
    x->x_result = 0;
    /* CHECKME if not x->x_phase = 0 */
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void average_tilde_setup(void)
{
    average_class = class_new(gensym("average~"), (t_newmethod)average_new,
            (t_method)average_free, sizeof(t_average), 0, A_DEFFLOAT, A_DEFSYM, 0);
    class_addmethod(average_class, (t_method) average_dsp, gensym("dsp"), 0);
    class_addmethod(average_class, nullfn, gensym("signal"), 0);
    class_addmethod(average_class, (t_method)average_bipolar, gensym("bipolar"), 0);
    class_addmethod(average_class, (t_method)average_absolute, gensym("absolute"), 0);
    class_addmethod(average_class, (t_method)average_rms, gensym("rms"), 0);
    class_addfloat(average_class, (t_method)average_float);
}
