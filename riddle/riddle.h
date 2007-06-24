/* Copyright (c) 2007 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __RIDDLE_H__
#define __RIDDLE_H__

EXTERN_STRUCT _riddle;
#define t_riddle  struct _riddle

EXTERN_STRUCT _rdsource;
#define t_rdsource  struct _rdsource

EXTERN_STRUCT _rdsink;
#define t_rdsink  struct _rdsink

EXTERN_STRUCT _rdpool;
#define t_rdpool  struct _rdpool

EXTERN_STRUCT _rdbuffer;
#define t_rdbuffer  struct _rdbuffer

typedef void (*t_rdblockfn)(t_riddle *);
typedef void (*t_rddspfn)(t_riddle *, t_signal **);

struct _riddle
{
    t_sic  rd_sic;

    /* designed for system-level control: block mismatches, etc.
       (user-level control via '_idle' slot in graphpool) */
    int  rd_disabled;
    int  rd_wasdisabled;

    t_rdblockfn  rd_blockfn;
    t_rddspfn    rd_dspfn;

    t_rdpool  *rd_graphpool;

    int  rd_graphsr;
    int  rd_graphblock;

    int          rd_nsiginlets;
    int          rd_nremoteslots;
    t_rdsource  *rd_inslots;      /* rd_nsiginlets + rd_nremoteslots elements */
    t_rdsource  *rd_remoteslots;  /* == rd_inslots + rd_nsiginlets */

    int        rd_nsigoutlets;
    t_rdsink  *rd_outslots;  /* rd_nsigoutlets elements */

    int  rd_remotesource;  /* LATER consider storing remote sources here */
};

void riddlebug_post(t_riddle *rd, char *pfx, char *fmt, ...);

int riddle_getsr(t_riddle *rd);
int riddle_getgraphblock(t_riddle *rd);

int riddle_getsourceblock(t_riddle *rd, int siginno);
t_symbol *riddle_getsourcepattern(t_riddle *rd, int siginno, int *maxblockp);
int riddle_getoutblock(t_riddle *rd, int sigoutno);
t_symbol *riddle_getoutpattern(t_riddle *rd, int sigoutno, int *maxblockp);

void riddle_setoutblock(t_riddle *rd, int sigoutno, int newblock);
void riddle_setoutpattern(t_riddle *rd, int sigoutno,
			  t_symbol *pattern, int maxblock);

int riddle_checksourceblock(t_riddle *rd, int siginno, int reqblock);
int riddle_checksourcepattern(t_riddle *rd, int siginno,
			      t_symbol *reqpattern, int *maxblockp);
int riddle_checkanysource(t_riddle *rd, int siginno);

int riddle_isdisabled(t_riddle *rd);
void riddle_disable(t_riddle *rd);
void riddle_mute(t_riddle *rd, t_signal **sp);

t_class *riddle_setup(t_symbol *name, t_newmethod newfn, t_method freefn,
		      size_t sz, t_method floatfn,
		      t_rdblockfn blockfn, t_rddspfn dspfn);

void riddle_updatedsp(void);

void rdpool_set(t_rdpool *rp, t_symbol *key, int size, t_float *data);
void rdpool_setbygraph(t_canvas *graph, t_symbol *key, int size, t_float *data);
t_float *rdpool_get(t_rdpool *rp, t_symbol *key, int *sizep);
t_float *riddle_getlink(t_riddle *rd, t_symbol *key, int *sizep);
t_canvas *riddle_getgraph(t_riddle *rd, int sigoutno);

t_rdbuffer *riddle_getbuffer(t_symbol *name);
t_rdbuffer *rdbuffer_getwriter(t_rdbuffer *rb);
void rdbuffer_validate(t_rdbuffer *rb, int nblock);
void rdbuffer_reset(t_rdbuffer *rb);
int rdbuffer_getframesize(t_rdbuffer *rb);
t_float *rdbuffer_gethead(t_rdbuffer *rb);
void rdbuffer_stephead(t_rdbuffer *rb);
void rdbuffer_movehead(t_rdbuffer *rb, int nframes);
void rdbuffer_delayhead(t_rdbuffer *rb, int nframes);
void rdbuffer_free(t_rdbuffer *rb);
t_rdbuffer *rdbuffer_new(t_riddle *owner, t_symbol *name, int nframes);
t_rdbuffer *rdbuffer_newreader(t_riddle *owner, t_symbol *name);

int riddle_erbfill(int nbands, int *buf, int nblock, int sr);

#endif
