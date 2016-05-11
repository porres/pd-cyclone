/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "shared.h"
#include "common/loud.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

#define BUFFIR_DEFSIZE    0  /* CHECKED */
#define BUFFIR_MAXSIZE  4096

typedef struct _buffir
{
    t_arsic  x_arsic;
    t_pd    *x_offinlet;
    t_pd    *x_sizinlet;
    t_float *x_lohead;
    t_float *x_hihead;
    t_float *x_histlo;
    t_float *x_histhi;
    t_float  x_histbuf[2 * BUFFIR_MAXSIZE];
    int      x_checked;
} t_buffir;

static t_class *buffir_class;

static void buffir_setrange(t_buffir *x, t_floatarg f1, t_floatarg f2)
{
    int off = (int)f1;
    int siz = (int)f2;
    if (off < 0)
	off = 0;
    if (siz <= 0)
	siz = BUFFIR_DEFSIZE;
	if (siz > BUFFIR_MAXSIZE)
	siz = BUFFIR_MAXSIZE;
	pd_float(x->x_offinlet, off);
	pd_float(x->x_sizinlet, siz);
}

static void buffir_clear(t_buffir *x)
{
    memset(x->x_histlo, 0, 2 * BUFFIR_MAXSIZE * sizeof(*x->x_histlo));
    x->x_lohead = x->x_histlo;
    x->x_hihead = x->x_histhi = x->x_histlo + BUFFIR_MAXSIZE;
}

static void buffir_set(t_buffir *x, t_symbol *s, t_floatarg f1, t_floatarg f2)
{
    arsic_setarray((t_arsic *)x, s, 1);
    buffir_setrange(x, f1, f2);
}

static t_int *buffir_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    t_buffir *x = (t_buffir *)sic;
    int nblock = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[6]);
    t_float *lohead = x->x_lohead;
    t_float *hihead = x->x_hihead;
    if (sic->s_playable)
    {	

	t_float *oin = (t_float *)(w[4]);
	t_float *sin = (t_float *)(w[5]);
	int vecsize = sic->s_vecsize;
	t_word *vec = sic->s_vectors[0];  /* playable implies nonzero (mono) */
	while (nblock--)
	{

	    /* CHECKME every sample or once per block.
	       If once per block, then LATER think about performance. */
	    /* CHECKME rounding */
	    int off = (int)*oin++;
	    int npoints = (int)*sin++;
	    if (off < 0)
		off = 0;
	    if (npoints > BUFFIR_MAXSIZE)
		npoints = BUFFIR_MAXSIZE;
	    if (npoints > vecsize - off)
		npoints = vecsize - off;
	    if (npoints > 0)
	    {
	    if (!(x->x_checked))
	    {
	    x->x_checked = 1;
	    }
		t_word *coefp = vec;
		t_float *hp = hihead;
		t_float sum = 0.;
		*lohead++ = *hihead++ = *xin++;
		while (npoints--)
			sum += coefp[off++].w_float * *hp--;
			//sum += coefp->w_float * *hp--;
		*out++ = sum;
	    }
	    else
	    {
		*lohead++ = *hihead++ = *xin++;
		*out++ = 0.;
	    }
	    if (lohead >= x->x_histhi)
	    {
		lohead = x->x_histlo;
		hihead = x->x_histhi;
	    }
	}
    }
    else while (nblock--)
    {
	*lohead++ = *hihead++ = *xin++;
	*out++ = 0.;
	if (lohead >= x->x_histhi)
	{
	    lohead = x->x_histlo;
	    hihead = x->x_histhi;
	}
    }
    x->x_lohead = lohead;
    x->x_hihead = hihead;
    return (w + 7);
}

static void buffir_dsp(t_buffir *x, t_signal **sp)
{
	x->x_checked = 0;
    arsic_dsp((t_arsic *)x, sp, buffir_perform, 1);
}

static void buffir_free(t_buffir *x)
{
    arsic_free((t_arsic *)x);
}

static void *buffir_new(t_symbol *s, t_floatarg f1, t_floatarg f2)
{
    /* CHECKME always the first channel used. */
    /* three auxiliary signals: main, offset and size inputs */
    t_buffir *x = (t_buffir *)arsic_new(buffir_class, s, 0, 0, 3);
    if (x)
    {
	arsic_setminsize((t_arsic *)x, 1);
	x->x_offinlet = (t_pd *)sic_newinlet((t_sic *)x, f1);
	x->x_sizinlet = (t_pd *)sic_newinlet((t_sic *)x, f2);
	outlet_new((t_object *)x, &s_signal);
	x->x_histlo = x->x_histbuf;
	x->x_histhi = x->x_histbuf+BUFFIR_MAXSIZE;
	x->x_checked = 0;
	buffir_clear(x);
	buffir_setrange(x, f1, f2);
    }
    return (x);
}

void buffir_tilde_setup(void)
{
    buffir_class = class_new(gensym("buffir~"), (t_newmethod)buffir_new, (t_method)buffir_free,
                             sizeof(t_buffir), 0, A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
    arsic_setup(buffir_class, buffir_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(buffir_class, (t_method)buffir_clear,
		    gensym("clear"), 0);
    class_addmethod(buffir_class, (t_method)buffir_set,
		    gensym("set"), A_SYMBOL, A_DEFFLOAT, A_DEFFLOAT, 0);
//    logpost(NULL, 4, "this is cyclone/buffir~ %s, %dth %s build",
//	CYCLONE_VERSION, CYCLONE_BUILD, CYCLONE_RELEASE);
}