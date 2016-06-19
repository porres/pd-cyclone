//alexandre torres porres and derek kwan - 2016

#include "m_pd.h"

#define CSDCFNUM 5   //number of coeffs per filter
#define CSDMAXCF 100 //defining max number of coeffs to take
static t_class *cascade_class;

typedef struct _cascade {
    t_object  x_obj;
    t_inlet  *x_coefflet;
    t_outlet *x_outlet;

	//i don't know if double precision helps with audio stuff
	//but i like to put it like that jist in case - Derek
    double   x_xnm1;
    double   x_xnm2;
    double   x_ynm1;
    double   x_ynm2;
    t_int     x_bypass;
    t_int     x_zero;
	int 	  x_numfilt; //number of biquad filters
	int 	  x_maxfilt; //max number of filters
	double 	  x_coeff[CSDMAXCF]; //array of coeffs
	/* the coeff array is an admittedly easy but cheap way of doing this
	 without malloc/calloc-ing, mb worth revisiting in future - Derek */
} t_cascade;

void *cascade_new(void);

void cascade_clear(t_cascade *x)
{
	int i;
  for(i=0; i<CSDMAXCF; i++){
	x->x_coeff[i] = 0.f;
  };

}

void cascade_bypass(t_cascade *x, t_floatarg f)
{
 x->x_bypass = (int)f;
}

void cascade_zero(t_cascade *x, t_floatarg f)
{
 x->x_zero = (int)f;
}

static void cascade_coeffs(t_cascade *x, t_symbol *s, int argc, t_atom *argv)
{
	//order is a0, a1, a2, b1, b2
	
	//this floors to the nearest mult of CSDCFNUM anything over not 
	//meeting a mult gets ignored 
	int numfilt = (int)(argc/CSDCFNUM);
	if(numfilt > x->x_maxfilt){ 
		// divided amt = max number of filters
		numfilt = x->x_maxfilt;
	};
	x->x_numfilt = numfilt;

	int curfilt = 0;//filter counter
	while(curfilt < numfilt){
		int curidx = CSDCFNUM*curfilt; //current starting index
		t_float a0 = atom_getfloatarg(curidx, argc, argv);
		t_float a1 = atom_getfloatarg(curidx+1, argc, argv);
		t_float a2 = atom_getfloatarg(curidx+2, argc, argv);
		t_float b1 = atom_getfloatarg(curidx+3, argc, argv);
		t_float b2 = atom_getfloatarg(curidx+4, argc, argv);
		x->x_coeff[curidx] = (double)a0;
		x->x_coeff[curidx+1] = (double)a1;
		x->x_coeff[curidx+2] = (double)a2;
		x->x_coeff[curidx+3] = (double)b1;
		x->x_coeff[curidx+4] = (double)b2;
		curfilt++;
	};
  }

static t_int * cascade_perform(t_int *w)
{
   t_cascade *x = (t_cascade *)(w[1]);
  int nblock = (int)(w[2]);
  t_float *in = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
    double xnm1 = x->x_xnm1;
    double xnm2 = x->x_xnm2;
    double ynm1 = x->x_ynm1;
    double ynm2 = x->x_ynm2;
    t_int zero = x->x_zero;
    t_int bypass = x->x_bypass;
	int numfilt = x->x_numfilt;
  while(nblock--)
  {
      if (bypass != 0) *out++ = *in++;
      else if (zero != 0) *out++ = 0;
      else
      {
      double yn;
	  double xn = *in++;
	  double inout = xn;
	  int curfilt = 0; //filter counter
	  while(curfilt < numfilt){
		/*inout gets used as the input of the filter each time so 
		we want to set the output of the current filter to inout as well
		- Derek
		*/

		int curidx = CSDCFNUM*curfilt; //current starting index
		double a0 = x->x_coeff[curidx];
		double a1 = x->x_coeff[curidx+1];
		double a2 = x->x_coeff[curidx+2];
		double b1 = x->x_coeff[curidx+3];
		double b2 = x->x_coeff[curidx+4];
      	inout = a0*inout + a1*xnm1 + a2*xnm2 -b1*ynm1 -b2*ynm2;
		curfilt++;
	  };
      *out++ = yn = inout;
      xnm2 = xnm1;
      xnm1 = xn;
      ynm2 = ynm1;
      ynm1 = yn;
      }
  }
    x->x_xnm1 = xnm1;
    x->x_xnm2 = xnm2;
    x->x_ynm1 = ynm1;
    x->x_ynm2 = ynm2;
  return (w + 5);
}

static void cascade_dsp(t_cascade *x, t_signal **sp)
{
  dsp_add(cascade_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *cascade_free(t_cascade *x){
	inlet_free(x->x_coefflet);
	outlet_free(x->x_outlet);
	return (void *)x;

}

void *cascade_new(void)
{
  t_cascade *x = (t_cascade *)pd_new(cascade_class);
	x->x_coefflet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("coeffs"));
  x->x_outlet = outlet_new(&x->x_obj, &s_signal);
  x->x_xnm1 = x->x_xnm2 = x->x_ynm1 = x->x_ynm2 = 0.f;
  x->x_zero = x->x_bypass = 0;
  x->x_numfilt = 0; //setting number of filters to 0 initially bc no coeffs
  x->x_maxfilt = (int)(CSDMAXCF/CSDCFNUM); //max number of filters
  //zeroing out coeff array 
  int i;
  for(i=0; i<CSDMAXCF; i++){
	x->x_coeff[i] = 0.f;
  };
  return (x);
}

void cascade_tilde_setup(void)
{
    cascade_class = class_new(gensym("cascade~"), (t_newmethod) cascade_new,
            CLASS_DEFAULT, sizeof (t_cascade), CLASS_DEFAULT, 0);
    class_addmethod(cascade_class, nullfn, gensym("signal"), 0);
    class_addmethod(cascade_class, (t_method) cascade_dsp, gensym("dsp"), 0);
    class_addmethod(cascade_class, (t_method) cascade_clear, gensym("clear"), 0);
    class_addmethod(cascade_class, (t_method) cascade_bypass, gensym("bypass"), A_DEFFLOAT, 0);
    class_addmethod(cascade_class, (t_method) cascade_zero, gensym("zero"), A_DEFFLOAT, 0);
   class_addmethod(cascade_class, (t_method)cascade_coeffs, gensym("coeffs"), A_GIMME, 0);
}
