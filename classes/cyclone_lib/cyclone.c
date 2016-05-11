/* This is the cyclone library containing 12 non alphanumeric objects
 Originally, the externals in cyclone used to come in a library called
 "cyclone", which included these 12 objects plus the "hammer" and "sicle"
 libraries (control/MAX and signal/MSP objects respectively). In (and since)
 the Pd-Extended days, the objects used to come as single binaries and these
 12 objects were lost to oblivion, as they need to come in a library because
 some file systems don't like and agree with these characters. 
 
 So, the original cyclone library is now restored, but only containing these
 12 objects, which are: [!-], [!/], [==~], [!=~], [<~], [<=~], [>~], [>=~], 
 [!-~], [!/~], [%~], [+=~] (the original code for such objects are in the 
 nettles.c file).
 
 Alternatively, alphanumeric versions of these objects are included as sigle 
 binaries in the cyclone package */

#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

#if defined(_WIN32) || defined(__APPLE__)
/* cf pd/src/x_arithmetic.c */
#define fmodf  fmod
#endif

/* think about float-to-int conversion -- there is no point in making
   the two below compatible, while all the others are not compatible... */

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

typedef struct _equals
{
    t_sic  x_sic;
    int    x_algo;
} t_equals;

static t_class *equals_class;

static t_int *equals_perform0(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
#ifdef CYCLONE_SAFE
    int32 truebits;
    fi.fi_f = 1.;
    truebits = fi.fi_i;
#endif
    while (nblock--)
    {
#ifdef CYCLONE_SAFE
	fi.fi_i = ~((*in1++ == *in2++) - 1) & truebits;
#else
	fi.fi_i = ~((*in1++ == *in2++) - 1) & SHARED_TRUEBITS;
#endif
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static t_int *equals_perform1(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) *out++ = (*in1++ == *in2++);
    return (w + 5);
}

static t_int *equals_perform2(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    for (; nblock; nblock -= 8, in1 += 8, in2 += 8, out += 8)
    {
	float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
	float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];
	float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
	float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];
	out[0] = f0 == g0; out[1] = f1 == g1;
	out[2] = f2 == g2; out[3] = f3 == g3;
	out[4] = f4 == g4; out[5] = f5 == g5;
	out[6] = f6 == g6; out[7] = f7 == g7;
    }
    return (w + 5);
}

static void equals_dsp(t_equals *x, t_signal **sp)
{
    switch (x->x_algo)
    {
    case 1:
	dsp_add(equals_perform1, 4, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
	break;
    case 2:
	dsp_add(equals_perform2, 4, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
	break;
    default:
	dsp_add(equals_perform0, 4, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
    }
}

static void equals__algo(t_equals *x, t_floatarg f)
{
    x->x_algo = f;
}

static void *equals_new(t_symbol *s, int ac, t_atom *av)
{
    t_equals *x = (t_equals *)pd_new(equals_class);
    if (s == gensym("_==1~"))
	x->x_algo = 1;
    else if (s == gensym("_==2~"))
	x->x_algo = 2;
    else
	x->x_algo = 0;
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_notequals;
static t_class *notequals_class;

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

static void *notequals_new(t_symbol *s, int ac, t_atom *av)
{
    t_notequals *x = (t_notequals *)pd_new(notequals_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_lessthan;
static t_class *lessthan_class;

static t_int *lessthan_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ < *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void lessthan_dsp(t_lessthan *x, t_signal **sp)
{
    dsp_add(lessthan_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *lessthan_new(t_symbol *s, int ac, t_atom *av)
{
    t_lessthan *x = (t_lessthan *)pd_new(lessthan_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_greaterthan;
static t_class *greaterthan_class;

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

static void *greaterthan_new(t_symbol *s, int ac, t_atom *av)
{
    t_greaterthan *x = (t_greaterthan *)pd_new(greaterthan_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_lessthaneq;
static t_class *lessthaneq_class;

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

static void *lessthaneq_new(t_symbol *s, int ac, t_atom *av)
{
    t_lessthaneq *x = (t_lessthaneq *)pd_new(lessthaneq_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_greaterthaneq;
static t_class *greaterthaneq_class;

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

static void *greaterthaneq_new(t_symbol *s, int ac, t_atom *av)
{
    t_greaterthaneq *x = (t_greaterthaneq *)pd_new(greaterthaneq_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_rminus_tilde;
static t_class *rminus_tilde_class;

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

static void *rminus_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_rminus_tilde *x = (t_rminus_tilde *)pd_new(rminus_tilde_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_rdiv_tilde;
static t_class *rdiv_tilde_class;

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

static void *rdiv_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_rdiv_tilde *x = (t_rdiv_tilde *)pd_new(rdiv_tilde_class);
    /* CHECKED default 0 (refman's error), LATER rethink */
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_modulo;
static t_class *modulo_class;

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

static void *modulo_new(t_symbol *s, int ac, t_atom *av)
{
    t_modulo *x = (t_modulo *)pd_new(modulo_class);
    /* CHECKED default 0 (refman's error), LATER rethink */
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef struct _plusequals
{
    t_sic    x_sic;
    t_float  x_sum;
} t_plusequals;

static t_class *plusequals_class;

static t_int *plusequals_perform(t_int *w)
{
    t_plusequals *x = (t_plusequals *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float sum = x->x_sum;
    while (nblock--)
    {
        float x1 = *in1++;
        float x2 = *in2++;
        sum = sum * (x2 == 0);
        *out++ = sum += x1;
    }
    x->x_sum = sum;
    return (w + 6);
}

static void plusequals_dsp(t_plusequals *x, t_signal **sp)
{
    dsp_add(plusequals_perform, 5, x, sp[0]->s_n,
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

static void *plusequals_new(t_floatarg f)
{
    t_plusequals *x = (t_plusequals *)pd_new(plusequals_class);
    x->x_sum = f;
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void cyclone_setup(void)
{
    rminus_class = class_new(gensym("!-"),
			     (t_newmethod)rminus_new, 0,
			     sizeof(t_rev_op), 0, A_DEFFLOAT, 0);
    class_addbang(rminus_class, rminus_bang);
    class_addfloat(rminus_class, rminus_float);
    class_sethelpsymbol(rminus_class, gensym("rminus"));
    
    rdiv_class = class_new(gensym("!/"),
			   (t_newmethod)rdiv_new, 0,
			   sizeof(t_rev_op), 0, A_DEFFLOAT, 0);
    class_addbang(rdiv_class, rdiv_bang);
    class_addfloat(rdiv_class, rdiv_float);
    class_sethelpsymbol(rdiv_class, gensym("rdiv"));
    
    equals_class = class_new(gensym("==~"),
			    (t_newmethod)equals_new, 0,
                sizeof(t_equals), 0, A_GIMME, 0);
    sic_setup(equals_class, equals_dsp, SIC_FLOATTOSIGNAL);
/*    class_addcreator((t_newmethod)equals_new,
		     gensym("_==1~"), A_GIMME, 0);
    class_addcreator((t_newmethod)equals_new,
		     gensym("_==2~"), A_GIMME, 0);
    class_addmethod(equals_class, (t_method)equals__algo,
		    gensym("_algo"), A_FLOAT, 0); // for testing purposes / undocumented */
    class_sethelpsymbol(equals_class, gensym("equals~"));

    notequals_class = class_new(gensym("!=~"),
			     (t_newmethod)notequals_new, 0,
			     sizeof(t_notequals), 0, A_GIMME, 0);
    sic_setup(notequals_class, notequals_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(notequals_class, gensym("notequals~"));
    
    lessthan_class = class_new(gensym("<~"),
			    (t_newmethod)lessthan_new, 0,
			    sizeof(t_lessthan), 0, A_GIMME, 0);
    sic_setup(lessthan_class, lessthan_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(lessthan_class, gensym("lessthan~"));
    
    greaterthan_class = class_new(gensym(">~"),
			    (t_newmethod)greaterthan_new, 0,
			    sizeof(t_greaterthan), 0, A_GIMME, 0);
    sic_setup(greaterthan_class, greaterthan_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(greaterthan_class, gensym("greaterthan~"));
    
    lessthaneq_class = class_new(gensym("<=~"),
			     (t_newmethod)lessthaneq_new, 0,
			     sizeof(t_lessthaneq), 0, A_GIMME, 0);
    sic_setup(lessthaneq_class, lessthaneq_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(lessthaneq_class, gensym("lessthaneq~"));
    
    greaterthaneq_class = class_new(gensym(">=~"),
			     (t_newmethod)greaterthaneq_new, 0,
			     sizeof(t_greaterthaneq), 0, A_GIMME, 0);
    sic_setup(greaterthaneq_class, greaterthaneq_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(greaterthaneq_class, gensym("greaterthaneq~"));
    
    rminus_tilde_class = class_new(gensym("!-~"),
				(t_newmethod)rminus_tilde_new, 0,
				sizeof(t_rminus_tilde), 0, A_GIMME, 0);
    sic_setup(rminus_tilde_class, rminus_tilde_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(rminus_tilde_class, gensym("rminus~"));
    
    rdiv_tilde_class = class_new(gensym("!/~"),
			       (t_newmethod)rdiv_tilde_new, 0,
			       sizeof(t_rdiv_tilde), 0, A_GIMME, 0);
    sic_setup(rdiv_tilde_class, rdiv_tilde_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(rdiv_tilde_class, gensym("rdiv~"));
    
    modulo_class = class_new(gensym("%~"),
			     (t_newmethod)modulo_new, 0,
			     sizeof(t_modulo), 0, A_GIMME, 0);
    sic_setup(modulo_class, modulo_dsp, SIC_FLOATTOSIGNAL);
    class_sethelpsymbol(modulo_class, gensym("modulo~"));
    
    plusequals_class = class_new(gensym("+=~"),
			       (t_newmethod)plusequals_new, 0,
			       sizeof(t_plusequals), 0, A_DEFFLOAT, 0);
    sic_setup(plusequals_class, plusequals_dsp, SIC_FLOATTOSIGNAL);
    class_addbang(plusequals_class, plusequals_bang);
    class_addmethod(plusequals_class, (t_method)plusequals_set,
		    gensym("set"), A_FLOAT, 0);
    class_sethelpsymbol(plusequals_class, gensym("plusequals~"));
    //    logpost(NULL, 4, "this is cyclone lib %s, %dth %s build",
    //    containing non alphanumeric objects +=~, etc...
    //	 CYCLONE_VERSION, CYCLONE_BUILD, CYCLONE_RELEASE);
}
