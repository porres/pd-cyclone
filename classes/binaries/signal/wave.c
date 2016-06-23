/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

/* CHECKME (the refman): the extra channels are not played */

typedef struct _wave
{
    t_arsic           x_arsic;
    int               x_bufsize;
    int               x_interp_mode;
    t_float	          x_bias;
    t_float           x_tension;
} t_wave;

static t_class *wave_class;





/* interpolation functions w/ jump table -- code was cleaner than a massive
switch - case block in the wave_perform() routine; might be very slightly
less efficient this way but I think the clarity was worth it.

The Max/MSP wave~ class seems to copy all of its interpolation functions
directly from http://paulbourke.net/miscellaneous/interpolation/ without
attribution. Should we find a way to attribute? Note that "cubic" is not 
the same interpolator as in tabread4~. For convenience, I've added that
interpolator as wave_lagrange().
  -- Matt Barber */
  


static void wave_interp_bias(t_wave *x, t_floatarg f)
{
	x->x_bias = f;
	return;
}

static void wave_interp_tension(t_wave *x, t_floatarg f)
{
	x->x_tension = f;
	return;
}


static void wave_interp(t_wave *x, t_floatarg f)
{
	
	x->x_interp_mode = f < 0 ? 0 : f;
	x->x_interp_mode = x->x_interp_mode > 7 ? 7 : x->x_interp_mode;
    arsic_setminsize((t_arsic *)x, (x->x_interp_mode >= 4 ? 4 : 1));
    arsic_check((t_arsic *)x);
}

static void wave_set(t_wave *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
}


/* interpolation functions w/ jump table -- code was cleaner than a massive
switch - case block in the wave_perform() routine; might be very slightly
less efficient this way but I think the clarity was worth it.

The Max/MSP wave~ class seems to copy all of its interpolation functions
directly from http://paulbourke.net/miscellaneous/interpolation/ without
attribution. Should we find a way to attribute? Note that "cubic" is not 
the same interpolator as in tabread4~. For convenience, I've added that
interpolator as wave_lagrange().
  -- Matt Barber */
  
/*stupid hacks; sorry. This saves a lot of typing.*/
#define BOUNDS_CHECK() \
	if (spos < 0) spos = 0; \
	else if (spos > maxindex) spos = maxindex; \
	if (epos > maxindex || epos <= 0) epos = maxindex; \
	else if (epos < spos) epos = spos; \
	int siz = (int)(epos - spos + 1.5); \
	int ndx; \
	int ch = nch; \
	if (phase < 0) phase = 0; \
	else if (phase > 1.0) phase = 0; \
	int sposi = (int)spos; \
	int eposi = sposi + siz
	
	
#define INDEX_2PT(TYPE) \
	int ndx1; \
	TYPE a, b; \
	TYPE xpos = phase*siz + spos; \
	ndx = (int)xpos; \
	TYPE frac = xpos - ndx; \
	if (ndx == eposi) ndx = sposi; \
	ndx1 = ndx + 1; \
	if (ndx1 == eposi) ndx1 = sposi
	
#define INDEX_4PT() \ 
	int ndxm1, ndx1, ndx2; \
	double a, b, c, d; \
	double xpos = phase*siz + spos; \
	ndx = (int)xpos; \
	double frac = xpos - ndx; \
	if (ndx == eposi) ndx = sposi; \
	ndxm1 = ndx - 1; \
	if (ndxm1 < sposi) ndxm1 = eposi - 1; \
	ndx1 = ndx + 1; \
	if (ndx1 == eposi) ndx1 = sposi; \
	ndx2 = ndx1 + 1; \
	if (ndx2 == eposi) ndx2 = sposi;


static void wave_nointerp(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		t_float phase = *xin++;
		t_float spos = *sin++ * ksr;
		t_float epos = *ein++ * ksr;
		BOUNDS_CHECK();
		ndx = (int)(phase*siz + spos);
		ndx = (ndx >= eposi ? sposi : ndx);
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			out[iblock] = (vp ? vp[ndx].w_float : 0);
		}	
	}
	return;
}

static void wave_linear(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		double phase = (double)(*xin++);
		double spos = (double)(*sin++) * ksr;
		double epos = (double)(*ein++) * ksr;
		BOUNDS_CHECK();
		INDEX_2PT(double);
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			if (vp)
			{
				a = (double)vp[ndx].w_float;
				b = (double)vp[ndx1].w_float;
				out[iblock] = (t_float)(a * (1.0 - frac) + b * frac);
			}
			else out[iblock] = 0;
		}
	}
	return;
}

static void wave_linlq(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		t_float phase = *xin++;
		t_float spos = *sin++ * ksr;
		t_float epos = *ein++ * ksr;
		BOUNDS_CHECK();
		INDEX_2PT(float);
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			if (vp)
			{
				a = vp[ndx].w_float;
				b = vp[ndx1].w_float;
				out[iblock] = a + frac * (b - a);
			}
			else out[iblock] = 0;
		}
	}
	return;
}

static void wave_cosine(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		t_float phase = *xin++;
		t_float spos = *sin++ * ksr;
		t_float epos = *ein++ * ksr;
		BOUNDS_CHECK();
		INDEX_2PT(double);
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			if (vp)
			{
				a = (double)vp[ndx].w_float;
				b = (double)vp[ndx1].w_float;
				frac = (1 - cos(frac * M_PI)) / 2.0;
				out[iblock] = (t_float)(a * (1 - frac) + b * (frac));
			}
			else out[iblock] = 0;
		}
		
	}
	return;
}

static void wave_cubic(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		t_float phase = *xin++;
		t_float spos = *sin++ * ksr;
		t_float epos = *ein++ * ksr;
		BOUNDS_CHECK();
		INDEX_4PT();
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			if (vp)
			{
				a = (double)vp[ndxm1].w_float;
				b = (double)vp[ndx].w_float;
				c = (double)vp[ndx1].w_float;
				d = (double)vp[ndx2].w_float;
				double p0, p1, p2;
				p0 = d - a + b - c;
				p1 = a - b - p0;
				p2 = c - a;
				out[iblock] = (t_float)(b+frac*(p2+frac*(p1+frac*p0)));	
			}
			else out[iblock] = 0;
		}
		
	}
	return;
}

static void wave_spline(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		t_float phase = *xin++;
		t_float spos = *sin++ * ksr;
		t_float epos = *ein++ * ksr;
		BOUNDS_CHECK();
		INDEX_4PT();
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			if (vp)
			{
				a = (double)vp[ndxm1].w_float;
				b = (double)vp[ndx].w_float;
				c = (double)vp[ndx1].w_float;
				d = (double)vp[ndx2].w_float;
				double p0, p1, p2;
				p0 = 0.5*(d - a) + 1.5*(b - c);
				p2 = 0.5*(c - a);
				p1 = a - b + p2 - p0;
				out[iblock] = (t_float)(b+frac*(p2+frac*(p1+frac*p0)));	
			}
			else out[iblock] = 0;
		}
		
	}
	return;
}

static void wave_hermite(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		t_float phase = *xin++;
		t_float spos = *sin++ * ksr;
		t_float epos = *ein++ * ksr;
		BOUNDS_CHECK();
		INDEX_4PT();
		double tension = (double)x->x_tension;
		double bias = (double)x->x_bias;
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			if (vp)
			{
				a = (double)vp[ndxm1].w_float;
				b = (double)vp[ndx].w_float;
				c = (double)vp[ndx1].w_float;
				d = (double)vp[ndx2].w_float;
				double p0, p1, p2, p3, m0, m1;
				double frac2 = frac*frac;
				double frac3 = frac*frac2;
				double cminusb = c - b;
				double bias1 = 1. - bias;
				bias++;
				tension = 0.5 * (1. - tension);
				m0 = tension * ((b-a)*bias + cminusb*bias1);
				m1 = tension * (cminusb*bias + (d-c)*bias1);
				p2 = frac3 - frac2;
				p0 = 2*p2 - frac2 + 1.;
				p1 = p2 - frac2 + frac;
				p3 = frac2 - 2*p2;
				out[iblock] = (t_float)(p0*b + p1*m0 + p2*m1 + p3*c);	
			}
			else out[iblock] = 0;
		}
		
	}
	return;
}

static void wave_lagrange(t_wave *x,
	t_int *outp, t_float *xin, t_float *sin, t_float *ein,
	int nblock, int nch, int maxindex, float ksr, t_word **vectable)
{
	int iblock;
	for (iblock = 0; iblock < nblock; iblock++)
	{
		t_float phase = *xin++;
		t_float spos = *sin++ * ksr;
		t_float epos = *ein++ * ksr;
		BOUNDS_CHECK();
		INDEX_4PT();
		while (ch--)
		{
			t_word *vp = vectable[ch];
			t_float *out = (t_float *)(outp[ch]);
			if (vp)
			{
				a = (double)vp[ndxm1].w_float;
				b = (double)vp[ndx].w_float;
				c = (double)vp[ndx1].w_float;
				d = (double)vp[ndx2].w_float;
				double cminusb = c-b;
				out[iblock] = (t_float)(b + frac * (
					cminusb - (1. - frac)/6. * (
						(d - a - 3.0*cminusb) * frac + d + 2.0*a - 3.0*b
					)
				));	
			}
			else out[iblock] = 0;
		}
		
	}
	return;
}

static t_int *wave_perform(t_int *w)
{
    /* The jump table; "wif" = "wave interpolation functions." */
	
	static void (* const wif[])(t_wave *x,
		t_int *outp, t_float *xin, t_float *sin, t_float *ein,
		int nblock, int nch, int maxindex, float ksr, t_word **vectable) = 
	{
   		wave_nointerp,
    	wave_linear,
    	wave_linlq,
    	wave_cosine,
    	wave_cubic,
    	wave_spline,
    	wave_hermite,
    	wave_lagrange
    };
    
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    t_int *outp = w + 6;
    int nch = sic->s_nchannels;
    if (sic->s_playable)
    {	
		t_wave *x = (t_wave *)sic;
		int vecsize = sic->s_vecsize;
		float ksr = sic->s_ksr;
		t_word **vectable = sic->s_vectors;
		t_float *xin = (t_float *)(w[3]);
		t_float *sin = (t_float *)(w[4]);
		t_float *ein = (t_float *)(w[5]);
		int maxindex = sic->s_vecsize - 1;
		int interp_mode = x->x_interp_mode;
		/*Choose interpolation function from jump table. The interpolation functions also
		  perform the block loop in order not to make a bunch of per-sample decisions.*/	
		wif[interp_mode](x, outp, xin, sin, ein, nblock, nch, maxindex, ksr, vectable);
    }
    else
    {
	int ch = nch;
	while (ch--)
	{
	    t_float *out = (t_float *)outp[ch];
	    int n = nblock;
	    while (n--) *out++ = 0;
	}
    }
    return (w + sic->s_nperfargs + 1);
}

static void wave_dsp(t_wave *x, t_signal **sp)
{
    arsic_dsp((t_arsic *)x, sp, wave_perform, 1);
}

static void wave_free(t_wave *x)
{
    arsic_free((t_arsic *)x);
}

static void *wave_new(t_symbol *s, t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
    /* three auxiliary signals:  phase, clipstart, and clipend inputs */
    t_wave *x = (t_wave *)arsic_new(wave_class, s, (int)f3, 0, 3);
    if (x)
    {
	int nch = arsic_getnchannels((t_arsic *)x);
	nch = nch > 4 ? 4 : nch;
	if (f1 < 0) f1 = 0;
	if (f2 < 0) f2 = 0;
	sic_newinlet((t_sic *)x, f1);
	sic_newinlet((t_sic *)x, f2);
	while (nch--)
	    outlet_new((t_object *)x, &s_signal);
	wave_interp(x, 1);
	x->x_bias = 0;
	x->x_tension = 0;
    }
    return (x);
}

void wave_tilde_setup(void)
{
    wave_class = class_new(gensym("wave~"),
			   (t_newmethod)wave_new,
			   (t_method)wave_free,
			   sizeof(t_wave), 0,
			   A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    arsic_setup(wave_class, wave_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(wave_class, (t_method)wave_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(wave_class, (t_method)wave_interp,
		    gensym("interp"), A_FLOAT, 0);
	class_addmethod(wave_class, (t_method)wave_interp_bias,
			gensym("interp_bias"), A_FLOAT, 0);
	class_addmethod(wave_class, (t_method)wave_interp_tension,
			gensym("interp_tension"), A_FLOAT, 0);
//    logpost(NULL, 4, "this is cyclone/wave~ %s, %dth %s build",
//	 CYCLONE_VERSION, CYCLONE_BUILD, CYCLONE_RELEASE);
}
