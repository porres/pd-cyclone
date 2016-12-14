// based on selector~

#include "m_pd.h"
#include <math.h>
/*MAGIC
 include forky.h for forky_hasfeeders*/
#include "unstable/forky.h"
/*end magic*/

#define GATEOUTPUT 1 //default number of outputs
#define MAXOUTPUT 500 //maximum number of outputs
#define GATESTATE 0 //default state (0 = closed)

typedef struct _gate
{
    t_object   x_obj;
	t_float   *x_mainsig;  // the main signal input
    int 	   x_sig_outs; // outlets ???excluding gate idx (1 indexed)???
	t_float    x_state;    // 0 = closed, nonzero = index of outlet to output (1 indexed)
    t_float   *x_ivec;     // input vector
    t_float  **x_ovecs;    // output vectors
    /*MAGIC
     *x_glist is a list of objects in the canvas
     *x_signalscalar is a pointer to the right inlet's float field, which we're going to poll
     x_badfloat is its value from the last dsp tick
     x_hasfeeders is a flag telling us whether right inlet has feeders*/
    t_glist *x_glist;
    t_float *x_signalscalar;
    t_float x_badfloat;
    int x_hasfeeders;
    /*end magic*/
} t_gate;

static t_class *gate_class;

/*MAGIC
 This is a public function that returns float fields. It is not declared in m_pd.h,
 so we have to declare it here. The arguments are the object and the inlet number
 (indexed from zero)*/
EXTERN t_float *obj_findsignalscalar(t_object *x, int m);
/*end magic*/

static t_int *gate_perform(t_int *w)
{
    t_gate *x = (t_gate *)(w[1]);
    int nblock = (int)(w[2]);
	t_float *state = x->x_mainsig;
    t_float *ivec = x->x_ivec;     // input vector
    t_float **ovecs = x->x_ovecs;  // output vectors
	int i, j;
	int sig_outs = x->x_sig_outs;
    
    /*MAGIC
     here we poll the float in the right inlet's float field to see if it's changed.
     if so, that means there's been a float input and we issue an error. Unfortunately,
     it won't work if you just input the same float over and over...
     */
    t_float scalar = *x->x_signalscalar;
    if (scalar != x->x_badfloat)
    {
        x->x_badfloat = scalar;
        pd_error(x, "inlet: expected 'signal' but got 'float'");
    }
    /*end magic*/
    
	for(i = 0; i < nblock; i++)
    {
        float f;
        /*MAGIC
         self explanatory*/
        if (x->x_hasfeeders) f = *ivec++;
        else f = 0.0;
        /*end magic*/
        
		int curst = (int)state[i];
 //       if (curst > sig_outs){
 //           curst = sig_outs;
        }
        
			for(j = 0; j < sig_outs; j++) // 'for' counter
            { 	/*remember that the signals are counted clockwise
				  so the outlets are labeled BACKWARDS from intuition 
				  */
				int realidx = sig_outs-j-1;
				if(curst == realidx+1) // if index == count
                {
                    ovecs[realidx][i] = ivec[i]; // gate_out == indexed output
				}
				else{
					ovecs[realidx][i] = 0;
				};
			};
	};
    return (w + 3);
}

static void gate_dsp(t_gate *x, t_signal **sp)
{
    /*MAGIC
     Get flag for signal feeders.*/
    x->x_hasfeeders = forky_hasfeeders((t_object *)x, x->x_glist, 1, &s_signal);
    /*end magic*/
    
	int i, nblock = sp[0]->s_n;
    t_signal **sigp = sp;
	x->x_mainsig = (*sigp++)->s_vec; // gate idx
    x->x_ivec = (*sigp++)->s_vec; // now for the signal inlet
    for (i = 0; i < x->x_sig_outs; i++){ //now for the sig_outs
		*(x->x_ovecs+i) = (*sigp++)->s_vec;
	};
	dsp_add(gate_perform, 2, x, nblock);
}

static void *gate_new(t_symbol *s, int argc, t_atom *argv)
{
    t_gate *x = (t_gate *)pd_new(gate_class);
    t_float sig_outs = (t_float) GATEOUTPUT; // outlets
	t_float state = (t_float) GATESTATE; // initially closed
    int i;
    int argnum = 0;
    while(argc > 0){
        if(argv -> a_type == A_FLOAT){
            t_float argval = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
					sig_outs = argval;
                    break;
				case 1:
					state = argval;
					break;
                default:
                    break;
            };
            argc--;
            argv++;
            argnum++;
        };
    };

	//bounds checking
	if(sig_outs < (t_float)GATEOUTPUT){
		sig_outs = GATEOUTPUT;
	}
    else if(sig_outs > (t_float)MAXOUTPUT){
        sig_outs = MAXOUTPUT;
    };
    if(state < 0){
		state = 0;
	}
	else if(state > sig_outs){
		state = sig_outs;
	};

	x->x_sig_outs = (int)sig_outs;
	x->x_state = state; 
	x->x_ovecs = getbytes(sig_outs * sizeof(*x->x_ovecs));

    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
	for (i = 0; i < sig_outs; i++)
    {
	 	outlet_new(&x->x_obj, gensym("signal"));
    };
    /*MAGIC
     1. get the current glist
     2. get a pointer to inlet 1's float field. */
    x->x_glist = canvas_getcurrent();
    x->x_signalscalar = obj_findsignalscalar((t_object *)x, 1);
    /*end magic*/
    return (x);
}

void gate_tilde_setup(void)
{
    gate_class = class_new(gensym("gate~"), (t_newmethod)gate_new, 0,
            sizeof(t_gate), CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(gate_class, (t_method)gate_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(gate_class, t_gate, x_state);
}
