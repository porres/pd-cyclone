/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Porres 2017:
 This has been fixed updated and renamed to 'cyclone_ops' by Porres;
 this source code used to be 'nettles.c', and the nettles objects used to be 
 part of the cyclone library;
*/

#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "magicbit.h"

/* think about float-to-int conversion -- there is no point in making
   the two below compatible, while all the others are not compatible... */

/* ------------------------------ [!-] AND [!/]  ------------------------------ */

typedef struct _rev_op
{
    t_object  x_ob;
    t_float   x_f1;
    t_float   x_f2;
} t_rev_op;

static t_class *rminus_class;

static void rminus_bang(t_rev_op *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_f2 - x->x_f1);
}

static void rminus_float(t_rev_op *x, t_float f)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_f2 - (x->x_f1 = f));
}

static void *rminus_new(t_floatarg f)
{
    t_rev_op *x = (t_rev_op *)pd_new(rminus_class);
    floatinlet_new((t_object *)x, &x->x_f2);
    outlet_new((t_object *)x, &s_float);
    x->x_f1 = 0;
    x->x_f2 = f;
    return (x);
}

static t_class *rdiv_class;

static void rdiv_bang(t_rev_op *x)
{
outlet_float(((t_object *)x)->ob_outlet, (x->x_f1 == 0. ? 0. : x->x_f2 / x->x_f1));
}

static void rdiv_float(t_rev_op *x, t_float f)
{
    x->x_f1 = f;
    rdiv_bang(x);
}

static void *rdiv_new(t_floatarg f)
{
    t_rev_op *x = (t_rev_op *)pd_new(rdiv_class);
    floatinlet_new((t_object *)x, &x->x_f2);
    outlet_new((t_object *)x, &s_float);
    x->x_f1 = 0;
    x->x_f2 = f;  /* CHECKED (refman's error) */
    return (x);
}

/* The implementation of signal relational operators below has been tuned
   somewhat, mostly in order to get rid of costly int->float conversions.
   Loops are not hand-unrolled, because these have proven to be slower
   in all the tests performed so far.  LATER find a good soul willing to
   make a serious profiling research... */

/* ------------------------------ [==~]  ------------------------------ */

typedef struct _equals
{
    t_object x_obj;
    t_inlet  *x_inlet;
    int    x_algo;
} t_equals;

static t_class *equals_class;

static t_int *equals_perform1(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) *out++ = (*in1++ == *in2++);
    return (w + 5);
}

static void equals_dsp(t_equals *x, t_signal **sp)
{
	dsp_add(equals_perform1, 4, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *equals_free(t_equals *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *equals_new(t_floatarg f)
{
    t_equals *x = (t_equals *)pd_new(equals_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

/* ------------------------------ [!=~]  ------------------------------ */

static t_class *notequals_class;

typedef struct _notequals
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_notequals;

static t_int *notequals_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ != *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void notequals_dsp(t_notequals *x, t_signal **sp)
{
    dsp_add(notequals_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *notequals_free(t_notequals *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *notequals_new(t_floatarg f)
{
    t_notequals *x = (t_notequals *)pd_new(notequals_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

/* ------------------------------ [<~]  ------------------------------ */

static t_class *lessthan_class;

typedef struct _lessthan
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_lessthan;


static t_int *lessthan_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ < *in2++) - 1) & SHARED_TRUEBITS; // change?
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void lessthan_dsp(t_lessthan *x, t_signal **sp)
{
    dsp_add(lessthan_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *lessthan_free(t_lessthan *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *lessthan_new(t_floatarg f)
{
    t_lessthan *x = (t_lessthan *)pd_new(lessthan_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

/* ------------------------------ [>~]  ------------------------------ */

static t_class *greaterthan_class;

typedef struct _greaterthan
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_greaterthan;

static t_int *greaterthan_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ > *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void greaterthan_dsp(t_greaterthan *x, t_signal **sp)
{
    dsp_add(greaterthan_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *greaterthan_free(t_greaterthan *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *greaterthan_new(t_floatarg f)
{
    t_greaterthan *x = (t_greaterthan *)pd_new(greaterthan_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    endpost();
    return (x);
}

/* ------------------------------ [<=~]  ------------------------------ */


static t_class *lessthaneq_class;

typedef struct _lessthaneq
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_lessthaneq;

static t_int *lessthaneq_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ <= *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void lessthaneq_dsp(t_lessthaneq *x, t_signal **sp)
{
    dsp_add(lessthaneq_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *lessthaneq_free(t_lessthaneq *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *lessthaneq_new(t_floatarg f)
{
    t_lessthaneq *x = (t_lessthaneq *)pd_new(lessthaneq_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

/* ------------------------------ [>=~]  ------------------------------ */

static t_class *greaterthaneq_class;

typedef struct _greaterthaneq
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_greaterthaneq;

static t_int *greaterthaneq_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ >= *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void greaterthaneq_dsp(t_greaterthaneq *x, t_signal **sp)
{
    dsp_add(greaterthaneq_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *greaterthaneq_free(t_greaterthaneq *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *greaterthaneq_new(t_floatarg f)
{
    t_greaterthaneq *x = (t_greaterthaneq *)pd_new(greaterthaneq_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

/* ------------------------------ [!-~]  ------------------------------ */

static t_class *rminus_tilde_class;

typedef struct _rminus_tilde
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_rminus_tilde;

static t_int *rminus_tilde_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) *out++ = *in2++ - *in1++;
    return (w + 5);
}

static void rminus_tilde_dsp(t_rminus_tilde *x, t_signal **sp)
{
    dsp_add(rminus_tilde_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *rminus_tilde_free(t_rminus_tilde *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *rminus_tilde_new(t_floatarg f)
{
    t_rminus_tilde *x = (t_rminus_tilde *)pd_new(rminus_tilde_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

/* ------------------------------ [!/~]  ------------------------------ */


static t_class *rdiv_tilde_class;

typedef struct _rdiv_tilde
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_rdiv_tilde;

static t_int *rdiv_tilde_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	/* CHECKED incompatible: c74 outputs NaNs.
	   The line below is consistent with Pd's /~, LATER rethink. */
	/* LATER multiply by reciprocal if in1 has no signal feeders */
	*out++ = (f1 == 0. ? 0. : *in2++ / f1);
    }
    return (w + 5);
}

static void rdiv_tilde_dsp(t_rdiv_tilde *x, t_signal **sp)
{
    dsp_add(rdiv_tilde_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *rdiv_tilde_new(t_floatarg f)
{
    t_rdiv_tilde *x = (t_rdiv_tilde *)pd_new(rdiv_tilde_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}


static void *rdiv_tilde_free(t_rdiv_tilde *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}


/* ------------------------------ [%~]  ------------------------------ */

static t_class *modulo_class;

typedef struct _modulo
{
    t_object x_obj;
    t_inlet  *x_inlet;
} t_modulo;

static t_int *modulo_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
	/* LATER think about using ieee-754 normalization tricks */
	*out++ = (f2 == 0. ? 0.  /* CHECKED */
		  : fmod(f1, f2));
    }
    return (w + 5);
}

static void modulo_dsp(t_modulo *x, t_signal **sp)
{
    dsp_add(modulo_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *modulo_free(t_modulo *x)
{
    inlet_free(x->x_inlet);
    return (void *)x;
}

static void *modulo_new(t_floatarg f)
{
    t_modulo *x = (t_modulo *)pd_new(modulo_class);
    x->x_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet, f);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

/* ------------------------------ [+=~]  ------------------------------ */


typedef struct _plusequals
{
    t_object x_obj;
    t_float  x_sum;
    t_inlet  *x_triglet;
    
    // Magic
    t_glist *x_glist;
    t_float *x_signalscalar;
    int x_hasfeeders;
    
} t_plusequals;

static t_class *plusequals_class;

// MAGIC
EXTERN t_float *obj_findsignalscalar(t_object *x, int m);

static t_int *plusequals_perform(t_int *w)
{
    t_plusequals *x = (t_plusequals *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float sum = x->x_sum;
    
    // MAGIC: poll float for error
    t_float scalar = *x->x_signalscalar;
    if (!magic_isnan(*x->x_signalscalar))
    {
        magic_setnan(x->x_signalscalar);
        pd_error(x, "plusequals~: doesn't understand 'float'");
    }
    
    while (nblock--)
    {
        float x1 = *in1++;
        float x2;
        // MAGIC
        if (x->x_hasfeeders) x2 = *in2++;
        else x2 = 0.0;
        
        sum = sum * (x2 == 0);
        *out++ = sum += x1;
    }
    x->x_sum = sum;
    return (w + 6);
}

static t_int *plusequals_perform_no_in(t_int *w)
{
    t_plusequals *x = (t_plusequals *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *out = (t_float *)(w[5]);
    
    // MAGIC: poll float for error
    if (!magic_isnan(*x->x_signalscalar))
    {
        magic_setnan(x->x_signalscalar);
        pd_error(x, "plusequals~: doesn't understand 'float'");
    }
    
    while (nblock--)
    {
        *out++ = 0;
    }
    return (w + 6);
}

static void plusequals_dsp(t_plusequals *x, t_signal **sp)
{
    // MAGIC
    x->x_hasfeeders = magic_inlet_connection((t_object *)x, x->x_glist, 1, &s_signal);
    magic_setnan(x->x_signalscalar);
    
    if (magic_inlet_connection((t_object *)x, x->x_glist, 0, &s_signal))
        dsp_add(plusequals_perform, 5, x, sp[0]->s_n,
                sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
    else
        dsp_add(plusequals_perform_no_in, 5, x, sp[0]->s_n,
                sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}


static void plusequals_bang(t_plusequals *x)
{
    x->x_sum = 0;
}

static void plusequals_set(t_plusequals *x, t_floatarg f)
{
    x->x_sum = f;
}

static void *plusequals_free(t_plusequals *x)
{
    inlet_free(x->x_triglet);
    return (void *)x;
}

static void *plusequals_new(t_floatarg f)
{
    t_plusequals *x = (t_plusequals *)pd_new(plusequals_class);
    x->x_sum = f;
    x->x_glist = canvas_getcurrent();
    x->x_triglet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    // MAGIC
    x->x_signalscalar = obj_findsignalscalar((t_object *)x, 1);
    
    return (x);
}

/* ----------------------------- SET UP ------------------------------ */

void cyclone_ops_setup(void)
{
    
    {
        post("------------------------------------------------------------------");
        post("Cyclone 0.3 beta-2; released: June 9th, 2017");
        post("------------------------------------------------------------------");
        post("Cyclops library, part of Cyclone");
        post("The Cyclops (Cyclone Operators) library contains the operators:");
        post("[!-], [!-~], [!/], [!/~], [!=~], [%%~], [+=~], [<=~], [<~], [==~], [>=~] and [>~]");
        post("------------------------------------------------------------------");
    }

/* -- [!-] -- */
    
    rminus_class = class_new(gensym("!-"),
			     (t_newmethod)rminus_new, 0,
			     sizeof(t_rev_op), 0, A_DEFFLOAT, 0);
    class_addbang(rminus_class, rminus_bang);
    class_addfloat(rminus_class, rminus_float);
    class_sethelpsymbol(rminus_class, gensym("rminus"));

/* -- [!/] -- */
    
    rdiv_class = class_new(gensym("!/"),
			   (t_newmethod)rdiv_new, 0,
			   sizeof(t_rev_op), 0, A_DEFFLOAT, 0);
    class_addbang(rdiv_class, rdiv_bang);
    class_addfloat(rdiv_class, rdiv_float);
    class_sethelpsymbol(rdiv_class, gensym("rdiv"));

/* -- [==~] -- */
    
    equals_class = class_new(gensym("==~"),
			    (t_newmethod)equals_new, (t_method)equals_free,
                sizeof(t_equals), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(equals_class, nullfn, gensym("signal"), 0);
    class_addmethod(equals_class, (t_method)equals_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(equals_class, gensym("equals~"));

/* -- [!=~] -- */
    
    notequals_class = class_new(gensym("!=~"), (t_newmethod)notequals_new,
        (t_method)notequals_free, sizeof(t_notequals), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(notequals_class, nullfn, gensym("signal"), 0);
    class_addmethod(notequals_class, (t_method)notequals_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(notequals_class, gensym("notequals~"));
    
/* -- [<~] -- */
    
    lessthan_class = class_new(gensym("<~"), (t_newmethod)lessthan_new,
        (t_method)lessthan_free, sizeof(t_lessthan), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(lessthan_class, nullfn, gensym("signal"), 0);
    class_addmethod(lessthan_class, (t_method)lessthan_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(lessthan_class, gensym("lessthan~"));

/* -- [>~] -- */
    
    greaterthan_class = class_new(gensym(">~"), (t_newmethod)greaterthan_new,
        (t_method)greaterthan_free, sizeof(t_greaterthan), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(greaterthan_class, nullfn, gensym("signal"), 0);
    class_addmethod(greaterthan_class, (t_method)greaterthan_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(greaterthan_class, gensym("greaterthan~"));

/* -- [<=~] -- */
    
    lessthaneq_class = class_new(gensym("<=~"), (t_newmethod)lessthaneq_new,
            (t_method)lessthaneq_free, sizeof(t_lessthaneq), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(lessthaneq_class, nullfn, gensym("signal"), 0);
    class_addmethod(lessthaneq_class, (t_method)lessthaneq_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(lessthaneq_class, gensym("lessthaneq~"));

/* -- [>=~] -- */
    
    greaterthaneq_class = class_new(gensym(">=~"), (t_newmethod)greaterthaneq_new,
        (t_method)greaterthaneq_free, sizeof(t_greaterthaneq), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(greaterthaneq_class, nullfn, gensym("signal"), 0);
    class_addmethod(greaterthaneq_class, (t_method)greaterthaneq_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(greaterthaneq_class, gensym("greaterthaneq~"));

/* -- [!-~] -- */
    
    rminus_tilde_class = class_new(gensym("!-~"), (t_newmethod)rminus_tilde_new,
            (t_method)rminus_tilde_free, sizeof(t_rminus_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(rminus_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(rminus_tilde_class, (t_method)rminus_tilde_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(rminus_tilde_class, gensym("rminus~"));
    
/* -- [!/~] -- */
    
    rdiv_tilde_class = class_new(gensym("!/~"), (t_newmethod)rdiv_tilde_new,
        (t_method)rdiv_tilde_free, sizeof(t_rdiv_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(rdiv_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(rdiv_tilde_class, (t_method)rdiv_tilde_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(rdiv_tilde_class, gensym("rdiv~"));

/* -- [%~] -- */
    
    modulo_class = class_new(gensym("%~"), (t_newmethod)modulo_new,
        (t_method)modulo_free, sizeof(t_modulo), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(modulo_class, nullfn, gensym("signal"), 0);
    class_addmethod(modulo_class, (t_method)modulo_dsp, gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(modulo_class, gensym("modulo~"));
    
/* -- [+=~] -- */
    
    plusequals_class = class_new(gensym("+=~"), (t_newmethod)plusequals_new, 0,
            sizeof(t_plusequals), 0, A_DEFFLOAT, 0);
    class_addmethod(plusequals_class, nullfn, gensym("signal"), 0);
    class_addmethod(plusequals_class, (t_method) plusequals_dsp, gensym("dsp"), 0);
    class_addbang(plusequals_class, plusequals_bang);
    class_addmethod(plusequals_class, (t_method)plusequals_set, gensym("set"), A_FLOAT, 0);
    class_sethelpsymbol(plusequals_class, gensym("plusequals~"));
    
}
