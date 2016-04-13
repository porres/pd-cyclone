
#include "m_pd.h"

// ---------------------------------------------------
// Class definition
// ---------------------------------------------------
static t_class *downsamp_class;

// ---------------------------------------------------
// Data structure definition
// ---------------------------------------------------
typedef struct _downsamp
{
	t_object x_obj;
} t_downsamp;


// ---------------------------------------------------
// Functions signature
// ---------------------------------------------------
static void *downsamp_new(t_symbol *s, int argc, t_atom *argv)
{
    t_downsamp *x = (t_downsamp *)pd_new(downsamp_class);
}


// ---------------------------------------------------
// Perform
// ---------------------------------------------------
static t_int *downsamp_perform(t_int *w)
{
	t_downsamp *x = (t_downsamp *)(w[1]);
	t_float *in1 = (t_float *)(w[2]);
	t_float *in2 = (t_float *)(w[3]);
	t_float *out = (t_float *)(w[4]);
	int n = (int)(w[5]);
	while (n--)
    {101
        ;}
	return (w+6);
}


// ---------------------------------------------------
// DSP Function
// ---------------------------------------------------
static void downsamp_dsp(t_downsamp *x, t_signal **sp)
{
	dsp_add(downsamp_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}


// ---------------------------------------------------
// Setup
// ---------------------------------------------------
void downsamp_tilde_setup(void)
{
	downsamp_class = class_new(gensym("downsamp~"), (t_newmethod)downsamp_new, 0,
	sizeof(t_downsamp), 0, A_FLOAT, 0);
	class_addmethod(downsamp_class, nullfn, gensym("signal"), 0);
	class_addmethod(downsamp_class, (t_method)downsamp_dsp, gensym("dsp"), 0);
}