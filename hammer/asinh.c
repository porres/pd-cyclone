/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"

#if defined(_WIN32) || defined(__APPLE__)
#define asinhf  asinh
#endif

typedef struct _asinh
{
    t_object  x_ob;
    float     x_value;
} t_asinh;

static t_class *asinh_class;

static void asinh_bang(t_asinh *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value);
}

static void asinh_float(t_asinh *x, t_float f)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value = asinhf(f)); /* no protection against NaNs */
}

static void *asinh_new(t_floatarg f)
{
    t_asinh *x = (t_asinh *)pd_new(asinh_class);
    x->x_value = asinhf(f); /* no protection against NaNs */
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void asinh_setup(void)
{
    asinh_class = class_new(gensym("asinh"),
			   (t_newmethod)asinh_new, 0,
			   sizeof(t_asinh), 0, A_DEFFLOAT, 0);
    class_addbang(asinh_class, asinh_bang);
    class_addfloat(asinh_class, asinh_float);
}
