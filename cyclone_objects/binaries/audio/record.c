 /* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.

Derek Kwan 2016
adding defaults for numchannels, append, loopstatus, loopstart, loopend
adding record_getarraysmp, record_startms, record_endms, record_list, record_loopstart, record_loopend
adding attribute parsing and making numchannels arg optional 
rewrite record_perform, record_reset, recor_float, record_startpoint, record_endpoint
original start and end methods redirect to startms, endms methods
basically almost everything in this thing
de-arsic/cybuffing

NOTED CHANGES IN BEHAVIOR:
start and end point inlets changed from float inlets to sig inlets
end point inlet set to 0 no longer defaults to the whole array
I can change this if necessariy,.. I just figure if you're sending a phasor that
starts 0 and goes to some number, you don't want
the beginning of the ramp to default to the end of the whole array, right?
*/

// Porres fixed bugs with sync output, append mode, reset message and cleaned up a bit
 
#include "m_pd.h"
#include "m_imp.h"
#include "signal/cybuf.h"
#include <common/api.h>

#define DRAWMS  500. // refractory period in ms

typedef struct _record{
    t_object    x_obj;
    t_cybuf    *x_cybuf;
    t_inlet    *x_stlet;        // start inlet
    t_inlet    *x_endlet;       // end inlet
    t_outlet   *x_outlet;
    int         x_appendmode;
    int         x_loopmode;
    int         x_phase;        // writing head
    t_float     x_sync;         // sync value
    t_clock     *x_clock;
    double      x_clocklasttick;
    int         x_isrunning;
    int         x_newrun;       // if running turned from off and on for this current block
    t_float     x_ksr;          // sample rate in ms
    int 	    x_numchans;
    t_float   **x_ivecs;        // input vectors
    t_float    *x_startvec;     // start position (in ms) vector
    t_float    *x_endvec;       // endposition (in ms) vec
    t_float    *x_ovec;         // output vector
}t_record;

static t_class *record_class;

static void record_list(t_record *x, t_symbol *s, int argc, t_atom * argv){
    s = NULL;
    t_float startms, endms;
    switch(argc){
        case 0: // nothing passed
            break;
        case 1: // startms passed
            startms = atom_getfloatarg(0, argc, argv);
            pd_float((t_pd *)x->x_stlet, startms);
            break;
        default:
        case 2: // both passed
            startms = atom_getfloatarg(0, argc, argv);
            endms = atom_getfloatarg(1, argc, argv);
            pd_float((t_pd *)x->x_stlet, startms);
            pd_float((t_pd *)x->x_endlet, endms);
        break;
    }
}

static void record_draw(t_record *x){
    if(x->x_cybuf->c_playable)
        cybuf_redraw(x->x_cybuf);
}

static void record_tick(t_record *x){
    double timesince = clock_gettimesince(x->x_clocklasttick);
    if(timesince >= DRAWMS){
        cybuf_redraw(x->x_cybuf);
        x->x_clocklasttick = clock_getlogicaltime();
    }
}

static void record_set(t_record *x, t_symbol *s){
    cybuf_setarray(x->x_cybuf, s );
}

static void record_reset(t_record *x){ // new
    t_float loopstart  = 0.;
    t_float loopend = (t_float)x->x_cybuf->c_npts/x->x_ksr; // array size in samples
    if(x->x_sync > 0)
        x->x_isrunning = 1;
    pd_float((t_pd *)x->x_stlet, loopstart);
    pd_float((t_pd *)x->x_endlet, loopend);
    x->x_phase = 0.;
}

static int record_startpoint(t_record *x, t_floatarg f){
    long long int npts = (long long int)x->x_cybuf->c_npts;
    long long int startindex = (long long int)(f * x->x_ksr);
    if(startindex >= npts || startindex < 0)
		startindex = npts - 1;
    return(startindex);
}

static int record_endpoint(t_record *x, t_floatarg f){
//    post("record_endpoint f = %f", f);
    long long int npts = (long long int)x->x_cybuf->c_npts;
    long long int endindex = (long long int)(f * x->x_ksr);
    if(endindex >= npts || endindex < 0)
		endindex = npts - 1;
    return(endindex);
}

static void record_float(t_record *x, t_float f){
    x->x_isrunning = (f != 0);
    if(x->x_isrunning)
        x->x_newrun = 1;
    else{
        clock_delay(x->x_clock, 0); // trigger a redraw
        x->x_sync = 0.;
        if(!x->x_appendmode)
            x->x_phase = 0.;
    }
}

static void record_append(t_record *x, t_floatarg f){
	x->x_appendmode = (f != 0);
}

static void record_loop(t_record *x, t_floatarg f){
    x->x_loopmode = (f != 0);
}

static t_int *record_perform(t_int *w){
    t_record *x = (t_record *)(w[1]);
    t_cybuf * c = x->x_cybuf;
    int nch = c->c_numchans;
    int nblock = (int)(w[2]);
    t_float *out = x->x_ovec;
    t_float *startin = x->x_startvec;
    t_float *endin = x->x_endvec;
    t_float startms, endms, sync;
    long long int startsamp, endsamp, phase, range;
    int i, j;
    cybuf_validate(c, 0);
    clock_delay(x->x_clock, 0); // calculate a redraw
    for(i = 0; i < nblock; i++){
        startms = startin[i] < 0 ? 0 : startin[i];
        endms = endin[i] < 0 ? 0 : endin[i];
//        post("endms (%f)", (float)endms);
        if((startms < endms) && c->c_playable && x->x_isrunning){
            startsamp = record_startpoint(x, startms);
            endsamp = record_endpoint(x, endms);
//            post("endsamp (%f)", (float)endsamp);
            range = endsamp - startsamp;
            // append mode shouldn't reset phase
            if(x->x_newrun == 1 && x->x_appendmode == 0){
                // isrunning 0->1 from last block, means reset phase appropriately
                x->x_newrun = 0;
                x->x_phase = startsamp;
                x->x_sync = 0.;
            };
            phase = x->x_phase;
            // boundschecking, do it here because points might changed when paused
            // easier case, when we're "done"
            if(phase >= endsamp){
                if(x->x_loopmode == 1)
                    phase = startsamp;
                else{ // not looping, just stop it
                    x->x_isrunning = 0;
                    // mb a bit redundant, but just make sure x->x_sync is 1
                    x->x_sync = 1;
                    // trigger redraw
                };
            };
            // harder case up to interpretation
            if(phase < startsamp) // if before startsamp, just jump to startsamp?
                phase = startsamp;
            if(x->x_isrunning == 1){ // if we're still running after boundschecking
                for(j = 0; j < nch; j++){
                    t_word *vp = c->c_vectors[j];
                    t_float *insig = x->x_ivecs[j];
                    if(vp)
                        vp[phase].w_float = insig[i];
                };
                // sync output
                sync = (t_float)(phase - startsamp)/(t_float)range;
                // increment stage
                phase++;
                x->x_phase = phase;
                x->x_sync = sync;
             };
        };
        // in any case, output sync value
        out[i] = x->x_sync;
    };
    // storing changed vars locally
    if(nblock){
        pd_float((t_pd *)x->x_stlet, startms);
        pd_float((t_pd *)x->x_endlet, endms);
    };
    return(w+3);
}

static void record_dsp(t_record *x, t_signal **sp){
    cybuf_checkdsp(x->x_cybuf);
    x->x_ksr= sp[0]->s_sr * 0.001;
    int i, nblock = sp[0]->s_n;
    t_signal **sigp = sp;
    for(i = 0; i < x->x_numchans; i++) // input vectors first
		*(x->x_ivecs+i) = (*sigp++)->s_vec;
    x->x_startvec = (*sigp++)->s_vec;
    x->x_endvec = (*sigp++)->s_vec;
    x->x_ovec = (*sigp++)->s_vec;
	dsp_add(record_perform, 2, x, nblock);
}

static void record_free(t_record *x){
    cybuf_free(x->x_cybuf);
    inlet_free(x->x_stlet);
    inlet_free(x->x_endlet);
    outlet_free(x->x_outlet);
    freebytes(x->x_ivecs, x->x_numchans * sizeof(*x->x_ivecs));
    if(x->x_clock)
        clock_free(x->x_clock);
}

static void record_loopstart(t_record *x, t_float loopstart){
    pd_float((t_pd *)x->x_stlet, loopstart);
}

static void record_loopend(t_record *x, t_float loopend){
    pd_float((t_pd *)x->x_endlet, loopend);
}

static void *record_new(t_symbol *s, int argc, t_atom *argv){
    s = NULL;
    int i;
	int numchan = 1;
	t_float append = 0;
	t_float loopstatus = 0;
	t_float loopstart = 0;
	t_float loopend = 1E+32;
    int nameset = 0; // flag if name is set
	t_symbol *arrname = NULL;
	if(argc > 0 && argv ->a_type == A_SYMBOL){
        if(!nameset){ // if name not passed so far, count arg as array name
            arrname = atom_getsymbolarg(0, argc, argv);
            argc--;
            argv++;
            nameset = 1;
        }
	};
	// NOW parse the rest of the args
	int argnum = 0;
	while(argc > 0){
		if(argv->a_type == A_SYMBOL){
			t_symbol *curarg = atom_getsymbolarg(0, argc, argv);
			if(curarg == gensym("@append")){
				if(argc >= 2){
					append = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else
					goto errstate;
			}
			else if(curarg == gensym("@loop")){
				if(argc >= 2){
					loopstatus = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else
					goto errstate;
			}
			else if(curarg == gensym("@loopstart")){
				if(argc >= 2){
					loopstart = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else
					goto errstate;
			}
			else if(curarg == gensym("@loopend")){
				if(argc >= 2){
					loopend = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else
					goto errstate;
			}
			else
				goto errstate;
		}
		else if(argv->a_type == A_FLOAT){
            if(nameset){
                t_float argval = atom_getfloatarg(0, argc, argv);
                switch(argnum){
                    case 0:
                        numchan = (int)argval;
                    break;
                    default:
                    break;
                };
                argnum++;
                argc--;
                argv++;
            }
            else
                goto errstate;
        }
		else
			goto errstate;
	};
    int chn_n = numchan < 1 ? 1 : numchan > 4 ? 4 : numchan == 3 ? 2 : numchan;
    // old arsic notes - chn_n number of channels, 0 nsigs 1 nauxsigs
    t_record *x = (t_record *)pd_new(record_class);
    x->x_ksr = (float)sys_getsr() * 0.001;
    x->x_cybuf = cybuf_init((t_class *)x, arrname, chn_n, 0);
    t_cybuf * c = x->x_cybuf;
    // init
    x->x_newrun = 0;
    x->x_isrunning = 0;
    x->x_sync = 0;
    x->x_phase = 0;
    if(c){ // setting channels and array sizes
        x->x_numchans = c->c_numchans;
        // allocate input vectors
        x->x_ivecs = getbytes(x->x_numchans * sizeof(*x->x_ivecs));
        // bounds checking
        if(loopstart < 0)
            loopstart = 0;
        cybuf_setminsize(x->x_cybuf, 2);
        record_append(x, append);
        record_loop(x, loopstatus);
        x->x_clock = clock_new(x, (t_method)record_tick);
        x->x_clocklasttick = clock_getlogicaltime();
        if(loopend < 0)
            loopend = 0;
        if(loopstart < 0)
            loopstart = 0;
        for(i = 1; i < x->x_numchans; i++)
            inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        x->x_stlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        pd_float((t_pd *)x->x_stlet, loopstart);
        x->x_endlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        pd_float((t_pd *)x->x_endlet, loopend);
        x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
    };
    pd_bind(&x->x_obj.ob_pd, gensym("pd-dsp-stopped"));
    return(x);
	errstate:
		post("record~: improper args");
		return NULL;
}

CYCLONE_OBJ_API void record_tilde_setup(void){
    record_class = class_new(gensym("record~"), (t_newmethod)record_new,
        (t_method)record_free, sizeof(t_record), 0, A_GIMME, 0);
    class_addfloat(record_class, record_float);
    class_addbang(record_class, record_draw);
    class_addmethod(record_class, (t_method)record_dsp, gensym("dsp"), A_CANT, 0);
    class_addlist(record_class, (t_method)record_list);
    class_domainsignalin(record_class, -1);
    class_addmethod(record_class, (t_method)record_append, gensym("append"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_loop, gensym("loop"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_set, gensym("set"), A_SYMBOL, 0);
    class_addmethod(record_class, (t_method)record_reset, gensym("reset"), 0);
    class_addmethod(record_class, (t_method)record_loopstart, gensym("loopstart"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_loopend, gensym("loopend"), A_FLOAT, 0);
}
