/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*
updated to add @ramp attribute, slightly updated argument parsing.
left a lot of legacy code (the way all methods do things remains intact)
but managed to remove all external dependencies. commented out the loud_debug stuff
(which got turned on when MATRIX_DEBUG was defined).
matrix_float is there because the first inlet shouldn't accept floats
(and not do the usu convert float to signal business)
for some reason this code segfaults if class_domainsignalin(matrix_class, -1)
isn't there (don't ask me why)
changed matrix_free to return void * instead of nothing
(for consistency's sake)
- Derek Kwan 2016
*/

#include <string.h>
#include <math.h>
#include "m_pd.h"
#include <common/api.h>
#include "common/magicbit.h"

#define MATRIX_DEFGAIN  0.  /* CHECKED */
#define MATRIX_DEFRAMP  10.  /* CHECKED */

#define MATRIX_GAINEPSILON  1e-20f
#define MATRIX_MINRAMP      .001  /* LATER rethink */

#define MATRIX_MININLETS 1
#define MATRIX_MAXINLETS 250
#define MATRIX_MINOUTLETS 1
#define MATRIX_MAXOUTLETS 499

typedef struct _matrix
{
	t_object 	x_obj;
    int        x_numinlets;
    int        x_numoutlets;
    int        x_nblock;
    int        x_maxblock;
    t_float  **x_ivecs;
    t_float  **x_ovecs;
    t_float  **x_osums;
    int        x_ncells;
    int       *x_cells;
    t_outlet  *x_dumpout;
    /* The following fields are specific to nonbinary mode, i.e. we keep them
       unallocated in binary mode.  This is CHECKED to be incompatible:  c74
       always accepts (and reports) gains and ramps, although they are actually
       meaningless in binary mode, and switching modes is not supported. */
    float      x_defgain; // gain given as argument
    float     *x_gains;  /* target gains */
    float      x_deframp;
    float     *x_ramps;
    float      x_ksr;
    float     *x_coefs;  /* current coefs */
    float     *x_incrs;
    float     *x_bigincrs;
    int       *x_remains;
    /* Additions for filtering floats from secondary inlets -- Matt Barber*/
    t_float   *x_zerovec;
    t_float   *x_signalscalars[MATRIX_MAXINLETS];
    t_glist   *x_glist;
    int        x_hasfeeders[MATRIX_MAXINLETS];
} t_matrix;

typedef void (*t_matrix_cellfn)(t_matrix *x, int indx, int ondx,
				int onoff, float gain);

static t_class *matrix_class;

//EXTERN t_float *obj_findsignalscalar(t_object *x, int m);

/* called only in nonbinary mode;  LATER deal with changing nblock/ksr */
static void matrix_retarget(t_matrix *x, int cellndx)
{
    float target = (x->x_cells[cellndx] ? x->x_gains[cellndx] : 0.);
    if (x->x_ramps[cellndx] < MATRIX_MINRAMP)
    {
	x->x_coefs[cellndx] = target;
	x->x_remains[cellndx] = 0;
    }
    else
    {
    	x->x_remains[cellndx] =
	    x->x_ramps[cellndx] * x->x_ksr + 0.5;  /* LATER rethink */
    	x->x_incrs[cellndx] =
	    (target - x->x_coefs[cellndx]) / (float)x->x_remains[cellndx];
	x->x_bigincrs[cellndx] = x->x_nblock * x->x_incrs[cellndx];
    }
}

/* called only in nonbinary mode;  LATER deal with changing nblock/ksr */
static void matrix_retarget_connect(t_matrix *x, int cellndx)
{
    float target = (x->x_cells[cellndx] ? x->x_gains[cellndx] = x->x_defgain : 0.);
    if (x->x_ramps[cellndx] < MATRIX_MINRAMP)
    {
        x->x_coefs[cellndx] = target;
        x->x_remains[cellndx] = 0;
    }
    else
    {
        x->x_remains[cellndx] =
        x->x_ramps[cellndx] * x->x_ksr + 0.5;  /* LATER rethink */
        x->x_incrs[cellndx] =
        (target - x->x_coefs[cellndx]) / (float)x->x_remains[cellndx];
        x->x_bigincrs[cellndx] = x->x_nblock * x->x_incrs[cellndx];
    }
}

static void matrix_float(t_matrix *x, t_float f)
{
	pd_error(x, "matrix~: no method for float");
}

static void matrix_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{

	int inlet_idx, outlet_idx, cell_idx, onoff;
    float gain, ramp;

	//init vals
	inlet_idx = 0;
	outlet_idx = 0;
	cell_idx = 0;
	onoff = 0;
	gain = 0;
	ramp = 0;

    if (argc < 3)
    { //ignore if less than 3 args
		return;  
	};

	int argnum = 0;
	int rampset = 0; //setting of ramp arg flag
	while(argc > 0)
        { //argument parsing
		t_float argval = 0; //if not float, set equal to 0, else get value
		if(argv -> a_type == A_FLOAT){
			argval = atom_getfloatarg(0,argc,argv);
		};
		switch(argnum)
            { //if more than 4 args, then just ignore;
			case 0:
				inlet_idx = (int)argval;
				break;
			case 1:
				outlet_idx = (int)argval;
				break;
			case 2:
				gain = argval;
				break;
			case 3:
				ramp = argval;
				rampset = 1;
				break;
			default:
				break;
            };
		argnum++;
		argc--;
		argv++;
	};

	//now for bounds checking!!!
	if(inlet_idx < 0 || inlet_idx >= x->x_numinlets){
		pd_error(x, "matrix~: %d is not a valid inlet index!", inlet_idx);
		return;
	};
	if(outlet_idx < 0 || outlet_idx >= x->x_numoutlets){
		pd_error(x, "matrix~: %d is not a valid outlet index!", outlet_idx);
		return;
	};

    cell_idx = inlet_idx * x->x_numoutlets + outlet_idx;
    //negative gain used in nonbinary mode, accepted as 1 in binary (legacy code)
    onoff = (gain < -MATRIX_GAINEPSILON || gain > MATRIX_GAINEPSILON);
    x->x_cells[cell_idx] = onoff;
    if (x->x_gains)
        { //if in nonbinary mode
		if (onoff)
            { // CHECKME
		    x->x_gains[cell_idx] = gain;
            };
		if (rampset)
            {
	    	x->x_ramps[cell_idx] = (ramp < MATRIX_MINRAMP ? 0. : ramp);
            };
        matrix_retarget(x, cell_idx);
        };
}

static void matrix_clear(t_matrix *x)
{
    int i;
    for (i = 0; i < x->x_ncells; i++)
    {
        x->x_cells[i] = 0;
        if (x->x_gains)
            matrix_retarget(x, i);
    }
}


static void matrix_connect(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
    int onoff = (s == gensym("connect")), inlet_idx, celloffset;
    if (argc < 2){ //if less than 2 args, fail gracefully
		return;  /* CHECKED */
	};

	//parse first arg as inlet index
	//if symbol, let equal 0
	t_float inlet_flidx = 0;
	if(argv ->a_type == A_FLOAT){
		inlet_flidx  = atom_getfloatarg(0, argc, argv);
	};
	inlet_idx = (int)inlet_flidx;
	//bounds checking for inlet index
	if(inlet_idx < 0 || inlet_idx >= x->x_numinlets){
		pd_error(x, "matrix~: %d is not a valid inlet index!", inlet_idx);
		return;
	};
	argc--;
	argv++;

    celloffset = inlet_idx * x->x_numoutlets;

	//parse the rest of the args as outlet indices
    while (argc > 0)
    {
		int outlet_idx, cell_idx;

		//pasre arg as outlet idx, if symbol, let equal 0
		t_float outlet_flidx = 0;
		if(argv -> a_type == A_FLOAT){
			outlet_flidx = atom_getfloatarg(0, argc, argv);
		};
		outlet_idx = (int)outlet_flidx;
		//bounds checking for outlet index
		if(outlet_idx < 0 || outlet_idx >= x->x_numoutlets){
			pd_error(x, "matrix~: %d is not a valid outlet index!", outlet_idx);
			return;
		};
		argc--;
		argv++;

		cell_idx = celloffset + outlet_idx;
		x->x_cells[cell_idx] = onoff;
		if (x->x_gains){
			//if in nonbinary mode
			matrix_retarget_connect(x, cell_idx);
		};
    };
}

/* CHECKED active ramps are not retargeted */
static void matrix_ramp(t_matrix *x, t_floatarg f)
{
    if (x->x_ramps)
    {
	int i;
	x->x_deframp = (f < MATRIX_MINRAMP ? 0. : f); /* CHECKED cell-specific ramps are lost */
	for (i = 0; i < x->x_ncells; i++)
	    x->x_ramps[i] = x->x_deframp;
    }
}

static t_int *matrix01_perform(t_int *w)
{
    t_matrix *x = (t_matrix *)(w[1]);
    int nblock = (int)(w[2]);
    t_float **ivecs = x->x_ivecs;
    t_float **ovecs = x->x_ovecs;
    t_float **osums = x->x_osums;
    int *cellp = x->x_cells;
    int indx;
    for (indx = 0; indx < x->x_numinlets; indx++)
    {
	t_float *ivec = *ivecs++;
	t_float **ovecp = osums;
	if (indx){
		if (!magic_isnan(*x->x_signalscalars[indx]))
		{
			pd_error(x, "matrix~: doesn't understand 'float'");
			magic_setnan(x->x_signalscalars[indx]);
		}
		if (!(x->x_hasfeeders[indx])) ivec = x->x_zerovec;
	}
	int ondx = x->x_numoutlets;
	while (ondx--)
	{
	    if (*cellp++)
	    {
		t_float *in = ivec;
		t_float *out = *ovecp;
		int sndx = nblock;
		while (sndx--)
		    *out++ += *in++;
	    }
	    ovecp++;
	}
    }
    osums = x->x_osums;
    indx = x->x_numoutlets;
    while (indx--)
    {
	t_float *in = *osums++;
	t_float *out = *ovecs++;
	int sndx = nblock;
	while (sndx--)
	{
	    *out++ = *in;
	    *in++ = 0.;
	}
    }
    return (w + 3);
}

static t_int *matrixnb_perform(t_int *w)
{
    t_matrix *x = (t_matrix *)(w[1]);
    int nblock = (int)(w[2]);
    t_float **ivecs = x->x_ivecs;
    t_float **ovecs = x->x_ovecs;
    t_float **osums = x->x_osums;
    int *cellp = x->x_cells;
    float *gainp = x->x_gains;
    float *coefp = x->x_coefs;
    float *incrp = x->x_incrs;
    float *bigincrp = x->x_bigincrs;
    int *nleftp = x->x_remains;
    int indx;
    for (indx = 0; indx < x->x_numinlets; indx++){
	t_float *ivec = *ivecs++;
	t_float **ovecp = osums;
	if (indx){
		if (!magic_isnan(*x->x_signalscalars[indx]))
		{
			pd_error(x, "matrix~: doesn't understand 'float'");
			magic_setnan(x->x_signalscalars[indx]);
		}
		if (!(x->x_hasfeeders[indx])) ivec = x->x_zerovec;
	}
	int ondx = x->x_numoutlets;
	while (ondx--){
	    t_float *in = ivec;
	    t_float *out = *ovecp;
	    float nleft = *nleftp;
	    int sndx = nblock;
	    if (nleft >= nblock){
			float coef = *coefp;
			float incr = *incrp;
			if ((*nleftp -= nblock) == 0){
			    *coefp = (*cellp ? *gainp : 0.);
			}
			else{
			    *coefp += *bigincrp;
			};
			while (sndx--)
		   		 *out++ += *in++ * coef, coef += incr;
	    }
	    else if (nleft > 0){
			float coef = *coefp;
			float incr = *incrp;
			sndx -= nleft;
			do{
				*out++ += *in++ * coef, coef += incr;
			}while (--nleft);
			if (*cellp)
			{
				coef = *coefp = *gainp;
				while (sndx--){
					*out++ += *in++ * coef;
				};
			}
			else{
				*coefp = 0.;
			};
			*nleftp = 0;
	    }
	    else if (*cellp)
	    {
			float coef = *coefp;
			while (sndx--){
				*out++ += *in++ * coef;
			};
	    }
	    cellp++;
	    ovecp++;
	    gainp++;
	    coefp++;
	    incrp++;
	    bigincrp++;
	    nleftp++;
	}
    }
    osums = x->x_osums;
    indx = x->x_numoutlets;
    while (indx--)
    {
	t_float *in = *osums++;
	t_float *out = *ovecs++;
	int sndx = nblock;
	while (sndx--)
	{
	    *out++ = *in;
	    *in++ = 0.;
	}
    }
    return (w + 3);
}

static void matrix_dsp(t_matrix *x, t_signal **sp)
{
    int i, nblock = sp[0]->s_n;
    t_float **vecp = x->x_ivecs;
    t_signal **sigp = sp;
    for (i = 0; i < x->x_numinlets; i++)
    {
		*vecp++ = (*sigp++)->s_vec;
		x->x_hasfeeders[i] = magic_inlet_connection((t_object *)x, x->x_glist, i, &s_signal);
	};
    vecp = x->x_ovecs;
    for (i = 0; i < x->x_numoutlets; i++){
		*vecp++ = (*sigp++)->s_vec;
	};
    if (nblock != x->x_nblock){
		if (nblock > x->x_maxblock){
			size_t oldsize = x->x_maxblock * sizeof(*x->x_osums[i]),
			newsize = nblock * sizeof(*x->x_osums[i]);
			for (i = 0; i < x->x_numoutlets; i++)
			x->x_osums[i] = resizebytes(x->x_osums[i], oldsize, newsize);
			oldsize = x->x_maxblock * sizeof(*x->x_zerovec);
			newsize = nblock * sizeof(*x->x_zerovec);
			x->x_zerovec = resizebytes(x->x_zerovec, oldsize, newsize);
			x->x_maxblock = nblock;
		};
	x->x_nblock = nblock;
    }
   
    if (x->x_gains) {
		x->x_ksr = sp[0]->s_sr * .001;
		dsp_add(matrixnb_perform, 2, x, nblock);
    }
    else{
		dsp_add(matrix01_perform, 2, x, nblock);
	};
}

static void matrix_cellout(t_matrix *x, int indx, int ondx,
			   int onoff, float gain)
{
    t_atom atout[3];
    SETFLOAT(&atout[0], (t_float)indx);
    SETFLOAT(&atout[1], (t_float)ondx);
    if (onoff)
	SETFLOAT(&atout[2], gain);
    else
	SETFLOAT(&atout[2], 0.);
    outlet_list(x->x_dumpout, &s_list, 3, atout);
}

static void matrix_cellprint(t_matrix *x, int indx, int ondx,
			     int onoff, float gain)
{
    post("%d %d %g", indx, ondx, (onoff ? gain : 0.));
}

/*
   legacy debug:
#ifdef MATRIX_DEBUG
static void matrix_celldebug(t_matrix *x, int indx, int ondx,
			     int onoff, float gain)
{
    loudbug_post("%d %d %g", indx, ondx, gain);
}
#endif
*/

static void matrix_report(t_matrix *x, float *gains, float defgain,
			  t_matrix_cellfn cellfn)
{
    if (gains)
    {
	int *cellp = x->x_cells;
	float *gp = gains;
	int indx, ondx;
	for (indx = 0; indx < x->x_numinlets; indx++)
	    for (ondx = 0; ondx < x->x_numoutlets; ondx++, cellp++, gp++)
		/* CHECKED all cells are printed */
		(*cellfn)(x, indx, ondx, *cellp, *gp);
    }
    else  /* CHECKED incompatible: gains confusingly printed in binary mode */
    {
	int *cellp = x->x_cells;
	int indx, ondx;
	for (indx = 0; indx < x->x_numinlets; indx++)
	    for (ondx = 0; ondx < x->x_numoutlets; ondx++, cellp++)
		/* CHECKED all cells are printed */
		(*cellfn)(x, indx, ondx, *cellp, defgain);
    }
}

static void matrix_dump(t_matrix *x)
{
    matrix_report(x, x->x_coefs, 1., matrix_cellout);
}

static void matrix_dumptarget(t_matrix *x)
{
    matrix_report(x, x->x_gains, 1., matrix_cellout);
}

static void matrix_print(t_matrix *x)
{ // CHECKED same output as 'dump' -> [matrix~] -> [print]
    matrix_report(x, x->x_coefs, 1., matrix_cellprint);
}

static void *matrix_free(t_matrix *x)
{
    if (x->x_ivecs)
	freebytes(x->x_ivecs, x->x_numinlets * sizeof(*x->x_ivecs));
    if (x->x_ovecs)
	freebytes(x->x_ovecs, x->x_numoutlets * sizeof(*x->x_ovecs));
    if (x->x_osums)
    {
	int i;
	for (i = 0; i < x->x_numoutlets; i++)
	    freebytes(x->x_osums[i], x->x_maxblock * sizeof(*x->x_osums[i]));
	freebytes(x->x_zerovec, x->x_maxblock * sizeof(*x->x_zerovec));
	freebytes(x->x_osums, x->x_numoutlets * sizeof(*x->x_osums));
    }
    if (x->x_cells)
	freebytes(x->x_cells, x->x_ncells * sizeof(*x->x_cells));
    if (x->x_gains)
	freebytes(x->x_gains, x->x_ncells * sizeof(*x->x_gains));
    if (x->x_ramps)
	freebytes(x->x_ramps, x->x_ncells * sizeof(*x->x_ramps));
    if (x->x_coefs)
	freebytes(x->x_coefs, x->x_ncells * sizeof(*x->x_coefs));
    if (x->x_incrs)
	freebytes(x->x_incrs, x->x_ncells * sizeof(*x->x_incrs));
    if (x->x_bigincrs)
	freebytes(x->x_bigincrs, x->x_ncells * sizeof(*x->x_bigincrs));
    if (x->x_remains)
	freebytes(x->x_remains, x->x_ncells * sizeof(*x->x_remains));

	return (void *)x;
}

static void *matrix_new(t_symbol *s, int argc, t_atom *argv)
{
	t_matrix *x = (t_matrix *)pd_new(matrix_class);

	t_float rampval = MATRIX_DEFRAMP;
	t_float nan32;
	magic_setnan(&nan32);
	x->x_numinlets = (int)MATRIX_MININLETS;
	x->x_numoutlets = (int)MATRIX_MINOUTLETS;
	x->x_defgain = MATRIX_DEFGAIN;

	int i;
	int argnum = 0;
	while(argc > 0){
		if(argv -> a_type == A_FLOAT){
			t_float argval = atom_getfloatarg(0, argc, argv);
			switch(argnum){
				case 0:
					if(argval < MATRIX_MININLETS){
						x->x_numinlets = (int)MATRIX_MININLETS;
					}
					else if (argval > MATRIX_MAXINLETS){
						x->x_numinlets = (int)MATRIX_MAXINLETS;
						post("matrix~: resizing to %d signal inlets", (int)MATRIX_MAXINLETS);
					}
					else{
						x->x_numinlets = (int)argval;
					};
					break;
				case 1:
					if(argval < MATRIX_MINOUTLETS){
						x->x_numoutlets = (int)MATRIX_MINOUTLETS;
					}
					else if (argval > MATRIX_MAXOUTLETS){
						x->x_numoutlets = (int)MATRIX_MAXOUTLETS;
						post("matrix~: resizing to %d signal outlets", (int)MATRIX_MAXOUTLETS);
					}
					else{
						x->x_numoutlets = (int)argval;
					};
					break;
				case 2:
					x->x_defgain = argval;
					break;
				default:
					break;
			};
			argc--;
			argv++;
			argnum++;
		}
		else if(argv -> a_type == A_SYMBOL){
			t_symbol *argname = atom_getsymbolarg(0, argc, argv);
			if(strcmp(argname->s_name, "@ramp")==0){
				if(argc >= 2){
					t_float argval = atom_getfloatarg(1, argc, argv);
					if(argval < MATRIX_MINRAMP){
						rampval = MATRIX_MINRAMP;
					}
					else{
						rampval = argval;
					};
					argc -= 2;
					argv += 2;
				}
				else{
					goto errstate;
				};
			}
			else{
				goto errstate;
			};
		}
		else{
			goto errstate;
		};
	};

	int gaingiven = argnum >= 3; //if >=  3 args given, then gain is given, binary mode is off

	x->x_ncells = x->x_numinlets * x->x_numoutlets;
	x->x_ivecs = getbytes(x->x_numinlets * sizeof(*x->x_ivecs));
	x->x_ovecs = getbytes(x->x_numoutlets * sizeof(*x->x_ovecs));
	x->x_nblock = x->x_maxblock = sys_getblksize();
	x->x_osums = getbytes(x->x_numoutlets * sizeof(*x->x_osums));
	for (i = 0; i < x->x_numoutlets; i++){
	    x->x_osums[i] = getbytes(x->x_maxblock * sizeof(*x->x_osums[i]));
	};
	x->x_cells = getbytes(x->x_ncells * sizeof(*x->x_cells));
	/* zerovec for filtering float inputs*/
	x->x_zerovec = getbytes(x->x_maxblock * sizeof(*x->x_zerovec));
	matrix_clear(x);

	if (gaingiven){
	    x->x_gains = getbytes(x->x_ncells * sizeof(*x->x_gains));
	    for (i = 0; i < x->x_ncells; i++){
            x->x_gains[i] = x->x_defgain;
		};
        
	    x->x_ramps = getbytes(x->x_ncells * sizeof(*x->x_ramps));
	    matrix_ramp(x, rampval);
	    x->x_coefs = getbytes(x->x_ncells * sizeof(*x->x_coefs));
	    
		for (i = 0; i < x->x_ncells; i++){
			x->x_coefs[i] = 0.;
		};
	    x->x_ksr = sys_getsr() * .001;
	    x->x_incrs = getbytes(x->x_ncells * sizeof(*x->x_incrs));
	    x->x_bigincrs = getbytes(x->x_ncells * sizeof(*x->x_bigincrs));
	    x->x_remains = getbytes(x->x_ncells * sizeof(*x->x_remains));
	    for (i = 0; i < x->x_ncells; i++){
			x->x_remains[i] = 0;
		};
	}
	else{
	    x->x_gains = 0;
	    x->x_ramps = 0;
	    x->x_coefs = 0;
	    x->x_incrs = 0;
	    x->x_bigincrs = 0;
	    x->x_remains = 0;
	};
	for (i = 1; i < x->x_numinlets; i++){
		pd_float( (t_pd *)inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal), nan32);
		x->x_signalscalars[i] = obj_findsignalscalar((t_object *)x, i);
	};
	for (i = 0; i < x->x_numoutlets; i++){
	 	outlet_new(&x->x_obj, gensym("signal"));
	};
	x->x_dumpout = outlet_new((t_object *)x, &s_list);
	x->x_glist = canvas_getcurrent();
	return (x);
	errstate:
		pd_error(x, "matrix~: improper args");
		return NULL;
}

CYCLONE_OBJ_API void matrix_tilde_setup(void)
{
    matrix_class = class_new(gensym("matrix~"),
			     (t_newmethod)matrix_new,
			     (t_method)matrix_free,
			     sizeof(t_matrix), 0, A_GIMME, 0);
	class_addmethod(matrix_class, nullfn, gensym("signal"), 0);
	class_addfloat(matrix_class, matrix_float);
    class_addlist(matrix_class, matrix_list);
    class_addmethod(matrix_class, (t_method)matrix_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(matrix_class, (t_method)matrix_clear, gensym("clear"), 0);
    class_addmethod(matrix_class, (t_method)matrix_connect, gensym("connect"), A_GIMME, 0);
    class_addmethod(matrix_class, (t_method)matrix_connect, gensym("disconnect"), A_GIMME, 0);
    class_addmethod(matrix_class, (t_method)matrix_ramp, gensym("ramp"), A_FLOAT, 0);
    class_addmethod(matrix_class, (t_method)matrix_dump, gensym("dump"), 0);
    class_addmethod(matrix_class, (t_method)matrix_dumptarget, gensym("dumptarget"), 0);
    class_addmethod(matrix_class, (t_method)matrix_print, gensym("print"), 0);

}
