/********************

 Copyright (c) 2016 Marco Matteo Markidis

 mm.markidis@gmail.com

 For information on usage and redistribution, and for a DISCLAIMER OF ALL
 WARRANTIES, see the file, "LICENSE.txt," in this distribution.

 Made while listening:

 Hiatus Kaiyote -- Choose Your Weapon

********************/

// Porres made some changes to fix some issues

#include <string.h>
#include "m_pd.h"
#include <common/api.h>
#include <math.h>

static t_class *scale_class;

typedef struct _scale{
    t_object obj;
    t_outlet *float_outlet;
    t_float minin;
    t_float maxin;
    t_float minout;
    t_float maxout;
    t_float expo;
    t_float exp_in;
    t_atom *input_list;
    t_atom *output_list;
    t_int a_bytes;
    t_int flag;
    t_int ac;
    t_int input_type;
}t_scale;

static void scale_bang(t_scale *x);
static void scale_list(t_scale *x, t_symbol *s, int argc, t_atom *argv);
static void scale_free(t_scale *x);
static void scale_classic(t_scale *x, t_floatarg f);
static t_float scaling(t_scale *x, t_float f);
static t_float exp_scaling(t_scale *x, t_float f);
static t_float clas_scaling(t_scale *x, t_float f);
static t_float (*ptrtoscaling)(t_scale *x,t_float f);
static void check(t_scale *x);

static t_float exp_scaling(t_scale *x, t_float f){
    f = ((f-x->minin)/(x->maxin-x->minin) == 0)
        ? x->minout : (((f-x->minin)/(x->maxin-x->minin)) > 0)
        ? (x->minout + (x->maxout-x->minout) * pow((f-x->minin)/(x->maxin-x->minin),x->expo))
        : (x->minout + (x->maxout-x->minout) * -(pow(((-f+x->minin)/(x->maxin-x->minin)),x->expo)));
    return(f);
}

static t_float clas_scaling(t_scale *x, t_float f){
    f = (x->maxout-x->minout >= 0) ?
        (x->minout + (x->maxout-x->minout) * ((x->maxout - x->minout) *
        exp(-1*(x->maxin-x->minin)*log(x->expo)) * exp(f*log(x->expo)))) :
        (-1) * (x->minout + (x->maxout-x->minout) * ((x->maxout - x->minout) *
        exp(-1*(x->maxin-x->minin)*log(x->expo)) * exp(f*log(x->expo))));
    return(f);
}

static t_float scaling(t_scale *x, t_float f){
    return ((x->maxout - x->minout)*(f-x->minin)/(x->maxin-x->minin) + x->minout);
}

static void check(t_scale *x){
    if(x->flag == 1)
        x->expo = x->exp_in < 1. ? 1. : x->exp_in;
    else
        x->expo = x->exp_in < 0. ? 0. : x->exp_in;
    switch(x->flag){
        case 0:
            ptrtoscaling = exp_scaling;
        break;
        default:
            ptrtoscaling = clas_scaling;
        break;
    }
    if(x->expo == 1)
        ptrtoscaling = scaling;
    return;
}

static void scale_classic(t_scale *x, t_floatarg f){
    x->flag = f;
    check(x);
}

static void scale_list(t_scale *x, t_symbol *s, int ac, t_atom *av){
    if(!ac){
        scale_bang(x);
        return;
    }
    int i = 0;
    int old_a = x->a_bytes;
    if(av != x->input_list){
        x->ac = ac;
        x->a_bytes = ac*sizeof(t_atom);
        x->input_list = (t_atom *)t_resizebytes(x->input_list,old_a,x->a_bytes);
        x->output_list = (t_atom *)t_resizebytes(x->output_list,old_a,x->a_bytes);
        if(x->input_list == NULL || x->output_list == NULL) {
            pd_error(x,"[scale]: memory allocation failure");
            return;
        }
        memcpy(x->input_list,av,x->a_bytes);
        x->input_type = 1;
    }
    check(x);
    for(i = 0; i < x->ac; i++)
        SETFLOAT(x->output_list + i,
            ptrtoscaling(x,atom_getfloatarg(i,x->ac,x->input_list)));
    if(x->input_type == 0 && x->ac == 1)
        outlet_float(x->float_outlet,atom_getfloat(x->output_list));
    else
        outlet_list(x->float_outlet,0,x->ac,x->output_list);
}

static void scale_bang(t_scale *x){
    scale_list(x, &s_list, x->ac, x->input_list);
}

static void scale_free(t_scale *x){
    t_freebytes(x->input_list,x->a_bytes);
    t_freebytes(x->output_list,x->a_bytes);
}

static void *scale_new(t_symbol *s, int ac, t_atom *av){
    t_scale *x = (t_scale *)pd_new(scale_class);
    x->float_outlet = outlet_new(&x->obj, 0);
    floatinlet_new(&x->obj,&x->minin);
    floatinlet_new(&x->obj,&x->maxin);
    floatinlet_new(&x->obj,&x->minout);
    floatinlet_new(&x->obj,&x->maxout);
    floatinlet_new(&x->obj,&x->exp_in);
    x->minin = 0;
    x->maxin = 127;
    x->minout = 0;
    x->maxout = 1;
    x->flag = 0;
    x->exp_in = 1.f;
    t_int numargs = 0;
    while(ac>0) {
        t_symbol *firstarg = atom_getsymbolarg(0,ac,av);
        if(firstarg==&s_) {
            switch(numargs) {
            case 0:
                x->minin = atom_getfloatarg(0,ac,av);
                numargs++;
                ac--;
                av++;
                break;
            case 1:
                x->maxin = atom_getfloatarg(0,ac,av);
                numargs++;
                ac--;
                av++;
                break;
            case 2:
                x->minout = atom_getfloatarg(0,ac,av);
                numargs++;
                ac--;
                av++;
                break;
            case 3:
                x->maxout = atom_getfloatarg(0,ac,av);
                numargs++;
                ac--;
                av++;
                break;
            case 4:
                x->exp_in = atom_getfloatarg(0,ac,av);
                numargs++;
                ac--;
                av++;
                break;
            default:
                ac--;
                av++;
            }
        }
        else{
            t_int isclassic = strcmp(firstarg->s_name, "@classic") == 0;
            if(isclassic && ac>=1) {
                t_symbol *arg = atom_getsymbolarg(1,ac,av);
                if(arg == &s_) {
                    t_int dummy = atom_getintarg(1, ac, av);
                    if(dummy == 1)
                        x->flag = 1;
                    ac-=2;
                    av+=2;
                }
            }
            else{
                ac = 0;
                pd_error(x,"[scale]: improper args");
            }
        }
    }
    if(x->flag == 0){
        if(x->exp_in < 0.f) x->exp_in = 0;
    }
    else{
        if(x->exp_in < 1.f) x->exp_in = 1;
    }
    x->ac = 1;
    x->a_bytes = x->ac*sizeof(t_atom);
    x->input_list = (t_atom *)getbytes(x->a_bytes);
    x->output_list = (t_atom *)getbytes(x->a_bytes);
    if(x->input_list == NULL || x->output_list == NULL){
        pd_error(x,"[scale]: memory allocation failure");
        if(x->input_list)
            freebytes(x->input_list,x->a_bytes);
        if(x->output_list)
            freebytes(x->output_list,x->a_bytes);
        return NULL;
    }
    SETFLOAT(x->input_list, 0);
    x->input_type = 0;
    return(x);
}

CYCLONE_OBJ_API void scale_setup(void){
    t_class *c;
    scale_class = class_new(gensym("scale"), (t_newmethod)scale_new,
        (t_method)scale_free,sizeof(t_scale), 0, A_GIMME, 0);
    c = scale_class;
    class_addbang(c,(t_method)scale_bang);
    class_addlist(c,(t_method)scale_list);
    class_addmethod(c,(t_method)scale_classic,gensym("classic"),A_DEFFLOAT,0);
}
