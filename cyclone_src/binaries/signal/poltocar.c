/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "unstable/forky.h"

typedef struct _poltocar
{
    t_object x_obj;
    t_inlet *poltocar;
    t_outlet  *x_out2;
    t_glist  *x_glist;
    t_float *x_signalscalar;
    t_int      x_hasfeeders;
} t_poltocar;

static t_class *poltocar_class;

EXTERN t_float *obj_findsignalscalar(t_object *x, int m);

static t_int *poltocar_perform(t_int *w)
{
    t_poltocar *x = (t_poltocar *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out1 = (t_float *)(w[5]);
    t_float *out2 = (t_float *)(w[6]);
// MAGIC: poll float for error
    if (!isnan(*x->x_signalscalar))
	{
		*x->x_signalscalar = NAN;
    //    pd_error(x, "poltocar~: doesn't understand 'float'");
    }
    while (nblock--)
    {
        float am = *in1++;
        float ph;
        // MAGIC
        if (x->x_hasfeeders) ph = *in2++;
        else ph = 0.0;
	*out1++ = am * cosf(ph);
	*out2++ = am * sinf(ph);  /* CHECKED */
    }
    return (w + 7);
}

static t_int *poltocar_perform_no_in(t_int *w)
{
    t_poltocar *x = (t_poltocar *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *out1 = (t_float *)(w[4]);
    t_float *out2 = (t_float *)(w[6]);
    if (!isnan(*x->x_signalscalar))
	{
		*x->x_signalscalar = NAN;
     //   pd_error(x, "poltocar~: doesn't understand 'float'");
    }
    while (nblock--)
    {
        *out1++ = *out2++ = 0;
    }
    return (w + 7);
}

static void poltocar_dsp(t_poltocar *x, t_signal **sp)
{
    x->x_hasfeeders = forky_hasfeeders((t_object *)x, x->x_glist, 1, &s_signal);
    if (forky_hasfeeders((t_object *)x, x->x_glist, 0, &s_signal))
	dsp_add(poltocar_perform, 6, x, sp[0]->s_n, sp[0]->s_vec,
		sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
    else dsp_add(poltocar_perform_no_in, 6, x, sp[0]->s_n, sp[0]->s_vec,
                 sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *poltocar_new(void)
{
    t_poltocar *x = (t_poltocar *)pd_new(poltocar_class);
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_glist = canvas_getcurrent();
    x->x_signalscalar = obj_findsignalscalar((t_object *)x, 1);
    *x->x_signalscalar = NAN;
    x->x_out2 = outlet_new((t_object *)x, &s_signal);
    return (x);
}

void poltocar_tilde_setup(void)
{
    poltocar_class = class_new(gensym("poltocar~"),
            (t_newmethod)poltocar_new, 0,
            sizeof(t_poltocar), 0, 0);
    class_addmethod(poltocar_class, nullfn, gensym("signal"), 0);
    class_addmethod(poltocar_class, (t_method) poltocar_dsp, gensym("dsp"), 0);
}
