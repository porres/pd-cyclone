/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <common/api.h>
#include <common/random.h>

typedef struct _decide{
    t_object      x_ob;
    unsigned int  x_seed;
    int           x_id;
}t_decide;

static t_class *decide_class;

static void decide_bang(t_decide *x){
    unsigned int state = x->x_seed;
    float rand = ((float)((state & 0x7fffffff) - 0x40000000))
        * (float)(1.0 / 0x40000000);
    x->x_seed = state * 435898247 + 382842987;
    outlet_float(((t_object *)x)->ob_outlet, rand > 0);
}

static void decide_float(t_decide *x, t_float f){
    f = 0;
	decide_bang(x);
}

static void decide_ft1(t_decide *x, t_floatarg f){
    x->x_seed = (int)f;
    if(x->x_seed == 0)
        x->x_seed = (int)(time(NULL)*x->x_id);
}

static void *decide_new(t_floatarg f){
    t_decide *x = (t_decide *)pd_new(decide_class);
    x->x_seed = (int)(time(NULL) * x->x_id * 1319);
    x->x_state = x->x_seed * 435898247 + 382842987;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    return(x);
}

CYCLONE_OBJ_API void decide_setup(void){
    decide_class = class_new(gensym("decide"), (t_newmethod)decide_new, 0,
        sizeof(t_decide), 0, A_DEFFLOAT, 0);
    class_addbang(decide_class, decide_bang);
    class_addfloat(decide_class, decide_float);
    class_addmethod(decide_class, (t_method)decide_ft1, gensym("ft1"), A_FLOAT, 0);
}
