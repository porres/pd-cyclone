/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */
 
 /* Modified for signal outlet F.J. Kraan 2015 */


/*

 */

#include <math.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/cheapsqrt.h"
#include "sickle/sic.h"
#include "shared.h"

#if defined(_WIN32) || defined(__APPLE__)
/* cf pd/src/x_arithmetic.c */
#define sqrtf  sqrt
#endif

#define AVERAGE_DEF_BUFFER          100  
#define AVERAGE_DEF_SAMPLE_INTERVAL 100
#define AVERAGE_DEFMODE             AVERAGE_BIPOLAR
enum { AVERAGE_BIPOLAR, AVERAGE_ABSOLUTE, AVERAGE_RMS };

typedef struct _average
{
    t_sic     x_sic;
    int       x_mode;
    int       x_bufferSize;
    int       x_sample_interval;
    
    t_float   x_bipolarSum;
    t_float   x_absoluteSum;
    t_float   x_rmsSum;
    
    t_float  *x_buffer;
    int       x_head;
    int       x_tail;
} t_average;

static t_class *average_class;

static void average_status(t_average *x)
{
    post(" ---===### average2 status ###===---");
    post("x_bufferSize: %i",      x->x_bufferSize);
    post("x_sample_interval: %i", x->x_sample_interval);
    post("x_mode: %s", (x->x_mode == 0) ? "bipolar" : (x->x_mode == 1) ? "absolute" : "rms");
    post("x_head: %i",            x->x_head);
    post("x_tail: %i",            x->x_tail);
    post("x_bipolarSum: %f",      x->x_bipolarSum);
    post("x_absoluteSum: %f",     x->x_absoluteSum);
    post("x_rmsSum: %f",          x->x_rmsSum);
    int major, minor, bugfix;
    sys_getversion(&major, &minor, &bugfix);
    post("sys version: %i.%i.%i", major, minor, bugfix);
    post("Cyclone version: %s, %dth %s build",
            CYCLONE_VERSION, CYCLONE_BUILD, CYCLONE_RELEASE);
}

static void average_setmode(t_average *x, int mode)
{
    if (mode != AVERAGE_BIPOLAR && mode != AVERAGE_ABSOLUTE && mode != AVERAGE_RMS)
    {
	loudbug_bug("average2_setmode invalid average mode");
	return;
    }
    x->x_mode = mode;
}

static void average_float(t_average *x, t_float f)
{
    int i = (int)f;  /* CHECKME noninteger */
    if (i > 0)  /* CHECKME */
    {
	x->x_sample_interval = i;
	if (x->x_sample_interval > x->x_bufferSize)
        {
            x->x_sample_interval = x->x_bufferSize;
            loudbug_bug("average_float interval truncated to buffer size");
        }
        x->x_tail = x->x_head - x->x_sample_interval;
        if (x->x_tail < 0) x->x_tail += x->x_bufferSize;
    }
}

static void average_bipolar(t_average *x)
{
    average_setmode(x, AVERAGE_BIPOLAR);
}

static void average_absolute(t_average *x)
{
    average_setmode(x, AVERAGE_ABSOLUTE);
}

static void average_rms(t_average *x)
{
    average_setmode(x, AVERAGE_RMS);
}

static void average_calculateFromScratch(t_average *x)
{
    int i, j;
    x->x_bipolarSum  = 0;
    x->x_absoluteSum = 0;
    x->x_rmsSum      = 0;
    int offset = x->x_tail + 1;
    
    for (i = 0; i < x->x_sample_interval; i++)
    {
        j = (offset + i >= x->x_bufferSize) ? offset + i - x->x_bufferSize : offset + i;
        x->x_bipolarSum  += x->x_buffer[j];
        x->x_absoluteSum += (x->x_buffer[j] > 0) ? x->x_buffer[j] : -x->x_buffer[j];
        x->x_rmsSum      += x->x_buffer[j] * x->x_buffer[j];
    }
}

static t_int *average_perform(t_int *w)
{
    t_average *x = (t_average *)(w[1]);
    int nblock   = (int)(w[2]);
    t_float *in  = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_float tailValue;
    int blockSize = nblock;
    while (nblock--) {
        x->x_buffer[x->x_head] = *in++;
        if (nblock + 1 == blockSize) 
        {
            average_calculateFromScratch(x); 
        } else {
            tailValue         = x->x_buffer[x->x_tail];
            x->x_bipolarSum  += *in;
            x->x_bipolarSum  -= tailValue;
            x->x_absoluteSum += (*in > 0) ? *in : -*in;
            x->x_absoluteSum -= (tailValue > 0) ? tailValue : -tailValue;
            x->x_rmsSum      += *in * *in;
            x->x_rmsSum      -= tailValue * tailValue;
        }
        if (x->x_mode == AVERAGE_BIPOLAR)
            *out++ = x->x_bipolarSum / x->x_sample_interval;
        else if (x->x_mode == AVERAGE_ABSOLUTE)
            *out++ = x->x_absoluteSum / x->x_sample_interval;
        else // AVERAGE_RMS
            *out++ = cheapsqrt(x->x_rmsSum / x->x_sample_interval);
        x->x_head++;
        x->x_tail++;
        if (x->x_head >= x->x_bufferSize)
            x->x_head = 0;
        if (x->x_tail >= x->x_bufferSize)
            x->x_tail = 0;
    }
    return (w + 5);
}

static void average_dsp(t_average *x, t_signal **sp)
{
    
    dsp_add(average_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void average_free(t_average *x)
{
    if (x->x_buffer) 
        freebytes(x->x_buffer, x->x_bufferSize * sizeof(*x->x_buffer));
}

static void *average_new(t_symbol *s, t_floatarg f)
{
    t_average *x = (t_average *)pd_new(average_class);
    int i = (int)f;  /* CHECKME noninteger */
    int mode;
    /* CHECKED it looks like memory is allocated for the entire window,
       in tune with the refman's note about ``maximum averaging interval'' --
       needed for dynamic control over window size, or what? LATER rethink */
    x->x_bufferSize = (i > 0 ?  /* CHECKME */
		    i : AVERAGE_DEF_BUFFER);
    if (s == gensym("bipolar"))
	mode = AVERAGE_BIPOLAR;
    else if (s == gensym("absolute"))
	mode = AVERAGE_ABSOLUTE;
    else if (s == gensym("rms"))
	mode = AVERAGE_RMS;
    else
    {
	mode = AVERAGE_DEFMODE;
	/* CHECKME a warning if (s && s != &s_) */
    }
    average_setmode(x, mode);
    
    post("average2: mode = %s", s->s_name );

    t_float *buffer;
    buffer = (t_float *)getbytes(x->x_bufferSize * sizeof(*buffer));
    int j;
    for (j = 0; j < x->x_bufferSize; j++) {
        buffer[j] = 0;
    }
    x->x_buffer = buffer;
    x->x_head = AVERAGE_DEF_SAMPLE_INTERVAL;
    x->x_tail = 0;
    x->x_sample_interval = AVERAGE_DEF_SAMPLE_INTERVAL;
    
    outlet_new(&x->x_sic, &s_signal);
    return (x);
}

void average2_tilde_setup(void)
{
    average_class = class_new(gensym("average2~"),
			      (t_newmethod)average_new,
			      (t_method)average_free,
			      sizeof(t_average), 0,
			      A_DEFFLOAT, A_DEFSYM, 0);
    sic_setup(average_class, average_dsp, average_float);
    class_addmethod(average_class, (t_method)average_bipolar,
		    gensym("bipolar"), 0);
    class_addmethod(average_class, (t_method)average_absolute,
		    gensym("absolute"), 0);
    class_addmethod(average_class, (t_method)average_rms,
		    gensym("rms"), 0);
    class_addmethod(average_class, (t_method)average_status,
		    gensym("status"), 0);
    int major, minor, bugfix;
    sys_getversion(&major, &minor, &bugfix);
    if (major > 0 || minor > 42) 
        logpost(NULL, 4, "this is cyclone/average2~ %s, %dth %s build",
            CYCLONE_VERSION, CYCLONE_BUILD, CYCLONE_RELEASE);
}
