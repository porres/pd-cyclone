/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "toxy/plusbob.h"
#include "plustot.h"

typedef struct _plustot_out
{
    t_object   x_ob;
    t_binbuf  *x_bb;
} t_plustot_out;

static t_class *plustot_out_class;

static void plustot_out_symbol(t_plustot_out *x, t_symbol *s)
{
    Tcl_Obj *ob = plustag_tobvalue(s, (t_pd *)x);
    if (ob)
    {
	int len;
	char *ptr;
	Tcl_IncrRefCount(ob);
	ptr = Tcl_GetStringFromObj(ob, &len);
	if (ptr && len)
	{
	    int ac;
	    binbuf_text(x->x_bb, ptr, len);
	    if (ac = binbuf_getnatom(x->x_bb))
	    {
		t_atom *av = binbuf_getvec(x->x_bb);
		if (av->a_type == A_SYMBOL)
		    outlet_anything(((t_object *)x)->ob_outlet,
				    av->a_w.w_symbol, ac - 1, av + 1);
		else if (av->a_type == A_FLOAT)
		{
		    if (ac > 1)
			outlet_list(((t_object *)x)->ob_outlet,
				    &s_list, ac, av);
		    else
			outlet_float(((t_object *)x)->ob_outlet,
				     av->a_w.w_float);
		}
	    }
	}
	Tcl_DecrRefCount(ob);
    }
}

static void plustot_out_free(t_plustot_out *x)
{
    binbuf_free(x->x_bb);
}

void *plustot_out_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot_out *x = (t_plustot_out *)pd_new(plustot_out_class);
    x->x_bb = binbuf_new();
    outlet_new((t_object *)x, &s_anything);
    return (x);
}

void plustot_out_setup(void)
{
    plustot_out_class = class_new(gensym("+out"), 0,
				  (t_method)plustot_out_free,
				  sizeof(t_plustot_out), 0, 0);
    class_addsymbol(plustot_out_class, plustot_out_symbol);
}
