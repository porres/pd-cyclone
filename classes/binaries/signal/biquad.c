// 64 bits???

#include "m_pd.h"

typedef struct _biquad
{
    t_object x_obj;
    t_inlet  *x_inlet_a0;
    t_inlet  *x_inlet_a1;
    t_inlet  *x_inlet_a2;
    t_inlet  *x_inlet_b1;
    t_inlet  *x_inlet_b2;
    t_float  x_xnm1;
    t_float  x_xnm2;
    t_float  x_ynm1;
    t_float  x_ynm2;
} t_biquad;

static t_class *biquad_class;

// ---------------------------------------------------
// biquad_list
// ---------------------------------------------------
static void biquad_list(t_biquad *x, t_symbol *s, int argc, t_atom *argv)
{
    pd_float((t_pd *)x->x_inlet_a0, atom_getfloatarg(0, argc, argv));
    pd_float((t_pd *)x->x_inlet_a1, atom_getfloatarg(1, argc, argv));
    pd_float((t_pd *)x->x_inlet_a2, atom_getfloatarg(2, argc, argv));
    pd_float((t_pd *)x->x_inlet_b1, atom_getfloatarg(3, argc, argv));
    pd_float((t_pd *)x->x_inlet_b2, atom_getfloatarg(4, argc, argv));
}


// ---------------------------------------------------
// biquad_clear
// ---------------------------------------------------
void biquad_clear(t_biquad *x){
    x->x_xnm1 = x->x_xnm2 = x->x_ynm1 = x->x_ynm2 = 0.;
}

// ---------------------------------------------------
// biquad_stoke
// ---------------------------------------------------
void biquad_stoke(t_biquad *x,
                  t_floatarg f1, t_floatarg f2, t_floatarg f3, t_floatarg f4)
{
    x->x_xnm1 = f1;
    x->x_xnm2 = f2;
    x->x_ynm1 = f3;
    x->x_ynm2 = f4;
}

// ---------------------------------------------------
// biquad_smooth
// ---------------------------------------------------
void biquad_smooth(t_biquad *x, t_floatarg f){ }

static t_int *biquad_perform(t_int *w)
{
    t_biquad *x = (t_biquad *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *coef_a0 = (t_float *)(w[4]);
    t_float *coef_a1 = (t_float *)(w[5]);
    t_float *coef_a2 = (t_float *)(w[6]);
    t_float *coef_b1 = (t_float *)(w[7]);
    t_float *coef_b2 = (t_float *)(w[8]);
    t_float *out = (t_float *)(w[9]);
    t_float xnm1 = x->x_xnm1;
    t_float xnm2 = x->x_xnm2;
    t_float ynm1 = x->x_ynm1;
    t_float ynm2 = x->x_ynm2;
    while (nblock--)
    {
        float yn, xn = *in++;
        float a0 = *coef_a0++;
        float a1 = *coef_a1++;
        float a2 = *coef_a2++;
        float b1 = *coef_b1++;
        float b2 = *coef_b2++;
        *out++ = yn = a0 * xn + a1 * xnm1 + a2 * xnm2 -b1 * ynm1 -b2 * ynm2;
        xnm2 = xnm1;
        xnm1 = xn;
        ynm2 = ynm1;
        ynm1 = yn;
    }
    x->x_xnm1 = xnm1;
    x->x_xnm2 = xnm2;
    x->x_ynm1 = ynm1;
    x->x_ynm2 = ynm2;
    return (w + 10);
}


static void biquad_dsp(t_biquad *x, t_signal **sp)
{
    dsp_add(biquad_perform, 9, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec,
            sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec);
}

static void *biquad_free(t_biquad *x)
{
		inlet_free(x->x_inlet_a0);
        inlet_free(x->x_inlet_a1);
        inlet_free(x->x_inlet_a2);
        inlet_free(x->x_inlet_b1);
        inlet_free(x->x_inlet_b2);
			return (void *)x;
}

static void *biquad_new(t_symbol *s, int argc, t_atom *argv)
{
    t_biquad *x = (t_biquad *)pd_new(biquad_class);
    t_float ca0, ca1, ca2, cb1, cb2;
    ca0 = ca1 = ca2 = cb1  = cb2 = 0.;
	int argnum = 0;
	while(argc > 0){
		if(argv -> a_type == A_FLOAT){
			t_float argval = atom_getfloatarg(0,argc,argv);
				switch(argnum){
					case 0:
						ca0 = argval;
						break;
					case 1:
						ca1 = argval;
						break;
                    case 2:
                        ca2 = argval;
                        break;
                    case 3:
                        cb1 = argval;
                        break;
                    case 4:
                        cb2 = argval;
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
    
	x->x_inlet_a0 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
	pd_float((t_pd *)x->x_inlet_a0, ca0);
    x->x_inlet_a1 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_a1, ca1);
    x->x_inlet_a2 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_a2, ca2);
    x->x_inlet_b1 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_b1, cb1);
    x->x_inlet_b2 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_b2, cb2);

    outlet_new((t_object *)x, &s_signal);

    x->x_xnm1 = 0;
    x->x_xnm2 = 0;
    x->x_ynm1 = 0;
    x->x_ynm2 = 0;
    
    return (x);
	errstate:
		pd_error(x, "biquad~: improper args");
		return NULL;
}

void biquad_tilde_setup(void)
{
    biquad_class = class_new(gensym("biquad~"),
				(t_newmethod)biquad_new,
                (t_method)biquad_free,
				sizeof(t_biquad), 0, A_GIMME, 0);
    class_addmethod(biquad_class, nullfn, gensym("signal"), 0);
    class_addmethod(biquad_class, (t_method)biquad_dsp, gensym("dsp"), A_CANT, 0);
    class_addlist(biquad_class, biquad_list);
    class_addmethod(biquad_class, (t_method) biquad_clear, gensym("clear"), 0);
    class_addmethod(biquad_class, (t_method) biquad_stoke, gensym("stoke"),
                    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(biquad_class, (t_method) biquad_smooth, gensym("smooth"), A_DEFFLOAT, 0);
}