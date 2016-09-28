// Porres 2016

#include "m_pd.h"

// MAGIC
#include "unstable/forky.h"

typedef struct _plusequals
{
    t_object x_obj;
    t_float  x_sum;
    t_inlet  *x_triglet;

// Magic
    t_glist *x_glist;
    t_float *x_signalscalar;
    t_float x_badfloat;
    int x_hasfeeders;;

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
    if (scalar != x->x_badfloat)
    {
        x->x_badfloat = scalar;
        pd_error(x, "inlet: expected 'signal' but got 'float'");
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
    while (nblock--)
    {
    *out++ = 0;
    }
    return (w + 6);
}

static void plusequals_dsp(t_plusequals *x, t_signal **sp)
{
    if (forky_hasfeeders((t_object *)x, x->x_glist, 0, &s_signal))
    {
// MAGIC
        x->x_hasfeeders = forky_hasfeeders((t_object *)x, x->x_glist, 1, &s_signal);

        dsp_add(plusequals_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
    }
    
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
    x->x_glist = canvas_getcurrent();
    x->x_signalscalar = obj_findsignalscalar((t_object *)x, 1);
    
    return (x);
}

void plusequals_tilde_setup(void)
{
    plusequals_class = class_new(gensym("plusequals~"), (t_newmethod)plusequals_new,
        0, sizeof(t_plusequals), CLASS_DEFAULT, A_DEFFLOAT, 0);
    class_addmethod(plusequals_class, nullfn, gensym("signal"), 0);
    class_addmethod(plusequals_class, (t_method) plusequals_dsp, gensym("dsp"), 0);
    class_addbang(plusequals_class, plusequals_bang);
    class_addmethod(plusequals_class, (t_method)plusequals_set,
                    gensym("set"), A_FLOAT, 0);
}