
/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "cybuf.h"

/* CHECKME (the refman): if the buffer~ has more channels, channels are mixed */


typedef struct _play
{
    t_object    x_obj;
    t_cybuf   *x_cybuf;
    t_float     x_ksr; //sample rate in ms
    int 	x_numchans;
    t_float     *x_ivec; // input vector
    t_float     **x_ovecs; //output vectors
} t_play;





static t_class *play_class;

static void play_set(t_play *x, t_symbol *s)
{
    cybuf_setarray(x->x_cybuf, s);
}

////////////////////////////////////////////////
// START
////////////////////////////////////////////////
static void play_start(t_play *x, t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
    //    x->x_? = (int)f;
}


////////////////////////////////////////////////
// float
////////////////////////////////////////////////
static void play_float(t_play *x, t_floatarg f)
{

}



////////////////////////////////////////////////
// STOP
////////////////////////////////////////////////
static void play_stop(t_play *x)
{
//    x->x_nsegs = 0;
}


////////////////////////////////////////////////
// PAUSE
////////////////////////////////////////////////
static void play_pause(t_play *x)
{
 //   x->x_pause = 1;
}


////////////////////////////////////////////////
// RESUME
////////////////////////////////////////////////
static void play_resume(t_play *x)
{
//    x->x_pause = 0;
}


////////////////////////////////////////////////
// RESUME
////////////////////////////////////////////////
static void play_loop(t_play *x, t_floatarg f)
{
    //    x->x_pause = 0;
}


/* LATER optimize */
static t_int *play_perform(t_int *w)
{
    t_play *x = (t_play *)(w[1]);
    t_cybuf *cybuf = x->x_cybuf;
    int nblock = (int)(w[2]);
    int nch = x->x_numchans;
    int chidx; //channel index
    if (cybuf->c_playable)
    {	
	t_float *xin = x->x_ivec;
	int npts = cybuf->c_npts;
	t_word **vectable = cybuf->c_vectors;
	float ksr = x->x_ksr;
	int nointerp = 0;
	int maxindex = (nointerp ? npts - 1 : npts - 3);
	int iblock;

	for (iblock = 0; iblock < nblock; iblock++)
	{
	    float phase = *xin++ * ksr; // converts input in ms to samples!
	    int ndx;
	    int ch = nch;
	    float frac,  a,  b,  c,  d, cminusb;
	    if (phase < 0 || phase > maxindex)
		phase = 0;  /* CHECKED: a value 0, not ndx 0 */
	    ndx = (int)phase;
	    /* CHECKME: what kind of interpolation? (CHECKED: multi-point) */
	    if (ndx < 1)
		ndx = 1, frac = 0;
	    else if (ndx > maxindex)
		ndx = maxindex, frac = 1;
	    else frac = phase - ndx;
	    for(chidx=0; chidx<ch; chidx++)
	    {
		t_word *vp = vectable[chidx];
		t_float *out = *(x->x_ovecs+chidx);
		if (vp)
		{
		    vp += ndx;
		    a = vp[-1].w_float;
		    b = vp[0].w_float;
		    c = vp[1].w_float;
		    d = vp[2].w_float;
		    cminusb = c-b;
		    out[iblock] = b + frac * (
			cminusb - 0.1666667f * (1. - frac) * (
			    (d - a - 3.0f * cminusb) * frac
			    + (d + 2.0f * a - 3.0f * b)
			)
		    );
		}
		else out[iblock] = 0;
	    }
	}
    }
    else
    {
	int ch = nch;
	for(chidx =0; chidx<ch;chidx++)
	{
	    t_float *out = *(x->x_ovecs+chidx);
	    int n = nblock;
	    while (n--) *out++ = 0;
	};
    };
    return (w + 3);
}

static void play_dsp(t_play *x, t_signal **sp)
{
      cybuf_checkdsp(x->x_cybuf); 
    t_float ksr= sp[0]->s_sr * 0.001;

    int i, nblock = sp[0]->s_n;

    t_signal **sigp = sp;
    x->x_ivec = (*sigp++)->s_vec;
    for (i = 0; i < x->x_numchans; i++){ //input vectors first
		*(x->x_ovecs+i) = (*sigp++)->s_vec;
	};
	dsp_add(play_perform, 2, x, nblock);


}

static void *play_free(t_play *x)
{
    cybuf_free(x->x_cybuf);
    freebytes(x->x_ovecs, x->x_numchans * sizeof(*x->x_ovecs));
    return (void *)x;
}

static void *play_new(t_symbol *arrname, t_floatarg f)
{
    /* one auxiliary signal:  position input */
    int chn_n = (int)f > 4 ? 4 : (int)f;
    //channels, if 3 then 2 else channels, nsigs = 0, nauxsigs = 1
    //nperfargs = channels + 1 + 2;
    //t_play *x = (t_play *)arsic_new(play_class, s, chn_n == 3 ? 2 : chn_n, 0, 1);

    t_play *x = (t_play *)pd_new(play_class);
    x->x_ksr = (float)sys_getsr() * 0.001;
    x->x_cybuf = cybuf_init((t_class *)x, arrname, chn_n == 3 ? 2 : chn_n);
    t_cybuf * c = x->x_cybuf;
    
    if (c)
    {
        int nch = c->c_numchans;
        x->x_numchans = nch;
        x->x_ovecs = getbytes(x->x_numchans * sizeof(*x->x_ovecs));
	while (nch--)
	    outlet_new((t_object *)x, &s_signal);
    }
    return (x);
}

void play_tilde_setup(void)
{
    play_class = class_new(gensym("play~"), (t_newmethod)play_new, (t_method)play_free,
			   sizeof(t_play), 0, A_DEFSYM, A_DEFFLOAT, 0);
    class_domainsignalin(play_class, -1);
    class_addfloat(play_class, play_float);
    class_addmethod(play_class, (t_method)play_dsp, gensym("dsp"), 0);
    //class_addmethod(c, (t_method)arsic_enable, gensym("enable"), 0);
    class_addmethod(play_class, (t_method)play_set, gensym("set"), A_SYMBOL, 0);
    class_addmethod(play_class, (t_method)play_stop, gensym("stop"), 0);
    class_addmethod(play_class, (t_method)play_pause, gensym("pause"), 0);
    class_addmethod(play_class, (t_method)play_resume, gensym("resume"), 0);
    class_addmethod(play_class, (t_method)play_loop, gensym("loop"), A_FLOAT, 0);
    class_addmethod(play_class, (t_method)play_start, gensym("start"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
}
