
/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "shared.h"
#include "common/loud.h"
#include "common/vefl.h"
#include "sickle/sic.h"

#define CYCLE_DEF_TABSIZE  512
#define COS_TABSIZE  16384



typedef struct _cycle
{
    t_sic      x_sic;
    double     x_phase;
    double     x_conv;
    t_symbol  *x_name;
    int        x_offset;
    //int		   x_buffer_sizeinsamps;
    int        x_cycle_tabsize; //how far to loop
    int		   x_use_all;
    t_float   *x_table;
    t_float   *x_costable;
    t_float   *x_usertable;
    t_float    x_usertableini[CYCLE_DEF_TABSIZE + 1];
    int        x_usertable_tabsize; // actual table size -- loop size might be smaller
} t_cycle;

static t_class *cycle_class;

static void cycle_gettable(t_cycle *x)
{   
    x->x_table = 0;
    int cycle_tabsize = x->x_cycle_tabsize;
    t_word *table = 0;
    int tabsize;
    
    if (x->x_name)
    {	
	table = vefl_get(x->x_name, &tabsize, 1, (t_pd *)x);
	}
		
	/* CHECKED buffer is copied */
	if (table)
    {
    	if (x->x_use_all) cycle_tabsize = x->x_cycle_tabsize = tabsize;
    	//else cycle_tabsize = x->x_cycle_tabsize = x->x_buffer_sizeinsamps;
    	int usertable_tabsize = x->x_usertable_tabsize;
	    int tablePtr, oldbytes;
	    int samplesFromTable = tabsize - x->x_offset;
	    int samplesToCopy = samplesFromTable < cycle_tabsize ?
		samplesFromTable : cycle_tabsize;
		
		/*  If requested cycle buffer size is larger than currently allocated,
			then resize the buffer, or if we're still using the builtin,
			get a new buffer on the heap. */
		if (cycle_tabsize > usertable_tabsize)
		{
			oldbytes = (usertable_tabsize + 1) * sizeof(*x->x_table);
			usertable_tabsize = x->x_usertable_tabsize = x->x_cycle_tabsize;
			if (x->x_usertable == x->x_usertableini)
			{
				if(!(x->x_usertable =
					getbytes((usertable_tabsize + 1) * sizeof(*x->x_usertable))))
				{
					x->x_usertable = x->x_usertableini;
					x->x_cycle_tabsize = x->x_usertable_tabsize = CYCLE_DEF_TABSIZE;
					pd_error(x,"unable to resize buffer; using size %d",CYCLE_DEF_TABSIZE);
				}
				else
				{
					//post("getting new table of size %d", cycle_tabsize);
					x->x_usertable_tabsize = cycle_tabsize;
				}
			}
			else
			{
				t_float *tmp;
				if(!(tmp =
					resizebytes(x->x_usertable, oldbytes,
						(usertable_tabsize + 1) * sizeof(*x->x_usertable))))
				{
					freebytes(x->x_usertable, oldbytes);
					x->x_usertable = x->x_usertableini;
					x->x_cycle_tabsize = x->x_usertable_tabsize = CYCLE_DEF_TABSIZE;
					pd_error(x,"unable to resize buffer; using size %d",CYCLE_DEF_TABSIZE);
				}
				else
				{
					x->x_usertable = tmp;
					x->x_usertable_tabsize = x->x_cycle_tabsize;
				}
			}
		}
	    // copy the internal table from the external one as far as 
	    // its size permits and fill the rest with zeroes.
	    for (tablePtr = 0; tablePtr < cycle_tabsize; tablePtr++)
	    {
		if (samplesToCopy > 0) 
		{
		    x->x_usertable[tablePtr] = 
			table[tablePtr + x->x_offset].w_float;
		    samplesToCopy--;
		}
		else 
		{
		    x->x_usertable[tablePtr] = 0;
		}
	    }
	    // the 513th sample
	    x->x_usertable[tablePtr] = (samplesFromTable > 0) ?
		table[x->x_offset].w_float : 0;

	    x->x_table = x->x_usertable;
	    /* CHECKED else no complaint */
	}
    else 
    {
    	if (x->x_name)
    	{
    		loud_error((t_pd *)x, "using cosine table");
    		x->x_name = 0;
    	}
    	x->x_table = x->x_costable;
    	x->x_cycle_tabsize = COS_TABSIZE;
    }
    if (!x->x_table)
    {
	/* CHECKED (incompatible) cycle~ is disabled -- garbage is output */
	x->x_table = x->x_usertable;
	memset(x->x_table, 0, (cycle_tabsize + 1) * sizeof(*x->x_table));
    }
}

static void cycle_set(t_cycle *x, t_symbol *s, t_floatarg f)
{
	x->x_use_all = 0;
	x->x_offset = 0;
	x->x_cycle_tabsize = CYCLE_DEF_TABSIZE;
    if (s && s != &s_)
    {
	x->x_name = s;
	if ((x->x_offset = (int)f) < 0)
	    x->x_offset = 0;
    }
    else x->x_name = 0;
    cycle_gettable(x);
}

static void cycle_setall(t_cycle *x, t_symbol *s)
{
	x->x_use_all = 1;
	x->x_offset = 0;
    if (s && s != &s_)
		x->x_name = s;
    else x->x_name = 0;
    cycle_gettable(x);
}

static void cycle_buffer_offset(t_cycle *x, t_floatarg f)
{
    if ((x->x_offset = (int)f) < 0)
    x->x_offset = 0;
    cycle_gettable(x);
}





static void cycle_buffer_sizeinsamps(t_cycle *x, t_floatarg f)
{

	if (!f)
	{
		x->x_use_all = 0;
		x->x_cycle_tabsize = CYCLE_DEF_TABSIZE;
	}
	else if (f == -1.)
	{
		x->x_use_all = 1;
	}
	else if (f == (1 << ilog2(f)) && f <= 65536. && f > 0)
	{
		x->x_use_all = 0;
		x->x_cycle_tabsize = f;
	}
	else
	{
		loud_error((t_pd *)x, "buffer_sizeinsamps must be a power of two <= 65536");
		return;
	}
	cycle_gettable(x);
}


static t_int *cycle_perform(t_int *w)
{
	t_cycle *x = (t_cycle *)(w[1]);
	int nblock = (int)(w[2]);
	t_float *in1 = (t_float *)(w[3]);
	t_float *in2 = (t_float *)(w[4]);
	t_float *out = (t_float *)(w[5]);
	t_float *tab = x->x_table;
	double dphase = x->x_phase;
	double conv = x->x_conv;
	double wrapphase, tabphase, frac;
	t_float f1, f2, freq, phasein;
	int intphase, index;
	int cycle_tabsize = x->x_cycle_tabsize;
	
	while (nblock--)
	{
		freq = *in1++;
		phasein = *in2++;
		wrapphase = dphase + phasein;
		if (wrapphase>=1.) 
		{
			intphase = (int)wrapphase;
			wrapphase -= intphase;
		}
		else if (wrapphase<=0.) 
		{
			intphase = (int)wrapphase;
			intphase--;
			wrapphase -= intphase;
		}
		
		tabphase = wrapphase * cycle_tabsize;
		index = (int)tabphase;
		frac = tabphase - index;
		f1 = tab[index++];
		f2 = tab[index];
		*out++ = f1 + frac * (f2 - f1);
		
		dphase += freq * conv ;
	}
	
	if (dphase>=1.)
	{
		intphase = (int)dphase;
		dphase -= intphase;
	}
	else if (dphase<=0.)
	{
		intphase = (int)dphase;
		intphase --;
		dphase -= intphase;
	}
	x->x_phase = dphase;
	return (w + 6);
}
	
static void cycle_dsp(t_cycle *x, t_signal **sp)
{
    cycle_gettable(x);
    x->x_conv = 1.0 / sp[0]->s_sr;
    x->x_phase = 0.;
    //post("cycle tabsize = %d", x->x_cycle_tabsize);
    dsp_add(cycle_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *cycle_new(t_symbol *s, int ac, t_atom *av)
{
    t_cycle *x = (t_cycle *)pd_new(cycle_class);
//    int i = (ac && av->a_type == A_FLOAT ? 1 : 0);
    int tabsize = CYCLE_DEF_TABSIZE;
    int costabsize = COS_TABSIZE;
    x->x_costable = sic_makecostable(&costabsize);
    x->x_usertable = x->x_usertableini;
    x->x_cycle_tabsize = x->x_usertable_tabsize = tabsize;

    
/*  if (tabsize != CYCLE_DEF_TABSIZE)
    {
	loudbug_bug("cycle_new");
	pd_free((t_pd *)x);
	return (0);
    } */
    
    if (ac && av->a_type == A_FLOAT)
    {
	sic_inlet((t_sic *)x, 0, 0, 0, ac, av);
	ac--, av++;
    }
    sic_newinlet((t_sic *)x, 0);
    outlet_new((t_object *)x, &s_signal);
    x->x_offset = 0;
    if (ac && av->a_type == A_SYMBOL)
    {
	x->x_name = av->a_w.w_symbol;
	ac--, av++;
	if (ac && av->a_type == A_FLOAT)
	    if ((x->x_offset = (int)av->a_w.w_float) < 0)
		x->x_offset = 0;
    }
    else x->x_name = 0;
    x->x_table = 0;
    x->x_phase = 0.;
    x->x_conv = 0.;
    x->x_use_all = 0;
    return (x);
}

void cycle_tilde_setup(void)
{    
    cycle_class = class_new(gensym("cycle~"),
        (t_newmethod)cycle_new, 0,
        sizeof(t_cycle), 0, A_GIMME, 0);
    sic_setup(cycle_class, cycle_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(cycle_class,
        (t_method)cycle_set,gensym("set"),
        A_DEFSYMBOL, A_DEFFLOAT, 0);
    class_addmethod(cycle_class,
        (t_method)cycle_buffer_offset,gensym("buffer_offset"),
        A_DEFFLOAT, 0);
    class_addmethod(cycle_class,
        (t_method)cycle_buffer_sizeinsamps,gensym("buffer_sizeinsamps"),
        A_DEFFLOAT, 0);
    class_addmethod(cycle_class,
        (t_method)cycle_setall,gensym("setall"),
        A_DEFSYMBOL, 0);
}