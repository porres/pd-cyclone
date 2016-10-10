//Derek Kwan 2016 - de-arsiced,.. cybuffed

/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER: 'click' method */

#include <string.h>
#include "m_pd.h"
#include "cybuf.h"

#define INDEX_MAXCHANNELS  4  /* LATER implement arsic resizing feature */

typedef struct _index
{
    t_object    x_obj;
    t_cybuf   *x_cybuf;
    int      x_maxchannels;
    int      x_effchannel;  /* effective channel (clipped reqchannel) */
    int      x_reqchannel;  /* requested channel */
    t_inlet  *x_phaselet;
    t_outlet *x_outlet;
} t_index;

static t_class *index_class;

static void index_set(t_index *x, t_symbol *s){
    cybuf_setarray(x->x_cybuf, s);
}

static t_int *index_perform(t_int *w)
{
    t_index *x = (t_index *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *out = (t_float *)(w[4]);

    t_cybuf *c = x->x_cybuf;
    if (c->c_playable)
    {	
	t_float *xin = (t_float *)(w[3]);
	int index, maxindex = c->c_npts - 1;
	t_word *vp = c->c_vectors[x->x_effchannel];
	if (vp)  /* handle array swapping on the fly via ft1 */
	{
	    while (nblock--)
	    {
		index = (int)(*xin++ + 0.5);
		if (index < 0)
		    index = 0;
		else if (index > maxindex)
		    index = maxindex;
		*out++ = vp[index].w_float;
	    }
	}
	else while (nblock--) *out++ = 0;
    }
    else while (nblock--) *out++ = 0;
    return (w + 5);
}

static void index_float(t_index *x, t_float f)
{
    pd_error(x, "index~: no method for 'float'");
}

static void index_ft1(t_index *x, t_floatarg f)
{
    if ((x->x_reqchannel = (f > 1 ? (int)f - 1 : 0)) > x->x_maxchannels)
        x->x_effchannel = x->x_maxchannels - 1;
    else
        x->x_effchannel = x->x_reqchannel;
}

static void index_dsp(t_index *x, t_signal **sp)
{
    cybuf_checkdsp(x->x_cybuf); 
    dsp_add(index_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void index_free(t_index *x)
{
    inlet_free(x->x_phaselet);
    outlet_free(x->x_outlet);
    cybuf_free(x->x_cybuf);
}

static void *index_new(t_symbol *s, t_floatarg f)
{
    int ch = (f > 0 ? (int)f : 0);
    /* two signals:  index input, value output */
    t_index *x = (t_index *)pd_new(index_class);
    x->x_cybuf = cybuf_init((t_class *)x, s, (ch ? INDEX_MAXCHANNELS : 0));
    if (x->x_cybuf){
	if (ch > INDEX_MAXCHANNELS)
	    ch = INDEX_MAXCHANNELS;
	x->x_maxchannels = (ch ? INDEX_MAXCHANNELS : 1);
	x->x_effchannel = x->x_reqchannel = (ch ? ch - 1 : 0);
	x->x_phaselet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
    };
    return (x);
}

void index_tilde_setup(void)
{
    index_class = class_new(gensym("index~"),
			    (t_newmethod)index_new,
			    (t_method)index_free,
			    sizeof(t_index), 0,
			    A_DEFSYM, A_DEFFLOAT, 0);
    class_addmethod(index_class, (t_method)index_dsp, gensym("dsp"), A_CANT, 0);
    class_domainsignalin(index_class, -1);
     class_addfloat(index_class, (t_method)index_float);
    class_addmethod(index_class, (t_method)index_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(index_class, (t_method)index_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}
