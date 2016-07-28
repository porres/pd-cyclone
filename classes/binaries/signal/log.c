/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"

#define logf  log
#endif

#define LOG_MININPUT  1e-10  /* CHECKED */

typedef struct _log
{
    t_object x_obj;
    t_inlet  *x_inlet;
    t_float  x_rcplogbase;  /* LATER consider using double (and log()) */
} t_log;

static t_class *log_class;

static t_int *log_perform(t_int *w)
{
    t_log *x = (t_log *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float rcplogbase = x->x_rcplogbase;
    while (nblock--)
    {
    float b = *in2++;
    rcplogbase = b == 0. ? 1. : (b == 1. ? 0. : 1. / logf(b));
    if (rcplogbase != 0.)
        {
	    float in = *in1++;
	    if (in < LOG_MININPUT) in = LOG_MININPUT;  // CHECKED
	    *out++ = logf(in) * rcplogbase;
        }
    else *out++ = 0.;
    }
    return (w + 6);
}

static void log_dsp(t_log *x, t_signal **sp)
{
    dsp_add(log_perform, 5, x, sp[0]->s_n,
            sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

// FREE
static void *log_free(t_log *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *log_new(t_floatarg f)
{
    t_log *x = (t_log *)pd_new(log_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void log_tilde_setup(void)
{
    log_class = class_new(gensym("cyclone/log~"),
			  (t_newmethod)log_new, (t_method)log_free,
			  sizeof(t_log), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(log_class, nullfn, gensym("signal"), 0);
    class_addmethod(log_class, (t_method)log_dsp, gensym("dsp"), A_CANT, 0);
}