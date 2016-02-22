#include "m_pd.h"
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


#ifndef HAVE_ALLOCA     /* can work without alloca() but we never need it */
#define HAVE_ALLOCA 1
#endif
#define TEXT_NGETBYTE 100 /* bigger that this we use alloc, not alloca */
#if HAVE_ALLOCA
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)((n) < TEXT_NGETBYTE ?  \
			        alloca((n) * sizeof(t_atom)) : getbytes((n) * sizeof(t_atom))))
#define ATOMS_FREEA(x, n) ( \
		    ((n) < TEXT_NGETBYTE || (freebytes((x), (n) * sizeof(t_atom)), 0)))
#else
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)getbytes((n) * sizeof(t_atom)))
#define ATOMS_FREEA(x, n) (freebytes((x), (n) * sizeof(t_atom)))
#endif


static t_class *round_class;

typedef struct _round
{
	t_object x_obj;
	t_float x_round;
	float x_nearest; //nearest attribute (1 = rounding, 0 = truncation)
} t_round;

static int checknum(const char *s)
{
	while(*s){
		if (isdigit(*s++) == 0) return 0;
	};
	return 1;
}

static void *round_new(t_symbol *s, int argc, t_atom *argv)
{ 
	int argcheck; //bool for checking arguments
	t_round *x = (t_round *)pd_new(round_class);
	if(argc == 1){	//one argument should be the rounded number
		t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
		x->x_nearest = 1;
		argcheck = checknum(firstarg->s_name);
		if(argcheck == 0){
			pd_error(x, "round: improper args");
			return NULL;
		}
		else{
			x->x_round = atom_getfloatarg(0, argc, argv);
		};
	};
	if(argc == 2){ //two args should be '@nearest' and nearest arg
		t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
		x->x_round = 0.;
		if(strcmp(firstarg->s_name, "@nearest")==0){
			x->x_nearest = atom_getfloatarg(1, argc, argv);
		}
		else{
			pd_error(x, "round: improper args");
			return NULL;
		};
	};
	if(argc == 3){
		t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
		argcheck = checknum(firstarg->s_name);
		if(argcheck == 1){//first arg is rounded number
			x->x_round = atom_getfloatarg(0, argc, argv);
			t_symbol *secondarg = atom_getsymbolarg(1, argc, argv);
			if(strcmp(secondarg->s_name, "@nearest")==0){//second arg is '@nearest'
				x->x_nearest = atom_getfloatarg(2, argc, argv);
			}
			else{
				pd_error(x, "round: improper args");
				return NULL;
			};
		}
		else{
			pd_error(x, "round: improper args");
			return NULL;
		};
	};
	if(!argc || argc < 1){
		x->x_nearest = 1;
		x->x_round = 0.;
	};
	floatinlet_new(&x->x_obj, &x->x_round);
 
	outlet_new(&x->x_obj, gensym("list"));
	return (x);
	
}

static float rounding_algo(float val, float roundto, int mode){
//rounding util function, val = val to round, roundto = val to round to, mode = nearest
	float div, rounded;	
		if(roundto > 0.){
			div = val/roundto; //get the result of dividing the two
			if(mode == 1){//rounding
				rounded = roundto*round(div); //round quotient to nearest int and multiply roundto by result
			}
			else{//truncation
				rounded = roundto*(float)((int)div); //else lop off the decimal and multiply roundto by result
			};
		}
		else{//round is 0, do nothing
			rounded = val;

		};
		return rounded;

}

static void round_list(t_round *x, t_symbol *s,
    int argc, t_atom *argv){
	int i, nearest;
	float curterm, nearfloat, roundto;
	t_atom *outatom;
	nearfloat = x->x_nearest;
	roundto = x->x_round;
	if(nearfloat <= 0.){
		nearest = 0;
	}
	else{
		nearest = 1;
	};
	ATOMS_ALLOCA(outatom, argc); //allocate memory for outatom
	for(i = 0; i<argc; i++){ //get terms one by one and round/truncate them then stick them in outatom
		float rounded;
		curterm = atom_getfloatarg(i, argc, argv);
		rounded = rounding_algo(curterm, roundto, nearest);
		SETFLOAT((outatom+i), (t_float)rounded);
	};
	outlet_list(x->x_obj.ob_outlet, &s_list, argc, outatom);
	ATOMS_FREEA(outatom, argc); //free allocated memory for outatom
	
}

static void round_float(t_round *x, t_float f){
	//since outlet is of type list, make a 1 element list
	float rounded, nearfloat, roundto;
	int nearest;
	t_atom *outatom;
	nearfloat = x->x_nearest;
	roundto = x->x_round;
	if(nearfloat <= 0.){
		nearest = 0;
	}
	else{
		nearest = 1;
	};
	ATOMS_ALLOCA(outatom, 1); //allocate memory of outatom
	rounded = rounding_algo((float)f, roundto, nearest);
	SETFLOAT(outatom, (t_float)rounded);
	outlet_list(x->x_obj.ob_outlet, &s_list, 1, outatom);
	ATOMS_FREEA(outatom, 1); //deallocate memory of outatom
}


static void round_nearest(t_round *x, t_float f, t_float glob){
	if(f <= 0.){
		x->x_nearest = 0.;
	}
	else{
		x->x_nearest = f;
	};
}

void round_setup(void)
{
	round_class = class_new(gensym("round"), (t_newmethod)round_new, 0,
	sizeof(t_round), 0, A_GIMME, 0);
	class_addfloat(round_class, (t_method)round_float);
	class_addlist(round_class, (t_method)round_list);	
	class_addmethod(round_class, (t_method)round_nearest,  gensym("nearest"), A_FLOAT, 0);
}
