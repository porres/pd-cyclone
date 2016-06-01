// based on selector~

#include "m_pd.h"
#include <math.h>

#define GATEOUTPUT 1 //default number of outputs
#define GATESTATE 0 //default state (0 = closed)

typedef struct _gate
{
    t_object   x_obj;
	t_float   *x_mainsig; //the main signal being piped in
    int 	   x_sigputs; //inlets excluding gate idx (1 indexed)
	t_float    x_state;   //0 = closed, nonzero = index of inlet to pass (1 indexed)
    t_float  **x_ivecs;   // copying from matrix
    t_float   *x_ovec;    // only should be single pointer since we're dealing with an array
						  //rather than an array of arrays
} t_gate;

static t_class *gate_class;

static t_int *gate_perform(t_int *w)
{
    t_gate *x = (t_gate *)(w[1]);
    int nblock = (int)(w[2]);
	
	t_float *state = x->x_mainsig;
    t_float **ivecs = x->x_ivecs;
    t_float *ovec = x->x_ovec;

	int i,j;

	int sigputs = x->x_sigputs;

	for(i=0; i< nblock; i++){
		int curst = (int)state[i];
		t_float output = 0;
		if(curst != 0){
			for(j=0; j<sigputs;j++){
				if(curst == (j+1)){
					output = ivecs[j][i];
				};
			};
		};
		ovec[i] = output;
	};
    return (w + 3);
}


static void gate_dsp(t_gate *x, t_signal **sp)
{
	int i, nblock = sp[0]->s_n;
    t_signal **sigp = sp;
	x->x_mainsig = (*sigp++)->s_vec; //the first sig in is the gate idx
    for (i = 0; i < x->x_sigputs; i++){ //now for the sigputs
		*(x->x_ivecs+i) = (*sigp++)->s_vec;
	};
	x->x_ovec = (*sigp++)->s_vec; //now for the outlet
	dsp_add(gate_perform, 2, x, nblock);
}

static void *gate_new(t_symbol *s, int argc, t_atom *argv)
{
    t_gate *x = (t_gate *)pd_new(gate_class);
    t_float sigputs = (t_float) GATEOUTPUT; //inlets not counting gate input
	t_float state = (t_float) GATESTATE; //start off closed initially
    int i;
    int argnum = 0;
    while(argc > 0){
        if(argv -> a_type == A_FLOAT){
            t_float argval = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
					sigputs = argval;
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
	if(sigputs < (t_float)GATEOUTPUT){
		sigputs = GATEOUTPUT;
	};
	if(state < 0){
		state = 0;
	}
	else if(state > sigputs){
		state = sigputs;
	};

	x->x_sigputs = (int)sigputs;
	x->x_state = state; 
	x->x_ivecs = getbytes(sigputs * sizeof(*x->x_ivecs));
    
	for (i = 0; i < sigputs; i++){
        pd_float((t_pd *)inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal), 0.);
    };
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void gate_tilde_setup(void)
{
    gate_class = class_new(gensym("gate~"), (t_newmethod)gate_new, 0,
            sizeof(t_gate), CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(gate_class, (t_method)gate_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(gate_class, t_gate, x_state);
}
