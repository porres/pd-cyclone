/************
 Copyright (c) 2016 Marco Matteo Markidis
 mm.markidis@gmail.com

 Made while listening:
 Hiatus Kaiyote -- Choose Your Weapon
*************/

#include "m_pd.h"
#include <math.h>

#define ARG (4) /* number of default args */

enum mode {
  LIN,
  EXP,
  CLAS
};

static t_class *scale_class;

typedef struct _scale
{
  t_object obj; /* object header */
  t_float f; /* incoming value */
  t_outlet *float_outlet;
  t_float minin;
  t_float maxin;
  t_float minout;
  t_float maxout;
  t_float expo;
  t_atom *output_list; /* for list output */
  t_int a_bytes;
  t_int flag;
} t_scale;

void *scale_new(t_symbol *s, int argc, t_atom *argv);
void scale_ft(t_scale *x, t_floatarg f);
void scale_bang(t_scale *x);
void scale_list(t_scale *x, t_symbol *s, int argc, t_atom *argv);
void scale_free(t_scale *x);
void scale_classic(t_scale *x,t_floatarg f);
void scale_setexpo(t_scale *x, t_floatarg f);
void scale_set(t_scale *x, t_floatarg f);

t_float scaling(t_scale *x, t_float f);
t_float exp_scaling(t_scale *x, t_float f);
t_float clas_scaling(t_scale *x, t_float f);
t_float (*ptrtoscaling)(t_scale *x,t_float f);
void check(t_scale *x);

void *scale_new(t_symbol *s, int argc, t_atom *argv)
{
  t_scale *x = (t_scale *)pd_new(scale_class);
  x->float_outlet = outlet_new(&x->obj, 0);
  floatinlet_new(&x->obj,&x->minin);
  floatinlet_new(&x->obj,&x->maxin);
  floatinlet_new(&x->obj,&x->minout);
  floatinlet_new(&x->obj,&x->maxout);
  inlet_new(&x->obj,&x->obj.ob_pd,gensym("float"),gensym("setexpo")); 
  if(argc<ARG || argc>ARG+1) {
    pd_error(x,"scale: %d arguments needed!\nscale is inizialized with default args.",ARG);
    x->minin = 0;
    x->maxin = 127;
    x->minout = 0;
    x->maxout = 1;
    return (x);
  }

  x->flag = LIN;
  x->expo = -1.f;
  
  x->minin = atom_getfloatarg(0,argc,argv); /* inserire controlli */
  x->maxin = atom_getfloatarg(1,argc,argv);
  x->minout = atom_getfloatarg(2,argc,argv);
  x->maxout = atom_getfloatarg(3,argc,argv);

  if(argc==ARG+1) {
    x->expo = atom_getfloatarg(4,argc,argv);
    x->flag = EXP;
  }

  x->a_bytes = sizeof(t_atom);
  x->output_list = (t_atom *)getbytes(x->a_bytes);
  if(x->output_list==NULL) {
    pd_error(x,"scale: memory allocation failure");
    return NULL;
  }
  
  return (x);
}

void scale_setup(void)
{
  t_class *c;
  scale_class = class_new(gensym("scale"), (t_newmethod)scale_new,
			  (t_method)scale_free,sizeof(t_scale),0,A_GIMME,0);
  c = scale_class;
  class_addfloat(c,(t_method)scale_ft);
  class_addbang(c,(t_method)scale_bang);
  class_addlist(c,(t_method)scale_list);
  class_addmethod(c,(t_method)scale_classic,gensym("classic"),A_DEFFLOAT,0);
  class_addmethod(c,(t_method)scale_setexpo,gensym("exp"),A_DEFFLOAT,0);
  class_addmethod(c,(t_method)scale_set,gensym("set"),A_DEFFLOAT,0);
}

void scale_ft(t_scale *x, t_floatarg f)
{
  check(x);
  outlet_float(x->float_outlet,ptrtoscaling(x,f));
  return;
}

void scale_classic(t_scale *x,t_floatarg f)
{
  if(x->expo==-1.f) {
    pd_error(x,"5th argument is not defined");
    return;
  }
  if(f==1)
    x->flag = CLAS;
  else x->flag = EXP;
  return;
}

t_float scaling(t_scale *x, t_float f)
{
  f = f / (x->maxin - x->minin) * (x->maxout-x->minout) + x->minout;
  return f;
}

t_float exp_scaling(t_scale *x, t_float f)
{
  f = ((f-x->minin)/(x->maxin-x->minin) == 0) 
    ? x->minout : (((f-x->minin)/(x->maxin-x->minin)) > 0) 
    ? (x->minout + (x->maxout-x->minout) * pow(x->expo,(f-x->minin)/(x->maxin-x->minin))) 
    : ( x->minout + (x->maxout-x->minout) * -(pow(x->expo,((-f+x->minin)/(x->maxin-x->minin)))));
    return f;
}

t_float clas_scaling(t_scale *x, t_float f)
{
  f = (x->maxout-x->minout >= 0) ?
    (x->minout + (x->maxout-x->minout) * ( (x->maxout - x->minout) * exp(-1*(x->maxin-x->minin)*log(x->expo)) * exp(f*log(x->expo)) )) :
    (-1) * ( x->minout + (x->maxout-x->minout) * ( (x->maxout - x->minout) * exp(-1*(x->maxin-x->minin)*log(x->expo)) * exp(f*log(x->expo))));
  return f;
}

void scale_bang(t_scale *x)
{
  outlet_float(x->float_outlet,x->f);
  return;
}

void scale_list(t_scale *x, t_symbol *s, int argc, t_atom *argv)
{
  int i = 0;
  int old_a = x->a_bytes;
  x->a_bytes = argc*sizeof(t_atom);
  x->output_list = (t_atom *)t_resizebytes(x->output_list,old_a,x->a_bytes);
  check(x);
  for(i=0;i<argc;i++)
    SETFLOAT(x->output_list+i,ptrtoscaling(x,atom_getfloatarg(i,argc,argv)));
  outlet_list(x->float_outlet,0,argc,x->output_list);
  return;
}

void scale_setexpo(t_scale *x, t_floatarg f)
{
  if(f<=1) {
    pd_error(x,"Exp must be > 1");
    x->expo = 1.06;
  }
  else x->expo = f;
  return;
}

void scale_set(t_scale *x, t_floatarg f)
{
  check(x);
  x->f = ptrtoscaling(x,f);
  return;
}

void check(t_scale *x)
{
  switch (x->flag) {
  case LIN:
    ptrtoscaling = scaling;
    break;
  case EXP:
    ptrtoscaling = exp_scaling;
    break;
  default:
    ptrtoscaling = clas_scaling;
    break;
  }
  return;
}

void scale_free(t_scale *x)
{
  t_freebytes(x->output_list,x->a_bytes);
}
