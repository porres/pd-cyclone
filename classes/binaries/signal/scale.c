// Porres 2016

#include "m_pd.h"
#include <math.h>

#define SCALE_MININ  0.
#define SCALE_MAXIN  127.
#define SCALE_MINOUT  0.
#define SCALE_MAXOUT  1.
#define SCALE_EXPO  1.

typedef struct _scale
{
    t_object x_obj;
    t_inlet  *x_inlet_1;
    t_inlet  *x_inlet_2;
    t_inlet  *x_inlet_3;
    t_inlet  *x_inlet_4;
    t_inlet  *x_inlet_5;
} t_scale;

static t_class *scale_class;

static t_int *scale_perform(t_int *w)
{
    t_scale *x = (t_scale *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *in4 = (t_float *)(w[6]);
    t_float *in5 = (t_float *)(w[7]);
    t_float *in6 = (t_float *)(w[8]);
    t_float *out = (t_float *)(w[9]);
    while (nblock--)
    {
        float in = *in1++;
        float minin = *in2++;
        float maxin = *in3++;
        float minout = *in4++;
        float maxout = *in5++;
        float expo = *in6++;
        {
        *out++ = (maxout - minout >= 0) ? (minout + (maxout - minout) * ((maxout - minout) * exp(-1*(maxin - minin)*log(expo)) * exp(in * log(expo)))) : (-1) * (minout + (maxout - minout) * ((maxout - minout) * exp(-1*(maxin - minin)*log(expo)) * exp(in * log(expo))));
        }
    }
    return (w + 10);
}

static void scale_dsp(t_scale *x, t_signal **sp)
{
    dsp_add(scale_perform, 9, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec,
            sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec);
}

static void *scale_free(t_scale *x)
{
		inlet_free(x->x_inlet_1);
        inlet_free(x->x_inlet_2);
        inlet_free(x->x_inlet_3);
        inlet_free(x->x_inlet_4);
        inlet_free(x->x_inlet_5);
        return (void *)x;
}

static void *scale_new(t_symbol *s, int argc, t_atom *argv)
{
    t_scale *x = (t_scale *)pd_new(scale_class);
    
    t_float min_in, max_in, min_out, max_out, exponential;
    min_in = SCALE_MININ;
    max_in = SCALE_MAXIN;
    min_out = SCALE_MINOUT;
    max_out = SCALE_MAXOUT;
    exponential = SCALE_EXPO;
    
	int argnum = 0;
	while(argc > 0){
		if(argv -> a_type == A_FLOAT){
			t_float argval = atom_getfloatarg(0,argc,argv);
				switch(argnum){
					case 0:
						min_in = argval;
						break;
					case 1:
						max_in = argval;
						break;
                    case 2:
                        min_out = argval;
                        break;
                    case 3:
                        max_out = argval;
                        break;
                    case 4:
                        exponential = argval;
                        break;
					default:
						break;
				};
				argc--;
				argv++;
				argnum++;
		}
		else{
			goto errstate;
		};
	};
    
	x->x_inlet_1 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
	pd_float((t_pd *)x->x_inlet_1, min_in);
    x->x_inlet_2 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_2, max_in);
    x->x_inlet_3 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_3, min_out);
    x->x_inlet_4 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_4, max_out);
    x->x_inlet_5 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_5, exponential);

    outlet_new((t_object *)x, &s_signal);
    
    return (x);
	errstate:
		pd_error(x, "scale~: improper args");
		return NULL;
}

void scale_tilde_setup(void)
{
    scale_class = class_new(gensym("scale~"),
				(t_newmethod)scale_new,
                (t_method)scale_free,
				sizeof(t_scale), 0, A_GIMME, 0);
    class_addmethod(scale_class, nullfn, gensym("signal"), 0);
    class_addmethod(scale_class, (t_method)scale_dsp, gensym("dsp"), A_CANT, 0);
//    class_addmethod(scale_class, (t_method) scale_classic, gensym("classic"), A_DEFFLOAT, 0);
}