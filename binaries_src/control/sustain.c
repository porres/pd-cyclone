/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"

#define SUSTAIN_NPITCHES  128 // check

typedef struct _sustain
{
    t_object       x_ob;
    t_float        x_velocity;
    int            x_switch;
    int            x_repeatmode;
    unsigned char  x_pitches[SUSTAIN_NPITCHES];
    t_outlet      *x_voutlet;
} t_sustain;

static t_class *sustain_class;

static void sustain_float(t_sustain *x, t_float f)
{
    int pitch = (int)f; // int, really?
    if (pitch >= 0 && pitch < SUSTAIN_NPITCHES) // check
// 0: historical: note-off accumulator
    {
	if (x->x_velocity || !x->x_switch)
	{
	    outlet_float(x->x_voutlet, x->x_velocity);
	    outlet_float(((t_object *)x)->ob_outlet, pitch);
	}
	else x->x_pitches[pitch]++;
    }
// 1: re-trigger
    /* if a note off corresponding to a pitch is already being held and
     another note on message corresponding to that pitch is played again, this mode
     will first send a note off to the previously held noteff message, clearing it out 
     from the memory, and then hold the next corresponding note off message */

// 2: stop-last
    /* in this mode, only one the last held noteoff message is output
     when the sustain is released*/
}

static void sustain_clear(t_sustain *x)
{
    // clear pitch array (derek?)
}

static void sustain_flush(t_sustain *x)
{
    int i;
    unsigned char *pp;
    for (i = 0, pp = x->x_pitches; i < SUSTAIN_NPITCHES; i++, pp++)
    {
	while (*pp)
	{
	    outlet_float(x->x_voutlet, 0);
	    outlet_float(((t_object *)x)->ob_outlet, i);
	    (*pp)--;
	}
    }
}

static void sustain_clear(t_sustain *x)
{
    memset(x->x_pitches, 0, sizeof(x->x_pitches));
}

static void sustain_repeatmode(t_sustain *x, t_floatarg f)
{
    int repeat = (int)f;
    if(repeat < 0) repeat = 0; // check
    if(repeat > 2) repeat = 2; // check
    x->x_repeatmode = repeat;
}

static void sustain_sustain(t_sustain *x, t_floatarg f)
{
    int newstate = ((int)f != 0);
    if (x->x_switch && !newstate) sustain_flush(x);
    x->x_switch = newstate;
}

static void sustain_ft2(t_sustain *x, t_floatarg f)
{
    int newstate = ((int)f != 0);
    if (x->x_switch && !newstate) sustain_flush(x);
    x->x_switch = newstate;
}

static void *sustain_new(void)
{
    t_sustain *x = (t_sustain *)pd_new(sustain_class);
    x->x_velocity = 0;
    x->x_switch = 0;
    x->x_repeatmode = 0;
    sustain_clear(x);
    floatinlet_new((t_object *)x, &x->x_velocity);
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft2"));
    outlet_new((t_object *)x, &s_float);
    x->x_voutlet = outlet_new((t_object *)x, &s_float);
    return (x);
}

void sustain_setup(void)
{
    sustain_class = class_new(gensym("sustain"), (t_newmethod)sustain_new,
			      0,  sizeof(t_sustain), 0, 0);
    class_addfloat(sustain_class, sustain_float);
    class_addmethod(sustain_class, (t_method)sustain_ft2,
		    gensym("ft2"), A_FLOAT, 0);
    class_addmethod(sustain_class, (t_method)sustain_sustain,
                    gensym("sustain"), A_FLOAT, 0);
    class_addmethod(sustain_class, (t_method)sustain_repeatmode,
                    gensym("repeatmode"), A_FLOAT, 0);
    class_addmethod(sustain_class, (t_method)sustain_flush,
                    gensym("flush"), 0);
    class_addmethod(sustain_class, (t_method)sustain_clear,
                    gensym("clear"), 0);
}
