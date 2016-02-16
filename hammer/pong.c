#include "m_pd.h"
#include <math.h>

static t_class *pong_class;

typedef struct _pong {//pong (control rate) 
	t_object x_obj;
	int mode; //0 = fold, 1 = wrap
	t_float minval;
	t_float maxval;
} t_pong;

static void *pong_new(t_floatarg f1, t_floatarg f2, t_floatarg f3){
	int _mode;
	t_pong *x = (t_pong *)pd_new(pong_class);
	if(f1 <= 0.){
		_mode = 0;
		}
	else{
		_mode = 1;
		};
	x->mode = _mode;
	x->minval = f2;
	x->maxval = f3;

	floatinlet_new(&x->x_obj, &x->minval);
	floatinlet_new(&x->x_obj, &x->maxval);

	outlet_new(&x->x_obj, &s_float);
	return (x);
}

static void pong_float(t_pong *x, t_float f){
	float returnval, minv, maxv, ipt;
	int mode;
	mode = x->mode;
	minv = x->minval;
	maxv = x->maxval;
	ipt = f;

	if(minv > maxv){//checking ranges
		float temp;
		temp = maxv;
		maxv = minv;
		minv = temp;
		};

	if(ipt <= maxv && ipt >= minv){//if ipt in range, return ipt
		returnval = ipt;
		}
	else if(mode == 0){//folding
		float range = maxv - minv;
		if(ipt < minv){
			float diff = minv - ipt; //diff between input and minimum (positive)
			int mag = (int)(diff/range); //case where ipt is more than a range away from minv
			if(mag % 2 == 0){// even number of ranges away = counting up from min
				diff = diff - ((float)mag)*range;
				returnval = diff + minv;
				}
			else{// odd number of ranges away = counting down from max
				diff = diff - ((float)mag)*range;
				returnval = maxv - diff;
				};
			}
		else{ //ipt > maxv
			float diff = ipt - maxv; //diff between input and max (positive)
			int mag  = (int)(diff/range); //case where ipt is more than a range away from maxv
			if(mag % 2 == 0){//even number of ranges away = counting down from max
				diff = diff - (float)mag*range;
				returnval = maxv - diff;
				}
			else{//odd number of ranges away = counting up from min
				diff = diff - (float)mag*range;
				returnval = diff + minv;
				};
			};
		}
	else{//mode = 1, wrapping
		returnval = fmod(ipt-minv,maxv-minv) + minv;
	};


	outlet_float(x->x_obj.ob_outlet, returnval);
}

static void pong_setmode(t_pong *x, t_float f){
	int _mode;
	if(f <= 0.){
		_mode = 0;
		}
	else{
		_mode = 1;
		};
	x->mode = _mode;
}

void pong_setup(void){
	pong_class = class_new(gensym("pong"), (t_newmethod)pong_new, 0,
			sizeof(t_pong), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addfloat(pong_class, pong_float);
	class_addmethod(pong_class, (t_method)pong_setmode, gensym("mode"), A_FLOAT, 0);
}
