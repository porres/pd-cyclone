// 64 bits???

#include "m_pd.h"

typedef struct _cascade
{
    t_object x_obj;
    t_inlet  x_a0;
    t_inlet  x_a1;
    t_inlet  x_a2;
    t_inlet  x_b1;
    t_inlet  x_b2;
    t_float  x_xnm1;
    t_float  x_xnm2;
    t_float  x_ynm1;
    t_float  x_ynm2;
} t_cascade;

static t_class *cascade_class;

static void cascade_list(t_cascade *x, t_symbol *s, int argc, t_atom *argv)
{
    pd_float((t_pd)x->x_a0, atom_getfloatarg(0, argc, argv));
    pd_float((t_pd)x->x_a1, atom_getfloatarg(1, argc, argv));
    pd_float((t_pd)x->x_a2, atom_getfloatarg(2, argc, argv));
    pd_float((t_pd)x->x_b1, atom_getfloatarg(3, argc, argv));
    pd_float((t_pd)x->x_b2, atom_getfloatarg(4, argc, argv));
}

void cascade_clear(t_cascade *x)
{
    x->x_xnm1 = x->x_xnm2 = x->x_ynm1 = x->x_ynm2 = 0.;
}

static t_int *cascade_perform(t_int *w)
{
    t_cascade *x = (t_cascade *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_float a0 = x->x_a0;
    t_float a1 = x->x_a1;
    t_float a2 = x->x_a2;
    t_float b1 = x->x_b1;
    t_float b2 = x->x_b2;
    t_float xnm1 = x->x_xnm1;
    t_float xnm2 = x->x_xnm2;
    t_float ynm1 = x->x_ynm1;
    t_float ynm2 = x->x_ynm2;
    while (nblock--)
    {
        float yn, xn = *in++;
        *out++ = yn = a0*xn + a1*xnm1 + a2*xnm2 -b1*ynm1 -b2*ynm2;
        xnm2 = xnm1;
        xnm1 = xn;
        ynm2 = ynm1;
        ynm1 = yn;
    }
    x->x_xnm1 = xnm1;
    x->x_xnm2 = xnm2;
    x->x_ynm1 = ynm1;
    x->x_ynm2 = ynm2;
    return (w + 5);
}

static void cascade_dsp(t_cascade *x, t_signal **sp)
{
    dsp_add(cascade_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *cascade_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cascade *x = (t_cascade *)pd_new(cascade_class);
    outlet_new((t_object *)x, &s_signal);
    x->x_xnm1 = x->x_xnm2 = x->x_ynm1 = x->x_ynm2 = 0.;
}

void cascade_tilde_setup(void)
{
    cascade_class = class_new(gensym("cyclone/cascade~"), (t_newmethod)cascade_new,
            (t_method)cascade_free, sizeof(t_cascade), 0, A_GIMME, 0);
    class_addmethod(cascade_class, nullfn, gensym("signal"), 0);
    class_addmethod(cascade_class, (t_method)cascade_dsp, gensym("dsp"), A_CANT, 0);
    class_addlist(cascade_class, cascade_list);
    class_addmethod(cascade_class, (t_method) cascade_clear, gensym("clear"), 0);
}