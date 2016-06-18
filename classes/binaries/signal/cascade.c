#include "m_pd.h"

static t_class *cascade_class;

typedef struct _cascade {
    t_object  x_obj;
    t_inlet  *x_inlet;
    t_outlet *x_outlet;
    t_float   x_a0;
    t_float   x_a1;
    t_float   x_a2;
    t_float   x_b1;
    t_float   x_b2;
    t_float   x_xnm1;
    t_float   x_xnm2;
    t_float   x_ynm1;
    t_float   x_ynm2;
    t_int     x_bypass;
    t_int     x_zero;
} t_cascade;

void *cascade_new(void);
static t_int * cascade_perform(t_int *w);
static void cascade_dsp(t_cascade *x, t_signal **sp);

void cascade_clear(t_cascade *x)
{
    x->x_xnm1 = x->x_xnm2 = x->x_ynm1 = x->x_ynm2 = 0.;
}

void cascade_bypass(t_cascade *x, t_floatarg f)
{
 x->x_bypass = (int)f;
}

void cascade_zero(t_cascade *x, t_floatarg f)
{
 x->x_zero = (int)f;
}

static void cascade_coef_list(t_cascade *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_a0 = atom_getfloatarg(0, argc, argv);
    x->x_a1 = atom_getfloatarg(1, argc, argv);
    x->x_a2 = atom_getfloatarg(2, argc, argv);
    x->x_b1 = atom_getfloatarg(3, argc, argv);
    x->x_b2 = atom_getfloatarg(4, argc, argv);
}


static void cascade_list(t_cascade *x, t_symbol *s, int argc, t_atom *argv)
{
    pd_error(x, "cascade~: no method for list");
}


static t_int * cascade_perform(t_int *w)
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
    t_int zero = x->x_zero;
    t_int bypass = x->x_bypass;
  while(nblock--)
  {
      if (bypass != 0) *out++ = *in++;
      else if (zero != 0) *out++ = 0;
      else
      {
      float yn, xn = *in++;
      *out++ = yn = a0*xn + a1*xnm1 + a2*xnm2 -b1*ynm1 -b2*ynm2;
      xnm2 = xnm1;
      xnm1 = xn;
      ynm2 = ynm1;
      ynm1 = yn;
      }
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

void *cascade_new(void)
{
  t_cascade *x = (t_cascade *)pd_new(cascade_class);
  inlet_new((t_object *)x, (t_pd *)x, &s_list, gensym("cascade_coef_list"));
  x->x_outlet = outlet_new(&x->x_obj, &s_signal);
  x->x_xnm1 = x->x_xnm2 = x->x_ynm1 = x->x_ynm2 = 0.;
  x->x_zero = x->x_bypass = 0;
  return (void *)x;
}

void cascade_tilde_setup(void)
{
    cascade_class = class_new(gensym("cascade~"), (t_newmethod) cascade_new,
            0, sizeof (t_cascade), CLASS_DEFAULT, 0);
    class_addmethod(cascade_class, nullfn, gensym("signal"), 0);
    class_addmethod(cascade_class, (t_method) cascade_dsp, gensym("dsp"), 0);
    class_addmethod(cascade_class, (t_method) cascade_clear, gensym("clear"), 0);
    class_addmethod(cascade_class, (t_method) cascade_bypass, gensym("bypass"), A_DEFFLOAT, 0);
    class_addmethod(cascade_class, (t_method) cascade_zero, gensym("zero"), A_DEFFLOAT, 0);
    class_addmethod(cascade_class, (t_method) cascade_coef_list,
                    gensym("cascade_coef_list"), A_GIMME, 0);
    class_addlist(cascade_class, cascade_list);
}
