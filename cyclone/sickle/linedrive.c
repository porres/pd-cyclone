/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKME polarity */

#include <math.h>
#include "m_pd.h"

#if defined(NT) || defined(MACOSX)
#define logf  log
#define expf  exp
#endif

static t_class *linedrive_class;

typedef struct _linedrive
{
    t_object  x_ob;
    t_float   x_maxin;
    t_float   x_maxout;
    t_float   x_expcoef;
    t_float   x_lincoef;
    t_atom    x_vec[2];
    int       x_linear;
} t_linedrive;

static void linedrive_float(t_linedrive *x, t_floatarg f)
{
    float outval = f - x->x_maxin;
    if (outval >= 0)
	outval = x->x_maxout;  /* CHECKED */
    else if (x->x_linear)
	outval = x->x_maxout + outval * x->x_lincoef;
    else
	outval = expf(outval * x->x_expcoef) * x->x_maxout;
    SETFLOAT(x->x_vec, outval);
    outlet_list(((t_object *)x)->ob_outlet, 0, 2, x->x_vec);
}

static void *linedrive_new(t_floatarg maxin, t_floatarg maxout,
			   t_floatarg curve, t_floatarg deltime)
{
    t_linedrive *x = (t_linedrive *)pd_new(linedrive_class);
    x->x_maxin = (maxin < 1.0e-20f && maxin > -1e-20f ? 0 : maxin);
    x->x_maxout = maxout;
    if (curve < 1.0e-20f) curve = 1.0;  /* a bug in msp? */
    if (curve == 1.0)
    {
	x->x_expcoef = 0;
	x->x_lincoef = (x->x_maxin == 0 ? 0 : x->x_maxout / x->x_maxin);
	x->x_linear = 1;
    }
    else {
	x->x_expcoef = logf(curve);
	x->x_lincoef = 0;
	x->x_linear = 0;
    }
    SETFLOAT(&x->x_vec[1], deltime);  /* CHECKED: any value accepted */
    floatinlet_new((t_object *)x, &x->x_vec[1].a_w.w_float);
    outlet_new((t_object *)x, &s_list);
    return (x);
}

void linedrive_setup(void)
{
    linedrive_class = class_new(gensym("linedrive"),
				(t_newmethod)linedrive_new, 0,
				sizeof(t_linedrive), 0,
				A_DEFFLOAT, A_DEFFLOAT,
				A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(linedrive_class, linedrive_float);
}
