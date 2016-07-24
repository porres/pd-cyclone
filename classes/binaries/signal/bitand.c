/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "unstable/forky.h"

typedef struct _bitand
{
    t_object  x_obj;
    t_inlet  *bitand;
    t_glist  *x_glist;
    int32_t   x_mask;
    int       x_mode;
    int       x_convert1;
} t_bitand;

static t_class *bitand_class;

static t_int *bitand_perform(t_int *w) // compare signals
{
    t_bitand *x = (t_bitand *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    switch (x->x_mode)
    {
        case 0:	while (nblock--)  // treat inputs as float
        { int32_t i = (*(int32_t *)(t_float *)in1++) & (*(int32_t *)(t_float *)in2++);
	    *out++ = *(t_float *)&i;
        }
        break;
        case 1: while (nblock--) // convert inputs to int
        { int32_t i = ((int32_t)*in1++) & ((int32_t)*in2++);
	    *out++ = (t_float)i;
        }
        break;
        case 2: while (nblock--) // convert right input to int
        { int32_t i = (*(int32_t *)(t_float *)in1++) & ((int32_t)*in2++);
	    *out++ = *(t_float *)&i;
        }
        break;
        case 3: while (nblock--) // convert left input to int
        { int32_t i = ((int32_t)*in1++) & (*(int32_t *)(t_float *)in2++);
        *out++ = (t_float)i;
        }
	break;
    }
    return (w + 6);
}

static t_int *bitand_perform_noin2(t_int *w) // compare to bitmask
{ // LATER think about performance
    t_bitand *x = (t_bitand *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    int32_t mask = x->x_mask;
    if (x->x_convert1) // convert left signal to int: Modes 1 (both as int) or 3 (left as int)
    while (nblock--)
        { int32_t i = ((int32_t)*in++) & mask;
          *out++ = (t_float)i;
        }
    else while (nblock--)
        { int32_t i = (*(int32_t *)(t_float *)in++) & mask;
          *out++ = *(t_float *)&i;
        }
    return (w + 5);
}

static void bitand_dsp(t_bitand *x, t_signal **sp)
{
    if (forky_hasfeeders((t_object *)x, x->x_glist, 1, 0)) // compare signals
	dsp_add(bitand_perform, 5, x,
        sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
    
    else  // use mask set by 'bits' message or argument
	dsp_add(bitand_perform_noin2, 4, x, sp[0]->s_n,
            sp[0]->s_vec, sp[1]->s_vec);
}

static void bitand_bits(t_bitand *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mask = forky_getbitmask(ac, av); // should overwrite argument???
}

static void bitand_mode(t_bitand *x, t_floatarg f)
{
    int i = (int)f;
    x->x_mode = i < 0 ? 0 : i > 3 ? 3 : i;
    x->x_convert1 = (x->x_mode == 1 || x->x_mode == 3);
}

static void *bitand_new(t_floatarg f1, t_floatarg f2)
{
    t_bitand *x = (t_bitand *)pd_new(bitand_class);
    x->x_glist = canvas_getcurrent();
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_mask = (int32_t)f1;
    bitand_mode(x, f2);
    return (x);
}

void bitand_tilde_setup(void)
{
    bitand_class = class_new(gensym("bitand~"), (t_newmethod)bitand_new, 0,
        sizeof(t_bitand), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(bitand_class, nullfn, gensym("signal"), 0);
    class_addmethod(bitand_class, (t_method) bitand_dsp, gensym("dsp"), 0);
    class_addmethod(bitand_class, (t_method)bitand_bits, gensym("bits"), A_GIMME, 0);
    class_addmethod(bitand_class, (t_method)bitand_mode, gensym("mode"), A_FLOAT, 0);
}