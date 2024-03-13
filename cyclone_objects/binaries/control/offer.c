/* Copyright (c) 2002-2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <common/api.h>
#include "control/tree.h"

/* As a class `derived' from the common hammertree code (also in funbuff),
   offer maintains the auxiliary list, the main purpose of which is faster
   traversal (not needed here).  As a side-effect, there is a bonus of a small
   speedup of deletion, and a penalty of a small slowdown of insertion. */

// Derek and Porres, added offer_bang and cleanup

typedef struct _offer{
    t_object      x_obj;
    t_float       x_value;
    int           x_valueset;
    t_hammertree  x_tree;
}t_offer;

static t_class *offer_class;

static void offer_float(t_offer *x, t_float f){
    int ndx = (int)f;
    if((t_float)ndx == f){
        t_hammernode *np;
        if(x->x_valueset){
            hammertree_insertfloat(&x->x_tree, ndx, x->x_value, 1);
            x->x_valueset = 0;
        }
        else if(np = hammertree_search(&x->x_tree, ndx)){
            outlet_float(((t_object *)x)->ob_outlet, HAMMERNODE_GETFLOAT(np));
            hammertree_delete(&x->x_tree, np);
        }
    }
    else
        pd_error(x, "[offer]: doesn't understand \"noninteger float\"");
}

static void offer_ft1(t_offer *x, t_floatarg f){
    x->x_value = (int)f;
    x->x_valueset = 1;
}

static void offer_bang(t_offer * x){
    t_hammernode * node = x->x_tree.t_last;
    while(node){
        t_float curf = HAMMERNODE_GETFLOAT(node);
        outlet_float(((t_object *)x)->ob_outlet, curf);
        node = node->n_prev;
    };
}

static void offer_clear(t_offer *x){
    hammertree_clear(&x->x_tree, 0);
}

static void offer_free(t_offer *x){
    hammertree_clear(&x->x_tree, 0);
}

static void *offer_new(void){
    t_offer *x = (t_offer *)pd_new(offer_class);
    x->x_valueset = 0;
    hammertree_inittyped(&x->x_tree, HAMMERTYPE_FLOAT, 0);
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    return(x);
}

CYCLONE_OBJ_API void offer_setup(void){
    offer_class = class_new(gensym("offer"), (t_newmethod)offer_new,
        (t_method)offer_free, sizeof(t_offer), 0, 0);
    class_addfloat(offer_class, offer_float);
    class_addmethod(offer_class, (t_method)offer_ft1, gensym("ft1"), A_FLOAT, 0);
    class_addmethod(offer_class, (t_method)offer_clear, gensym("clear"), 0);
    class_addbang(offer_class, offer_bang);
}
