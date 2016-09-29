#include <string.h>
#include "m_pd.h"

typedef struct _teeth
{
    t_object  x_obj;
    t_inlet  *x_del_inlet1;
    t_inlet  *x_del_inlet2;
    t_inlet  *x_a_inlet;
    t_inlet  *x_b_inlet;
    t_inlet  *x_c_inlet;
    float     x_sr;
    float     x_ksr;
    t_float  *x_buf;
    int       x_bufsize;   /* as allocated */
    int       x_maxsize;   /* as used */
    float     x_maxdelay;  /* same in ms */
    int       x_phase;     /* writing head */
} t_teeth;

static t_class *teeth_class;

#define TEETH_DEFMAXDELAY  10.0

static void teeth_clear(t_teeth *x)
{
    memset(x->x_buf, 0, x->x_maxsize * sizeof(*x->x_buf));
    x->x_phase = 0;
}

static void teeth_resize(t_teeth *x, int newsize)
{
    if (newsize > 0 && newsize != x->x_maxsize)
    {
	if (newsize > x->x_bufsize)
	{
	    x->x_buf = resizebytes(x->x_buf,
				   x->x_bufsize * sizeof(*x->x_buf),
				   newsize * sizeof(*x->x_buf));
	    /* LATER test for failure */
	    x->x_bufsize = newsize;
	}
	x->x_maxsize = newsize;
    }
    teeth_clear(x);
}

static t_int *teeth_perform(t_int *w)
{
    t_teeth *x = (t_teeth *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float *din1 = (t_float *)(w[4]);
    t_float *din2 = (t_float *)(w[5]);
    t_float *ain = (t_float *)(w[6]);
    t_float *bin = (t_float *)(w[7]);
    t_float *cin = (t_float *)(w[8]);
    t_float *out = (t_float *)(w[9]);
    t_float *buf = x->x_buf;
    int maxsize = x->x_maxsize;
    int guardpoint = maxsize - 1;
    float ksr = x->x_ksr;
    int wph = x->x_phase;
    while (nblock--)
    {  /* TDFII scheme is used.  Do not forget, that any signal value
	  read after writing to out has to be saved beforehand. */
	float xn = *xin++;
	float ff_delsize = ksr * *din1++;
    float fb_delsize = ksr * *din2++;
	float bgain = *bin++;
	float cgain = *cin++;
	float yn = *ain++ * xn; // y = a * in
	float rph;  /* reading head */
	if (ff_delsize >= 0)
	{
	    int ndx;
	    float val;
	    rph = wph - (ff_delsize > guardpoint ? guardpoint : ff_delsize);
	    if (rph < 0) rph += guardpoint;
	    ndx = (int)rph;
	    val = buf[ndx]; // linear interpolation
	    yn += val + (buf[ndx+1] - val) * (rph - ndx);
	}
	*out++ = yn;
	if (wph == guardpoint)
	{
	    buf[wph] = *buf = bgain * xn + cgain * yn;
	    wph = 1;
	}
	else buf[wph++] = bgain * xn + cgain * yn;
    }
    x->x_phase = wph;
    return (w + 10);
}

static void teeth_dsp(t_teeth *x, t_signal **sp)
{
    float sr = sp[0]->s_sr;
    if (sr != x->x_sr)
    {
	x->x_sr = sr;
	x->x_ksr = sr * 0.001;
	teeth_resize(x, x->x_ksr * x->x_maxdelay);
    }
    else teeth_clear(x);
    dsp_add(teeth_perform, 9, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
	    sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec);
}

static void *teeth_new(t_symbol *s, int argc, t_atom *argv)
{
    t_teeth *x;
    float maxdelay = TEETH_DEFMAXDELAY;
    t_float ff_d = 0, fb_d = 0,  a = 0, b = 0,  c = 0;
    int argnum = 0;
    while(argc > 0){
        if(argv -> a_type == A_FLOAT){
            t_float argval = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    maxdelay = argval;
                    break;
                case 1:
                    ff_d = argval;
                    break;
                case 2:
                    fb_d = argval;
                    break;
                case 3:
                    a = argval;
                    break;
                case 4:
                    b = argval;
                    break;
                case 5:
                    c = argval;
                    break;
                default:
                    break;
            };
            argnum++;
            argc--;
            argv++;
        }
    }
    float sr = sys_getsr();
    float ksr = sr * 0.001;
    int bufsize = ksr * maxdelay;
    t_float *buf = (t_float *)getbytes(bufsize * sizeof(*buf));
    if (!buf)
	return (0);
    x = (t_teeth *)pd_new(teeth_class);
    x->x_maxdelay = maxdelay;
    x->x_sr = sr;
    x->x_ksr = ksr;
    x->x_bufsize = x->x_maxsize = bufsize;
    x->x_buf = buf;
    if (ff_d < 0) ff_d = 0;
    if (fb_d < 0) ff_d = 0;
    x->x_del_inlet1 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_del_inlet1, ff_d);
    x->x_del_inlet2 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_del_inlet2, fb_d);
    x->x_a_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_a_inlet, a);
    x->x_b_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_b_inlet, b);
    x->x_c_inlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_c_inlet, c);
    outlet_new((t_object *)x, &s_signal);
    teeth_clear(x);
    return (x);
}

static void teeth_free(t_teeth *x)
{ // porra caralho
    if(x->x_buf) freebytes(x->x_buf, x->x_bufsize * sizeof(*x->x_buf));
}

void teeth_tilde_setup(void)
{
    teeth_class = class_new(gensym("teeth~"),
			   (t_newmethod)teeth_new,
			   (t_method)teeth_free,
			   sizeof(t_teeth), 0,
			   A_GIMME, 0);
    class_addmethod(teeth_class, nullfn, gensym("signal"), 0);
    class_addmethod(teeth_class, (t_method)teeth_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(teeth_class, (t_method)teeth_clear, gensym("clear"), 0);
}
