//old code scrapped, brand new code by Derek Kwan - 2016

#include "m_pd.h"
#include <math.h>

#define CYLKUP_DEFSIZE 512

static t_class *lookup_class;

typedef struct _lookup
{
	t_object x_obj;
	t_symbol *x_arrayname;
	t_word *x_vec;
	t_float x_f; //dummy variable
	int x_npoints; //arraysize in samples

	t_inlet *x_offlet; //inlet for offset
	t_inlet *x_sizelet; //inlet for lookup size
	t_outlet *x_out; //outlet
} t_lookup;

static void lookup_set(t_lookup *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name){
            pd_error(x, "lookup~: %s: no such array", x->x_arrayname->s_name);
		};
        x->x_vec = 0;
    }
    else if (!garray_getfloatwords(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for lookup~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void *lookup_new(t_symbol *s, t_floatarg offset, t_floatarg lookupsz){ 
	t_lookup *x = (t_lookup *)pd_new(lookup_class);

	if(!s){ //null t_symbol
		s = &s_;
	};

	if(offset < 0){
		offset = 0;
	}
	else{
		offset = (t_float)floor(offset); //basically typecasting to int without actual typecasting
	};
	if(lookupsz <= 0){
		lookupsz = CYLKUP_DEFSIZE;
	}
	else{
		lookupsz = (t_float)floor(lookupsz);
	};

	x->x_arrayname = s;
	x->x_offlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	x->x_sizelet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	pd_float((t_pd *)x->x_offlet, offset);
	pd_float( (t_pd *)x->x_sizelet, lookupsz);
	x->x_out = outlet_new(&x->x_obj, gensym("signal"));
	
	return (x);
}

static t_int *lookup_perform(t_int *w)
{
	t_lookup *x = (t_lookup *)(w[1]);
	t_float *phase = (t_float *)(w[2]);
	t_float *offset = (t_float *)(w[3]);
	t_float *lookupsz = (t_float *)(w[4]);
	t_float *out = (t_float *)(w[5]);
	int n = (int)(w[6]);
	int npoints = x->x_npoints;
	int maxidx = npoints -1;
	t_word *buf = x->x_vec;

	int i;
	for(i=0;i<n;i++){
		double curout = 0.f;
		double curphs = phase[i]; //current phase
		int curlupsz = (int)lookupsz[i]; //current lookup size
		int curoff = (int)offset[i]; //current offset

		//bounds checking
		if(curlupsz > npoints){
			curlupsz = npoints;
		};
		if(curoff < 0){
			curoff = 0;
		}
		else if(curoff > maxidx){
			curoff = maxidx;
		};
		if((curlupsz + curoff) > npoints){
			//resize lookup size if offset + lookup size goes beyond array
			curlupsz = npoints-curoff;	
		};


		if(curphs >= -1 && curphs <= 1 && buf){
			//if phase if b/w -1 and 1, map and read, else 0
			//mapping curphs to the realidx where -1 maps to offset and 1 maps to offset + lookupsz - 1
			//resulting in mappings to lookupsz indices (if offset = 0, and lookupsz = 9, 8 is the maxidx to map to)
			int realidx; //the real mapped idx in the buffer
			realidx = curoff + (int)((curphs+1.f) * 0.5f * (curlupsz-1));

			//bounds checking
			if(realidx > maxidx){
				realidx = maxidx;
			};
			curout = buf[realidx].w_float; //read the buffer!
		};

		out[i] = curout;
	};
	
	return (w+7);
}

static void lookup_dsp(t_lookup *x, t_signal **sp)
{
	lookup_set(x, x->x_arrayname);
	dsp_add(lookup_perform, 6, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

static void *lookup_free(t_lookup *x)
{
	inlet_free(x->x_offlet);
	inlet_free(x->x_sizelet);
	outlet_free(x->x_out);
	return (void *)x;
}


void lookup_tilde_setup(void)
{
   lookup_class = class_new(gensym("lookup~"),
			     (t_newmethod)lookup_new,
			     (t_method)lookup_free,
			     sizeof(t_lookup), 0,
			     A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);

    CLASS_MAINSIGNALIN(lookup_class, t_lookup, x_f);
    class_addmethod(lookup_class, (t_method)lookup_dsp,
        gensym("dsp"), A_CANT, 0);
    class_addmethod(lookup_class, (t_method)lookup_set,
		    gensym("set"), A_SYMBOL, 0);
}
