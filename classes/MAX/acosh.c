/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"

#if defined(_WIN32) || defined(__APPLE__)
#define acoshf  acosh
#endif

typedef struct _acosh
{
    t_object  x_ob;
    float     x_value;
} t_acosh;

static t_class *acosh_class;

static void acosh_bang(t_acosh *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value);
}

static void acosh_float(t_acosh *x, t_float f)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value = acoshf(f)); /* no protection against NaNs */
}

static void *acosh_new(t_floatarg f)
{
    t_acosh *x = (t_acosh *)pd_new(acosh_class);
    x->x_value = acoshf(f); /* no protection against NaNs */
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void acosh_setup(void)
{
    acosh_class = class_new(gensym("acosh"),
			   (t_newmethod)acosh_new, 0,
			   sizeof(t_acosh), 0, A_DEFFLOAT, 0);
    class_addbang(acosh_class, acosh_bang);
    class_addfloat(acosh_class, acosh_float);
}
