/* Copyright (c) 2016 Porres.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "shared.h"

#if defined(_WIN32) || defined(__APPLE__)/* Precisa? */
#endif

typedef struct _rdiv
{
    t_object  x_ob;
    t_float   x_f1;
    t_float   x_f2;
} t_rdiv;

static t_class *rdiv_class;

static void rdiv_bang(t_rdiv *x)
{
    if (x->x_f1 != 0.)
        outlet_float(((t_object *)x)->ob_outlet, x->x_f2 / x->x_f1);
    else
        outlet_float(((t_object *)x)->ob_outlet,
                     (x->x_f2 > 0 ? SHARED_INT_MAX : SHARED_INT_MIN));
}


static void rdiv_float(t_rdiv *x, t_float f)
{
    x->x_f1 = f;
    rdiv_bang(x);
}

static void *rdiv_new(t_floatarg f)
{
    t_rdiv *x = (t_rdiv *)pd_new(rdiv_class);
    floatinlet_new((t_object *)x, &x->x_f2);
    outlet_new((t_object *)x, &s_float);
    x->x_f1 = 0;
    x->x_f2 = f;
    return (x);
}

void rdiv_setup(void)
{
    rdiv_class = class_new(gensym("rdiv"),
			     (t_newmethod)rdiv_new, 0,
			     sizeof(t_rdiv), 0, A_DEFFLOAT, 0);
    class_addbang(rdiv_class, rdiv_bang);
    class_addfloat(rdiv_class, rdiv_float);
}