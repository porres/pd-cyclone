/************
 Copyright (c) 2016 Marco Matteo Markidis
 mm.markidis@gmail.com
 
 For information on usage and redistribution, and for a DISCLAIMER OF ALL
 WARRANTIES, see the file, "LICENSE.txt," in this distribution.

 Made while listening:
 Hiatus Kaiyote -- Choose Your Weapon
*************/

#include <string.h>

#include "m_pd.h"
#include <math.h>

/* 0 modern mode 
   1 classic mode */

static t_class *scale_class;

typedef struct _scale
{
  t_object obj; /* object header */
  t_float in; /* stored input value */
  t_outlet *float_outlet;
  t_float minin;
  t_float maxin;
  t_float minout;
  t_float maxout;
  t_float expo;
  t_atom *output_list; /* for list output */
  t_int a_bytes;
  t_int flag;
  t_int ac;
} t_scale;

void *scale_new(t_symbol *s, int argc, t_atom *argv);
void scale_ft(t_scale *x, t_floatarg f);
void scale_bang(t_scale *x);
void scale_list(t_scale *x, t_symbol *s, int argc, t_atom *argv);
void scale_free(t_scale *x);
void scale_setexpo(t_scale *x, t_floatarg f);

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
  x->minin = 0;
  x->maxin = 127;
  x->minout = 0;
  x->maxout = 1;
  x->flag = 1;
  x->expo = 1.f;
  t_int numargs = 0;
  t_float tmp = -1.f;

  while(argc>0) {
    t_symbol *firstarg = atom_getsymbolarg(0,argc,argv);
    if(firstarg==&s_){
      switch(numargs) {
      case 0:
	x->minin = atom_getfloatarg(0,argc,argv);
	numargs++;
	argc--;
	argv++;
	break;
      case 1:
	x->maxin = atom_getfloatarg(0,argc,argv);
	numargs++;
	argc--;
	argv++;
	break;
      case 2:
	x->minout = atom_getfloatarg(0,argc,argv);
	numargs++;
	argc--;
	argv++;
	break;
      case 3:
	x->maxout = atom_getfloatarg(0,argc,argv);
	numargs++;
	argc--;
	argv++;
	break;
      case 4:
	tmp = atom_getfloatarg(0,argc,argv);
	numargs++;
	argc--;
	argv++;
	break;
      default:
	argc--;
	argv++;
      }
    }
    else {
      t_int isclassic = strcmp(firstarg->s_name,"@classic") == 0;
      if(isclassic && argc>=1) {
	t_symbol *arg = atom_getsymbolarg(1,argc,argv);
	if(arg == &s_) {
	  t_int dummy = atom_getintarg(1,argc,argv);
	  if(dummy==0)
	    x->flag = 0;
	  argc-=2;
	  argv+=2;
	}
      }
      else {
	argc = 0;
	pd_error(x,"scale: improper args");
      }
    }
  }


  if(tmp!=-1) {
    if(x->flag==0)
      x->expo = ((tmp<0.f) ? 0.f : tmp);
    else x->expo = ((tmp<1.f) ? 0.f : tmp);
  }
  
  x->ac = 1;
  x->a_bytes = x->ac*sizeof(t_atom);
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
  class_addmethod(c,(t_method)scale_setexpo,gensym("setexpo"),A_DEFFLOAT,0);
}

void scale_ft(t_scale *x, t_floatarg f)
{
  x->in = f;
  check(x);
  t_float temp = ptrtoscaling(x,f);
  SETFLOAT(x->output_list,temp);
  outlet_list(x->float_outlet,0,x->ac,x->output_list);
  return;
}

t_float scaling(t_scale *x, t_float f)
{
  f = (x->maxout - x->minout)*(f-x->minin)/(x->maxin-x->minin) + x->minout;
  return f;
}

t_float exp_scaling(t_scale *x, t_float f)
{
  f = ((f-x->minin)/(x->maxin-x->minin) == 0) 
    ? x->minout : (((f-x->minin)/(x->maxin-x->minin)) > 0) 
    ? (x->minout + (x->maxout-x->minout) * pow((f-x->minin)/(x->maxin-x->minin),x->expo)) 
    : ( x->minout + (x->maxout-x->minout) * -(pow(((-f+x->minin)/(x->maxin-x->minin)),x->expo)));
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
  scale_ft(x,x->in);
  return;
}

void scale_list(t_scale *x, t_symbol *s, int argc, t_atom *argv)
{
  int i = 0;
  int old_a = x->a_bytes;
  x->ac = argc;
  x->a_bytes = argc*sizeof(t_atom);
  x->output_list = (t_atom *)t_resizebytes(x->output_list,old_a,x->a_bytes);
  check(x);
  x->in = atom_getfloatarg(0,argc,argv);
  for(i=0;i<argc;i++)
    SETFLOAT(x->output_list+i,ptrtoscaling(x,atom_getfloatarg(i,argc,argv)));
  outlet_list(x->float_outlet,0,argc,x->output_list);
  return;
}

void scale_setexpo(t_scale *x, t_floatarg f)
{
  if(x->flag==1)
    x->expo = ((f<1.)?1.:f);
  else x->expo = ((f<0.)?0.:f);
  return;
}

void check(t_scale *x)
{
  switch (x->flag) {
  case 0:
    ptrtoscaling = exp_scaling;
    break;
  default:
    ptrtoscaling = clas_scaling;
    break;
  }
  if(x->expo==1)
    ptrtoscaling = scaling;
  return;
}

void scale_free(t_scale *x)
{
  t_freebytes(x->output_list,x->a_bytes);
}
