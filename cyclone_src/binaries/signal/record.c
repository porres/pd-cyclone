 /* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Derek Kwan 2016
adding defaults for numchannels, append, loopstatus, loopstart, loopend
adding record_getarraysmp
adding attribute parsing and making numchannels arg optional 
de-arsic/cybuffing
*/

/* Porres 2016 fixed bugs with sync output, append mode, reset message, max number channels in  */
 
#include <string.h>
#include "m_pd.h"
#include "m_imp.h"
#include "shared.h"
#include "cybuf.h"

#define PDCYREC_NCH 1
#define PDCYREC_APPEND 0
#define PDCYREC_LOOPSTATUS 0
#define PDCYREC_LOOPSTART 0

#define RECORD_REDRAWPAUSE  1000.  /* refractory period */

typedef struct _record
{
    t_object    x_obj;
    t_cybuf   *x_cybuf;
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


    t_inlet     *x_stlet; //start inlet
    t_inlet     *x_endlet; //end inlet
    t_outlet    *x_outlet;

    t_float     x_ksr; //sample rate in ms
    int 	x_numchans;
    t_float     **x_ivecs; // input vectors
    t_float     *x_ovec; //output vector
} t_record;

static t_class *record_class;

/*
kept here for legacy, now built into cybuf - DK
static t_float record_getarraysmp(t_record *x, t_symbol *arrayname){
  t_garray *garray;
  t_symbol *checkname; //name to check
	char bufname[MAXPDSTRING];
	t_float retsmp = -1;
	int numchan = x->x_numchans;
	if(numchan > 1){
		sprintf(bufname, "0-%s", arrayname->s_name);
                checkname = gensym(bufname);
	}
	else{
		sprintf(bufname, "%s", arrayname->s_name);
                checkname = arrayname;
	};
  if(!(garray = (t_garray *)pd_findbyclass(checkname,garray_class))) {
    pd_error(x, "%s: no such table", bufname);
  } else {
   	retsmp = garray_npoints(garray);
  };
	return retsmp;
}
*/

static void record_tick(t_record *x)
{
    double timesince = clock_gettimesince(x->x_clocklasttick);
    if (timesince >= RECORD_REDRAWPAUSE)
    {
	cybuf_redraw(x->x_cybuf);
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
    t_cybuf * c = x->x_cybuf;
    x->x_startindex = (int)(x->x_startpoint * x->x_ksr);
    if (x->x_startindex < 0){
		x->x_startindex = 0;  /* CHECKED */
	};
    x->x_endindex = (int)(x->x_endpoint * x->x_ksr);
    if (x->x_endindex > c->c_npts
	|| x->x_endindex <= 0){
		x->x_endindex = c->c_npts;  /* CHECKED (both ways) */
	};
    record_setsync(x);
}

static void record_set(t_record *x, t_symbol *s)
{
    cybuf_setarray(x->x_cybuf, s );
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
    t_record *x = (t_record *)(w[1]);
    t_cybuf * c = x->x_cybuf;
    int nch = c->c_numchans;

    int nblock = (int)(w[2]);
    t_float *out = x->x_ovec;
    int phase = x->x_phase;
    float endphase = x->x_endindex;
    float startphase = x->x_startindex;
    float phase_range = (float)(endphase - startphase);
    float sync = x->x_sync;
    if (c->c_playable && endphase > phase)
    {
	//int vecsize = c->c_npts;
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
	    t_word *vp = c->c_vectors[ch];
	    if (vp)
	    {
		t_float *ip = *(x->x_ivecs + ch +  ndone);
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
    return (w + 3);
}

static void record_dsp(t_record *x, t_signal **sp)
{
    cybuf_checkdsp(x->x_cybuf); 
    t_float ksr= sp[0]->s_sr * 0.001;

        record_mstoindex(x);
    int i, nblock = sp[0]->s_n;

    t_signal **sigp = sp;
    for (i = 0; i < x->x_numchans; i++){ //input vectors first
		*(x->x_ivecs+i) = (*sigp++)->s_vec;
	};
        x->x_ovec = (*sigp++)->s_vec;
	dsp_add(record_perform, 2, x, nblock);


}

static void record_free(t_record *x)
{
    cybuf_free(x->x_cybuf);
    inlet_free(x->x_stlet);
    inlet_free(x->x_endlet);
    outlet_free(x->x_outlet);
    freebytes(x->x_ivecs, x->x_numchans * sizeof(*x->x_ivecs));
    if (x->x_clock) clock_free(x->x_clock);
}


static void *record_new(t_symbol *s, int argc, t_atom *argv)
{
        int i;
	t_float numchan = PDCYREC_NCH;
	t_float append = PDCYREC_APPEND;
	t_float loopstatus = PDCYREC_LOOPSTATUS;
	t_float loopstart = PDCYREC_LOOPSTART;
	t_float loopend = -1;
	
	t_symbol * arrname = &s_;
	if(argc > 0 && argv ->a_type == A_SYMBOL){
		//first arg HAS to be array name, parse it now
		arrname = atom_getsymbolarg(0, argc, argv);
		argc--;
		argv++;
		//post("record~ setting to '%s'", arrname->s_name);
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

        //old arsic notes - chn_n number of channels, 0 nsigs 1 nauxsigs
    t_record *x = (t_record *)pd_new(record_class);
    x->x_ksr = (float)sys_getsr() * 0.001;
    x->x_cybuf = cybuf_init((t_class *)x, arrname, chn_n);
    t_cybuf * c = x->x_cybuf;
    if (c)
    {
	
	x->x_numchans = c->c_numchans;
	t_float arraysmp = (t_float)c->c_npts;
        x->x_ivecs = getbytes(x->x_numchans * sizeof(*x->x_ivecs));
	if(loopend < 0 && arraysmp > 0){
//if loopend not set or less than 0 and arraysmp doesn't fail, set it to arraylen in ms
       loopend = (arraysmp/x->x_ksr);
	};
	cybuf_setminsize(x->x_cybuf, 2);
	record_append(x, append);	
	record_loop(x, loopstatus);
    
    x->x_phase = SHARED_INT_MAX; // instead of old record_reset(x) and so it doesn't record when DSP is on (stupid code);
    x->x_pauseindex = 0; // instead of old record_reset(x)'s SHARED_INT_MAX so append works
	x->x_clock = clock_new(x, (t_method)record_tick);
	x->x_clocklasttick = clock_getlogicaltime();
    x->x_array_ms = loopend;
	for (i=1;i<x->x_numchans;i++){
	    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	};
	x->x_stlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("start"));
	x->x_endlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("end"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal")); // sync output
    
	record_startpoint(x, loopstart);
	record_endpoint(x, loopend);
        record_mstoindex(x);
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
    class_addfloat(record_class, record_float);
    class_addmethod(record_class, (t_method)record_dsp, gensym("dsp"), A_CANT, 0);
    class_domainsignalin(record_class, -1);
    class_addmethod(record_class, (t_method)record_startpoint,
		    gensym("start"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_endpoint,
		    gensym("end"), A_FLOAT, 0);
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
