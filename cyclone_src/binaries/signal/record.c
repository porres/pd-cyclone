 /* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Derek Kwan 2016
adding defaults for numchannels, append, loopstatus, loopstart, loopend
adding record_getarraysmp
adding attribute parsing and making numchannels arg optional 
*/

/* Porres 2016 fixed bugs with sync output, append mode, reset message, max number channels in  */
 
#include <string.h>
#include "m_pd.h"
#include "m_imp.h"
#include "shared.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

#define PDCYREC_NCH 1
#define PDCYREC_APPEND 0
#define PDCYREC_LOOPSTATUS 0
#define PDCYREC_LOOPSTART 0

#define RECORD_REDRAWPAUSE  1000.  /* refractory period */

typedef struct _record
{
    t_arsic   x_arsic;
    float     x_array_ms;  // array size in ms
    float     x_startpoint;  /* the inputs */
    float     x_endpoint;
    int       x_appendmode;
    int       x_loopmode;
    int       x_startindex;
    int       x_endindex;    /* (one past last record position) */
    int       x_pauseindex;
    int       x_phase;       /* writing head */
    float     x_sync;
    int       x_isrunning;   /* to know if sync should be 0.0 or 1.0 */
    t_clock  *x_clock;
    double    x_clocklasttick;

	int 	  x_numchan;
} t_record;

static t_class *record_class;


static t_float record_getarraysmp(t_record *x, t_symbol *arrayname){
  t_garray *garray;
	char bufname[MAXPDSTRING];
	t_float retsmp = -1;
	int numchan = x->x_numchan;
	if(numchan > 1){
		sprintf(bufname, "0-%s", arrayname->s_name);
	}
	else{
		sprintf(bufname, "%s", arrayname->s_name);
	};
  if(!(garray = (t_garray *)pd_findbyclass(arrayname,garray_class))) {
    pd_error(x, "%s: no such table", bufname);
  } else {
   	retsmp = garray_npoints(garray);
  };
	return retsmp;
}


static void record_tick(t_record *x)
{
    double timesince = clock_gettimesince(x->x_clocklasttick);
    if (timesince >= RECORD_REDRAWPAUSE)
    {
	arsic_redraw((t_arsic *)x);
	x->x_clocklasttick = clock_getlogicaltime();
    }
    else clock_delay(x->x_clock, RECORD_REDRAWPAUSE - timesince);
}

static void record_setsync(t_record *x)
{
    /* CHECKED: clipped to array size -- using indices, not points */
    float range = (float)(x->x_endindex - x->x_startindex);
    int phase = x->x_phase;
    if (phase == SHARED_INT_MAX || range < 1.)
    {
    x->x_sync = x->x_isrunning;  /* CHECKED */
    }
    else
    {
	x->x_sync = (float)(phase - x->x_startindex) / range;
    }
}

static void record_mstoindex(t_record *x)
{
    t_arsic *sic = (t_arsic *)x;
    x->x_startindex = (int)(x->x_startpoint * sic->s_ksr);
    if (x->x_startindex < 0){
		x->x_startindex = 0;  /* CHECKED */
	};
    x->x_endindex = (int)(x->x_endpoint * sic->s_ksr);
    if (x->x_endindex > sic->s_vecsize
	|| x->x_endindex <= 0){
		x->x_endindex = sic->s_vecsize;  /* CHECKED (both ways) */
	};
    record_setsync(x);
}

static void record_set(t_record *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
    record_mstoindex(x);
}

static void record_reset(t_record *x) // new
{
    x->x_startpoint = 0.;
    x->x_endpoint = x->x_array_ms;
    if (x->x_isrunning) x->x_phase = 0.;
    record_mstoindex(x);
} // */

static void record_startpoint(t_record *x, t_floatarg f)
{
    x->x_startpoint = f;
    record_mstoindex(x);
}

static void record_endpoint(t_record *x, t_floatarg f)
{
    x->x_endpoint = f;
    record_mstoindex(x);
}

static void record_float(t_record *x, t_float f)
{
    x->x_isrunning = (f != 0);
    if (x->x_isrunning)
    {
	/* CHECKED: no (re)start in append mode */
	/* LATER consider restart if x->x_pauseindex == SHARED_INT_MAX */
	x->x_phase = x->x_appendmode ? x->x_pauseindex : x->x_startindex;
        if (x->x_phase >= x->x_endindex) x->x_phase = SHARED_INT_MAX;
    }
    else if (x->x_phase != SHARED_INT_MAX)
    {
	clock_delay(x->x_clock, 10.);
	x->x_pauseindex = x->x_phase;
	x->x_phase = SHARED_INT_MAX;
    }
    record_setsync(x);
}

static void record_append(t_record *x, t_floatarg f)
{
    if (f != 0)
    {
	x->x_appendmode = 1;  /* CHECKED: always allow appending */
    }
    else x->x_appendmode = 0;
}

static void record_loop(t_record *x, t_floatarg f)
{
    x->x_loopmode = (f != 0);
}

static t_int *record_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    int nch = sic->s_nchannels;
    t_float *out = (t_float *)(w[3 + nch]);
    t_record *x = (t_record *)sic;
    int phase = x->x_phase;
    float endphase = x->x_endindex;
    float startphase = x->x_startindex;
    float phase_range = (float)(endphase - startphase);
    float sync = x->x_sync;
    if (sic->s_playable && endphase > phase)
    {
	int vecsize = sic->s_vecsize;
	int ch, over, i, nxfer, ndone = 0;
loopover:
	if ((nxfer = endphase - phase) > nblock)
	{
	    nxfer = nblock;
	    over = 0;
	}
	else over = 1;
	ch = nch;
	while (ch--)
	{
	    t_word *vp = sic->s_vectors[ch];
	    if (vp)
	    {
		t_float *ip = (t_float *)(w[3 + ch]) + ndone;
		vp += phase;
		i = nxfer;
//		while (i--) *vp++ = *ip++; /* LATER consider handling under and overflows */
		int j = 0;
		while (i--) 
            {vp[j].w_float = ip[j];
                j++;}
	    }
	}
	i = nxfer;
        
    sync = (float)(phase - startphase) / phase_range;
	
    while (i--)
        {
        *out++ = sync;
        }
        
	if (over)
	{
	    clock_delay(x->x_clock, 0);
	    nblock -= nxfer;
	    if (x->x_loopmode
		&& (phase = x->x_startindex) < endphase)
	    {
		x->x_phase = phase;
		x->x_sync = sync = 0.;
		if (nblock > 0)
		{
		    ndone += nxfer;
		    goto loopover;
		}
		goto alldone;
	    }
        x->x_pauseindex = SHARED_INT_MAX;   /* CHECKED: no restart in append mode */
	    x->x_phase = SHARED_INT_MAX;
	    x->x_sync = 1.;
	}
	else
	{
	    x->x_phase += nxfer;
	    x->x_sync = sync;
	    goto alldone;
	}
    }
    while (nblock--) *out++ = sync; // sync output = 0
alldone:
    return (w + sic->s_nperfargs + 1);
}

static void record_dsp(t_record *x, t_signal **sp)
{
    arsic_dsp((t_arsic *)x, sp, record_perform, 1);
    record_mstoindex(x);
}

static void record_free(t_record *x)
{
    arsic_free((t_arsic *)x);
    if (x->x_clock) clock_free(x->x_clock);
}


static void *record_new(t_symbol *s, int argc, t_atom *argv)
{
	t_float numchan = PDCYREC_NCH;
	t_float append = PDCYREC_APPEND;
	t_float loopstatus = PDCYREC_LOOPSTATUS;
	t_float loopstart = PDCYREC_LOOPSTART;
	t_float loopend = -1;
	
	t_symbol * arrname = gensym("record_def");
	if(argc > 0 && argv ->a_type == A_SYMBOL){
		//first arg HAS to be array name, parse it now
		arrname = atom_getsymbolarg(0, argc, argv);
		argc--;
		argv++;
		//post("record~ setting to '%s'", arrname->s_name);
	}
	else{
	// else default to dummy name but warn
		post("defaulting to name '%s'", arrname->s_name);
	};
	
	//NOW parse the rest of the args
	int argnum = 0;
	while(argc > 0){
		if(argv->a_type == A_SYMBOL){
			t_symbol * curarg = atom_getsymbolarg(0, argc, argv);
			if(strcmp(curarg->s_name, "@append")==0){
				if(argc >= 2){
					append = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@loop")==0){
				if(argc >= 2){
					loopstatus = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@loopstart")==0){
				if(argc >= 2){
					loopstart = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@loopend")==0){
				if(argc >= 2){
					loopend = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else{
				goto errstate;
			};
		}
		else if(argv->a_type == A_FLOAT){
			t_float argval = atom_getfloatarg(0, argc, argv);
			switch(argnum){
				case 0:
					numchan = argval;
					break;
				default:
					break;
			};
			argnum++;
			argc--;
			argv++;
		}
		else{
			goto errstate;
		};
	};
    int chn_n = (int)numchan > 4 ? 4 : (int)numchan;
	if(chn_n == 3){
		chn_n = 2;
	};
    t_record *x = (t_record *)arsic_new(record_class, arrname, chn_n, 0, 1);
    if (x)
    {
	
	int nch = arsic_getnchannels((t_arsic *)x);
	x->x_numchan = nch;
	t_float arraysmp = record_getarraysmp(x, arrname);
	if(loopend < 0 && arraysmp > 0){
//if loopend not set or less than 0 and arraysmp doesn't fail, set it to arraylen in ms
       loopend = (arraysmp/(sys_getsr()*0.001));
	};
	arsic_setminsize((t_arsic *)x, 2);
	record_append(x, append);	
	record_loop(x, loopstatus);
    
    x->x_phase = SHARED_INT_MAX; // instead of old record_reset(x) and so it doesn't record when DSP is on (stupid code);
    x->x_pauseindex = 0; // instead of old record_reset(x)'s SHARED_INT_MAX so append works
	x->x_clock = clock_new(x, (t_method)record_tick);
	x->x_clocklasttick = clock_getlogicaltime();
    x->x_array_ms = loopend;
	while (--nch){
	    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
	};
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft-2"));
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft-1"));
	outlet_new((t_object *)x, &s_signal); //  sync output
    
	record_startpoint(x, loopstart);
	record_endpoint(x, loopend);
    }
    return (x);
	errstate:
		post("record~: improper args");
		return NULL;
}

void record_tilde_setup(void)
{
    record_class = class_new(gensym("record~"),
			     (t_newmethod)record_new,
			     (t_method)record_free,
			     sizeof(t_record), 0,
			     A_GIMME, 0);
    arsic_setup(record_class, record_dsp, record_float);
    class_addmethod(record_class, (t_method)record_startpoint,
		    gensym("ft-2"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_endpoint,
		    gensym("ft-1"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_append,
		    gensym("append"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_loop,
		    gensym("loop"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(record_class, (t_method)record_reset,
		    gensym("reset"), 0);
//    logpost(NULL, 4, "this is cyclone/record~ %s, %dth %s build",
//	 CYCLONE_VERSION, CYCLONE_BUILD, CYCLONE_RELEASE);
}
