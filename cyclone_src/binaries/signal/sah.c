/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */


#include "m_pd.h"
/*MAGIC
include forky.h for forky_hasfeeders*/
#include "unstable/forky.h"
/*end magic*/

typedef struct _sah
{
    t_object x_obj;
    t_float  x_threshold;
    t_float  x_lastin;
    t_float  x_lastout;
    /*MAGIC
    *x_glist is a list of objects in the canvas
    *x_signalscalar is a pointer to the right inlet's float field, which we're going to poll
    x_badfloat is its value from the last dsp tick
    x_hasfeeders is a flag telling us whether right inlet has feeders*/
    t_glist *x_glist;
    t_float *x_signalscalar;
    t_float x_badfloat;
    int x_hasfeeders;
    /*end magic*/
} t_sah;

static t_class *sah_class;

/*MAGIC
This is a public function that returns float fields. It is not declared in m_pd.h,
so we have to declare it here. The arguments are the object and the inlet number
(indexed from zero)*/
EXTERN t_float *obj_findsignalscalar(t_object *x, int m);
/*end magic*/

static t_int *sah_perform(t_int *w)
{

    t_sah *x = (t_sah *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float threshold = x->x_threshold;
    t_float lastin = x->x_lastin;
    t_float lastout = x->x_lastout;
    
    /*MAGIC
    here we poll the float in the right inlet's float field to see if it's changed.
    if so, that means there's been a float input and we issue an error. Unfortunately,
    it won't work if you just input the same float over and over...
    */
    t_float scalar = *x->x_signalscalar;
	if (scalar != x->x_badfloat)
	{
		x->x_badfloat = scalar;
		pd_error(x, "inlet: expected 'signal' but got 'float'");	
	}
	/*end magic*/
    while (nblock--)
    {
    	float f;
    	/*MAGIC
    	self explanatory*/
    	if (x->x_hasfeeders) f = *in2++;
    	else f = 0.0;
    	/*end magic*/
	if (lastin <= threshold && f > threshold)  /* CHECKME <=, > */
	    lastout = *in1;
	in1++;
	lastin = f;
	*out++ = lastout;
    }
    x->x_lastin = lastin;
    x->x_lastout = lastout;
    return (w + 6);
}

static void sah_dsp(t_sah *x, t_signal **sp)
{
	/*MAGIC
	Get flag for signal feeders.*/
	x->x_hasfeeders = forky_hasfeeders((t_object *)x, x->x_glist, 1, &s_signal);
	/*end magic*/
    dsp_add(sah_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void sah_float(t_sah *x, t_float f)
{
    x->x_threshold = f;
}

static void *sah_new(t_floatarg f)
{
    t_sah *x = (t_sah *)pd_new(sah_class);
    x->x_threshold = f;
    x->x_lastin = 0;
    x->x_lastout = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    /*MAGIC
    1. get the current glist
    2. get a pointer to inlet 1's float field. */
    x->x_glist = canvas_getcurrent();
    x->x_signalscalar = obj_findsignalscalar((t_object *)x, 1);
    /*end magic*/
    return (x);
}

void sah_tilde_setup(void)
{
    sah_class = class_new(gensym("sah~"), (t_newmethod)sah_new, 0,
                        sizeof(t_sah), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(sah_class, nullfn, gensym("signal"), 0);
    class_addmethod(sah_class, (t_method)sah_dsp, gensym("dsp"), A_CANT, 0);
    class_addfloat(sah_class, (t_method)sah_float);
}