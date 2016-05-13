/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

// FIXME find a way of setting a 32-bit mask in an argument (?)
// FIXME SIG INPUT Not Working in 2nd inlet on Mode 3

#include "m_pd.h"
#include "unstable/forky.h"

typedef struct _bitand
{
    t_object x_obj;
    t_inlet *bitand;
    t_glist  *x_glist;
    t_int     x_mask;  // set as 'bits' message or argument
    int       x_mode;
    int       x_convert1;
} t_bitand;

static t_class *bitand_class;

static t_int *bitand_perform(t_int *w) // LATER think about performance
{
    t_bitand *x = (t_bitand *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_int mask = x->x_mask;
    switch (x->x_mode)
    {
        case 0:	while (nblock--)  // float inputs as raw 32bit
        { t_int i = (*(t_int *)(t_float *)in1++) & (*(t_int *)(t_float *)in2++);
	    *out++ = *(t_float *)&i;
        }
        break;
        case 1: while (nblock--) // convert float inputs to int
        { t_int i = ((t_int)*in1++) & ((t_int)*in2++);
	    *out++ = (t_float)i;
        }
        break;
        case 2: while (nblock--) // left input as raw 32bit - right as int
        { t_int i = (*(t_int *)(t_float *)in1++) & ((t_int)*in2++);
	    *out++ = *(t_float *)&i;
        }
        break;
        case 3: while (nblock--) // right input as raw 32bit - left as int
        { t_int i = ((t_int)*in1++) & (*(t_int *)(t_float *)in2++); // NOT WORKING!!!
	    *out++ = (t_float)i;
        }
	break;
    }
    return (w + 6);
}

static t_int *bitand_perform_noin2(t_int *w)
{ // LATER think about performance
    t_bitand *x = (t_bitand *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_int mask = x->x_mask;
    if (x->x_convert1) while (nblock--) // Mode 1 or 3 (left as int)
        { t_int i = ((t_int)*in++) & mask;
          *out++ = (t_float)i;
        }
    else while (nblock--)
        { t_int i = (*(t_int *)(t_float *)in++) & mask;
          *out++ = *(t_float *)&i;
        }
    return (w + 5);
}

static void bitand_dsp(t_bitand *x, t_signal **sp)
{
    if (forky_hasfeeders((t_object *)x, x->x_glist, 1, 0))
// use mask set by 2nd in sig/float (incompatible; 2nd inlet's int persisting) [?]
	dsp_add(bitand_perform, 5, x, sp[0]->s_n,
            sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
    
    else  // use mask set by 'bits' message or argument
	dsp_add(bitand_perform_noin2, 4, x, sp[0]->s_n,
            sp[0]->s_vec, sp[1]->s_vec);
}

static void bitand_bits(t_bitand *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mask = forky_getbitmask(ac, av);
}

static void bitand_mode(t_bitand *x, t_floatarg f)
{
    int i = (int)f;
    if (i < 0)
	i = 0;
    else if (i > 3)
	i = 3;
    x->x_mode = i;
    x->x_convert1 = (x->x_mode == 1 || x->x_mode == 3);
}

static void *bitand_new(t_floatarg f1, t_floatarg f2)
{
    t_bitand *x = (t_bitand *)pd_new(bitand_class);
    x->x_glist = canvas_getcurrent();
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_mask = (t_int)f1;  // FIXME (how?) || argument not working if anything (not only signal) is connected to it at object creation...
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