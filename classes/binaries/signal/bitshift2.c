/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

static t_class *bitshift_class;

typedef struct _bitshift {
    t_object x_obj;
    t_outlet *x_outlet;
    int    x_convert1;
    int    x_lshift;
    int    x_rshift;
//    int    x_lover;
} t_bitshift;

void *bitshift_new(t_floatarg f1, t_floatarg f2);
static t_int * bitshift_perform(t_int *w);
static void bitshift_dsp(t_bitshift *x, t_signal **sp);
static void bitshift_mode(t_bitshift *x, t_floatarg f);
static void bitshift_float(t_bitshift *x, t_float f);


static t_int * bitshift_perform(t_int *w)
{   // LATER think about performance
    t_bitshift *x = (t_bitshift *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    if (x->x_lshift)
    {
        unsigned int shift = x->x_lshift;
        if (x->x_convert1) while (nblock--)
        {
            t_int i = ((t_int)*in++ << shift);
            *out++ = (t_float)i;
        }
        else while (nblock--)
        {
            t_int i = (*(t_int *)(t_float *)in++ << shift);
            *out++ = *(t_float *)&i;
        }
    }
    else if (x->x_rshift)
    {
        unsigned int shift = x->x_rshift;
        if (x->x_convert1) while (nblock--)
        {
            t_int i = ((t_int)*in++ >> shift); /* CHECKME */
            *out++ = (t_float)i;
        }
        else while (nblock--)
        {
            t_int i = (*(t_int *)(t_float *)in++ >> shift);  /* CHECKME */
            *out++ = *(t_float *)&i;
        }
    }
//    else if (x->x_lover)
//        while (nblock--) *out++ = 0;  /* CHECKED both modes */
    else
        while (nblock--) *out++ = *in++;  /* CHECKED both modes */
    return (w + 5);
}

static void bitshift_dsp(t_bitshift *x, t_signal **sp)
{
    dsp_add(bitshift_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void bitshift_mode(t_bitshift *x, t_floatarg f)
{
    int i = (int)f;
    x->x_convert1 = (i > 0);  /* CHECKME */
}

static void bitshift_float(t_bitshift *x, t_float f)
{
    int i = (int)f;
    x->x_convert1 = (i > 0);  /* CHECKME */
}

/* static void bitshift_shift(t_bitshift *x, t_floatarg f)
{
    int i = (int)f;
    int nbits = sizeof(t_int) * 8;
    x->x_lshift = x->x_rshift = 0;
    x->x_lover = 0;
    if (i > 0)
    {
        if (i < nbits)
            x->x_lshift = i;
        else
            x->x_lover = 1;
    }
    else if (i < 0)
    {
        x->x_rshift = (i <= -nbits ? nbits - 1 : -i);
    }
} */

static void bitshift_shift(t_bitshift *x, t_floatarg f)
 {
 int i = (int)f;
 x->x_lshift = x->x_rshift = 0;
 if (i > 0) x->x_lshift = i;
 else x->x_rshift = -i;
 }

void *bitshift_new(t_floatarg f1, t_floatarg f2)
{
    t_bitshift *x = (t_bitshift *)pd_new(bitshift_class);
    x->x_outlet = outlet_new(&x->x_obj, &s_signal);
    bitshift_shift(x, f1);
    bitshift_mode(x, f2);
    return (x);
}

void bitshift_tilde_setup(void) { bitshift_class = class_new(gensym("bitshift~"),
        (t_newmethod) bitshift_new, 0, sizeof (t_bitshift), CLASS_DEFAULT,
        A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(bitshift_class, (t_method)bitshift_float);
    class_addmethod(bitshift_class, nullfn, gensym("signal"), 0);
    class_addmethod(bitshift_class, (t_method) bitshift_dsp, gensym("dsp"), 0);
    class_addmethod(bitshift_class, (t_method)bitshift_mode, gensym("mode"), A_FLOAT, 0);
    class_addmethod(bitshift_class, (t_method)bitshift_shift, gensym("shift"), A_FLOAT, 0);
}