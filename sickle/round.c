#include "m_pd.h"
#include <math.h>
#include "sickle/sic.h"

//can't figure out how to make round fallback to float arg if right inlet has no signal (like max version)
// so made it a float inlet for now. CLASS_MAINSIGNALIN does this but only for the
// main inlet. the original cyclone/pong~ does it but it uses nonstandard pd methods

//also added a second inlet to set mode (done in max by attribute)
static t_class *round_tilde_class;

typedef struct _round_tilde
{
	t_object x_obj;
	float x_f; //dummy var needed for CLASS_MAINSIGNALIN
	float x_round; //value to round to
	float x_nearest; //nearest attribute (1 = rounding, 0 = truncation)
} t_round_tilde;

static void *round_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
	t_round_tilde *x = (t_round_tilde *)pd_new(round_tilde_class);
	if(argc >= 1){
		float f = atom_getfloatarg(0, argc, argv);
		if(f < 0.){
			x->x_round = 0.;	
			}
		else{
			x->x_round = f;
		};
	}
	if(argc == 2){
		x->x_nearest = atom_getfloatarg(1, argc, argv);
	};
	if(argc < 2){
		x->x_nearest = 1.;
	};
	if(!argc){
		x->x_round = 0.;
		x->x_nearest = 1.;
	};
	outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_round);
	floatinlet_new(&x->x_obj, &x->x_nearest);
	return (x);
	}

static t_int *round_tilde_perform(t_int *w)
{
	t_round_tilde *x = (t_round_tilde *)(w[1]);
	t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	float roundto, nearfloat;
	int nearest;

	nearfloat = x->x_nearest;
	roundto = x->x_round;
	if(nearfloat <= 0.){
		nearest = 0;
	}
	else{
		nearest = 1;
	};
	if(roundto < 0){
		roundto = 0.;
	};
	
	while (n--){
		float rounded,div;
		float val = *(in++);
		if(roundto != 0.){
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
	return (w+5);
}


static void round_tilde_dsp(t_round_tilde *x, t_signal **sp)
{
	dsp_add(round_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void round_tilde_setup(void)
{
	round_tilde_class = class_new(gensym("round~"), (t_newmethod)round_tilde_new, 0,
	sizeof(t_round_tilde), 0, A_GIMME, 0);
	CLASS_MAINSIGNALIN(round_tilde_class, t_round_tilde, x_f);
	class_addmethod(round_tilde_class, (t_method)round_tilde_dsp, gensym("dsp"), 0);
}
