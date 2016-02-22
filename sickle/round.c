#include "m_pd.h"
#include <math.h>
#include <string.h>
#include <ctype.h>

static t_class *round_tilde_class;

typedef struct _round_tilde
{
	t_object x_obj;
	float x_f; //dummy var needed for CLASS_MAINSIGNALIN
	float x_nearest; //nearest attribute (1 = rounding, 0 = truncation)
} t_round_tilde;

static int checknum(const char *s)
{
	while(*s){
		if (isdigit(*s++) == 0) return 0;
	};
	return 1;
}

static void *round_tilde_new(t_symbol *s, int argc, t_atom *argv)
{ float f;
	int argcheck; //bool for checking arguments
	t_round_tilde *x = (t_round_tilde *)pd_new(round_tilde_class);
	if(argc == 1){	//one argument should be the rounded number
		t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
		x->x_nearest = 1;
		argcheck = checknum(firstarg->s_name);
		if(argcheck == 0){
			pd_error(x, "round~: improper args");
			return NULL;
		}
		else{
			f = atom_getfloatarg(0, argc, argv);
		};
	};
	if(argc == 2){ //two args should be '@nearest' and nearest arg
		t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
		f = 0.;
		if(strcmp(firstarg->s_name, "@nearest")==0){
			x->x_nearest = atom_getfloatarg(1, argc, argv);
		}
		else{
			pd_error(x, "round~: improper args");
			return NULL;
		};
	};
	if(argc == 3){
		t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
		argcheck = checknum(firstarg->s_name);
		if(argcheck == 1){//first arg is rounded number
			f = atom_getfloatarg(0, argc, argv);
			t_symbol *secondarg = atom_getsymbolarg(1, argc, argv);
			if(strcmp(secondarg->s_name, "@nearest")==0){//second arg is '@nearest'
				x->x_nearest = atom_getfloatarg(2, argc, argv);
			}
			else{
				pd_error(x, "round~: improper args");
				return NULL;
			};
		}
		else{
			pd_error(x, "round~: improper args");
			return NULL;
		};
	};
	if(!argc || argc < 1){
		x->x_nearest = 1;
		f = 0.;
	};
	pd_float( (t_pd *)inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal), f);
	 outlet_new(&x->x_obj, gensym("signal"));
	return (x);
	
}
static t_int *round_tilde_perform(t_int *w)
{
	t_round_tilde *x = (t_round_tilde *)(w[1]);
	t_float *in1 = (t_float *)(w[2]);
	t_float *in2 = (t_float *)(w[3]);
	t_float *out = (t_float *)(w[4]);
	int n = (int)(w[5]);
	float nearfloat;
	int nearest;

	nearfloat = x->x_nearest;
	if(nearfloat <= 0.){
		nearest = 0;
	}
	else{
		nearest = 1;
	};
	
	while (n--){
		float rounded,div;
		float val = *(in1++);
		float roundto = *(in2++);
		if(roundto > 0.){
			div = val/roundto; //get the result of dividing the two
			if(nearest == 1){//rounding
				rounded = roundto*round(div); //round quotient to nearest int and multiply roundto by result
			}
			else{//truncation
				rounded = roundto*(float)((int)div); //else lop off the decimal and multiply roundto by result
			};
		}
		else{//round is 0, do nothing
			rounded = val;

		};
		*out++ = rounded;
	};
	return (w+6);
}


static void round_tilde_nearest(t_round_tilde *x, t_float f, t_float glob){
	if(f <= 0.){
		x->x_nearest = 0.;
	}
	else{
		x->x_nearest = f;
	};
}
static void round_tilde_dsp(t_round_tilde *x, t_signal **sp)
{
	dsp_add(round_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}


void round_tilde_setup(void)
{
	round_tilde_class = class_new(gensym("round~"), (t_newmethod)round_tilde_new, 0,
	sizeof(t_round_tilde), 0, A_GIMME, 0);
	CLASS_MAINSIGNALIN(round_tilde_class, t_round_tilde, x_f);
	class_addmethod(round_tilde_class, (t_method)round_tilde_dsp, gensym("dsp"), 0);
	class_addmethod(round_tilde_class, (t_method)round_tilde_nearest,  gensym("nearest"), A_FLOAT, 0);
}
