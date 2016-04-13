
#include "m_pd.h"
#include <string.h>
#include <ctype.h>

#ifndef CYbiquadNEAR_DEF
#define CYbiquadNEAR_DEF 1
#endif

#ifndef CYbiquadNUM_DEF
#define CYbiquadNUM_DEF 0.
#endif

static t_class *biquad_tilde_class;

typedef struct _biquad_tilde
{
	t_object x_obj;
	float x_nearest; //nearest attribute (1 = biquading, 0 = truncation)
    float  x_xnm1;
    float  x_xnm2;
    float  x_ynm1;
    float  x_ynm2;
} t_biquad_tilde;

static void *biquad_tilde_new(t_symbol *s, int argc, t_atom *argv)
{ float f;
	t_biquad_tilde *x = (t_biquad_tilde *)pd_new(biquad_tilde_class);
		int numargs = 0;
		int pastargs = 0;//if we haven't declared any attrs yet
		x->x_nearest = CYbiquadNEAR_DEF;
		f = CYbiquadNUM_DEF;
		while(argc > 0 ){
			if(argv -> a_type == A_FLOAT){ //if nullpointer, should be float or int
				if(!pastargs){
					switch(numargs){//we haven't declared attrs yet
						case 0: 	f = atom_getfloatarg(0, argc, argv);
									numargs++;
									argc--;
									argv++;
									break;
						default:	argc--;
									argv++;
									break;
					};
				}
				else{
					argc--;
					argv++;
				};
			}
			else if(argv -> a_type == A_SYMBOL){
				t_symbol *curarg = atom_getsymbolarg(0, argc, argv); //returns nullpointer if not symbol
				pastargs = 1;
				int isnear = strcmp(curarg->s_name, "@nearest") == 0;
				if(isnear && argc >= 2){
					t_symbol *arg1 = atom_getsymbolarg(1, argc, argv);
					if(arg1 == &s_){// is a number
						x->x_nearest = atom_getfloatarg(1, argc, argv);
						argc -= 2;
						argv += 2;
						}
					else{
						goto errstate;
					};}
				else{
					goto errstate;
					};
			}
			else{
				goto errstate;
			};
	};

	pd_float( (t_pd *)inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal), f);
	 outlet_new(&x->x_obj, gensym("signal"));
	return (x);
	
	errstate:
		pd_error(x, "biquad~: improper args");
		return NULL;
}


static t_int * biquad_perform(t_int *w){
    t_biquad *x = (t_biquad *)(w[1]);
    int n = (int)(w[2]);
    t_float *xn = (t_float *)(w[3]);
    t_float *a0 = (t_float *)(w[4]);
    t_float *a1 = (t_float *)(w[5]);
    t_float *a2 = (t_float *)(w[6]);
    t_float *b1 = (t_float *)(w[7]);
    t_float *b2 = (t_float *)(w[8]);
    t_float *out = (t_float *)(w[9]);
    float xnm1 = x->x_xnm1;
    float xnm2 = x->x_xnm2;
    float ynm1 = x->x_ynm1;
    float ynm2 = x->x_ynm2;
    while (n--)
    {
        float yn;
        //      *out++ = yn = (*xn++ * *a0++) + (xnm1 * *a1++) + (xnm2 * *a2++) - (ynm1 * *b1++) - (ynm2 * *b2++);
        *out++ = yn = (*xn++ * *a0++) + (*a1++) + (*a2++) - (*b1++) - (*b2++);
        xnm2 = xnm1;
        xnm1 = *xn++;
        ynm2 = ynm1;
        ynm1 = yn;
    }
    x->x_xnm1 = xnm1;
    x->x_xnm2 = xnm2;
    x->x_ynm1 = ynm1;
    x->x_ynm2 = ynm2;
    return (w + 10);
}



static void biquad_tilde_nearest(t_biquad_tilde *x, t_float f, t_float glob){
	if(f <= 0.){
		x->x_nearest = 0;
	}
	else{
		x->x_nearest = 1;
	};
}
static void biquad_tilde_dsp(t_biquad_tilde *x, t_signal **sp)
{
	dsp_add(biquad_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
            sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec , sp[0]->s_n);
}

void biquad_tilde_setup(void)
{
	biquad_tilde_class = class_new(gensym("biquad~"), (t_newmethod)biquad_tilde_new, 0,
	sizeof(t_biquad_tilde), 0, A_GIMME, 0);
	class_addmethod(biquad_tilde_class, nullfn, gensym("signal"), 0);
	class_addmethod(biquad_tilde_class, (t_method)biquad_tilde_dsp, gensym("dsp"), 0);
	class_addmethod(biquad_tilde_class, (t_method)biquad_tilde_nearest,  gensym("nearest"), A_FLOAT, 0);
}