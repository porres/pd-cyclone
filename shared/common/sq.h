/* Copyright (c) 2001-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* squeak and squeal: sequencing utilities, a prototype version
   (the main, 'sq' part of the library) */
/* LATER move everything not needed in mifi to squeal */

#ifndef __SQ_H__
#define __SQ_H__

/* Generic buffer structure, the `base' for t_squeve, t_squmpi and t_squax */
typedef struct _squb
{
    uint32  b_length;   /* length of data currently used (in items) */
    uchar  *b_data;     /* data buffer */
    size_t  b_bufsize;  /* allocated size of data buffer (in bytes) */
} t_squb;

/* Generic event structure.  Designed as an interface to squeak routines,
   and not to be kept in arrays or lists (use sequence containers instead).
   Data buffer is automatically allocated and resized by those routines. */
typedef struct _squeve
{
    uint32  e_length;  /* set for any event type! */
    uchar  *e_data;
    size_t  e_bufsize;
    uint32  e_delay;
} t_squeve;

/* tempo map element */
typedef struct _squmpo
{
    t_float  te_onset;  /* ticks or microseconds from start of sequence */
    uint32   te_value;  /* microseconds per beat */
} t_squmpo;

typedef struct _squmpi
{
    uint32     m_ntempi;
    t_squmpo  *m_map;
    size_t     m_bufsize;  /* allocated size of m_map array in bytes */
} t_squmpi;

/* track/subtrack map element */
typedef struct _squack
{
    int        tr_id;       /* according to target template */
    uint32     tr_nevents;  /* number of events (in this track or pre-total) */
    t_symbol  *tr_name;     /* track name */
    void      *tr_head;     /* pointer to first event */
} t_squack;

typedef struct _squax
{
    uint32     m_ntracks;
    t_squack  *m_map;
    size_t     m_bufsize;  /* allocated size of m_map array in bytes */
} t_squax;

/* generic type of callback routines used to read/write
   sequence containers through t_squiter */
typedef int (*t_squiterhook)(void *it);

#define SQUITER_SEEKHOOK    0
#define SQUITER_INCRHOOK    1
#define SQUITER_GETEVEHOOK  2
#define SQUITER_SETEVEHOOK  3
#define SQUITER_GETTIMHOOK  4
#define SQUITER_SETTIMHOOK  5
#define SQUITER_GETTARHOOK  6
#define SQUITER_SETTARHOOK  7
#define SQUITER_NHOOKS      8
/* LATER move these typedefs to sq.c, if still not used globally */
typedef int (*t_squiter_seekhook)(void *it, int offset);
typedef void (*t_squiter_incrhook)(void *it);
typedef void (*t_squiter_getevehook)(void *it, t_squeve *evp, int *ret);
typedef void (*t_squiter_setevehook)(void *it, t_squeve *evp, int *ret);
typedef t_float (*t_squiter_gettimhook)(void *it, int *ret);
typedef void (*t_squiter_settimhook)(void *it, t_float v, int *ret);
typedef t_symbol (*t_squiter_gettarhook)(void *it, int *ret);
typedef void (*t_squiter_settarhook)(void *it, t_symbol *s, int *ret);

/* elements might be 'atoms' or whole events, whatever suits better */
typedef struct _squiter
{
    void          *i_owner;
    int            i_nelems;
    void          *i_sequence;  /* first element pointer */
    void          *i_element;   /* current element pointer */
    int            i_index;     /* current element index */
    t_squiterhook  i_hooks[SQUITER_NHOOKS];
} t_squiter;

/* This is a good candidate for a derivation hierarchy. */
typedef struct _sq
{
    t_squiter  *s_myiter;
    t_squax    *s_mytracks;
    t_squmpi   *s_mytempi;   /* use shortcuts #defined below */
    void       *s_auxeve;    /* auxiliary event */
    uint32  s_nevents;       /* total number of events */
    FILE   *s_fp;            /* hmm... */
    int     s_autoalloc:1;   /* set if auto-allocated */
    int     s_eof:1;      /* reading: set in case of early eof (error) */
    int     s_newtrack:1; /* reading: set if first event in a track */
    int     s_anapass:1;  /* read/write: set during analysis (pass #1)  */
    uchar   s_nframes;    /* fps if nonzero, else use metrical time */
    uint16  s_nticks;     /* number of ticks per beat or per frame */
    uint16  s_format;     /* `ismultitrack' flag, LATER add other formats */
    uint32  s_time;       /* current time in ticks */
    uint32  s_tempo;      /* current tempo, or last one encountered in a file */
    int     s_trackid;    /* LATER remove? */
    uint16  s_track;      /* current track number */

    /* fields below are specific to midifile streams */
    uchar   s_status;     /* current running status, | channel in writing */
    uchar   s_channel;    /* current channel, not used in writing */
    uint16  s_hdtracks;   /* number of tracks declared in a midifile header */
    uint32  s_alltracks;  /* total number of nonempty tracks */
                          /* (s_ntracks counts `in range' nonempty tracks) */
    float   s_timecoef;   /* msecs->ticks, used in writing only */
    uint32  s_bytesleft;  /* nbytes remaining to be read from current track,
			     or number of bytes written to a track so far */
} t_sq;

#define s_ntempi              s_mytempi->m_ntempi
#define s_tempomap            s_mytempi->m_map
#define s_tempo_onset(ndx)    s_mytempi->m_map[ndx].te_onset
#define s_tempo_value(ndx)    s_mytempi->m_map[ndx].te_value
#define s_ntracks             s_mytracks->m_ntracks
#define s_trackmap            s_mytracks->m_map
#define s_track_id(ndx)       s_mytracks->m_map[ndx].tr_id
#define s_track_nevents(ndx)  s_mytracks->m_map[ndx].tr_nevents
#define s_track_name(ndx)     s_mytracks->m_map[ndx].tr_name
#define s_track_head(ndx)     s_mytracks->m_map[ndx].tr_head

/* prototypes of public interface routines */

size_t squb_checksize(void *buf, size_t reqcount, size_t elsize);

#define squiter_inrange(it)  ((it)->i_index < (it)->i_nelems)
void *squiter_new(t_sq *x);
t_squiter_seekhook squiter_seekhook(t_squiter *x);
t_squiter_incrhook squiter_incrhook(t_squiter *x);
t_squiter_getevehook squiter_getevehook(t_squiter *x);
t_squiter_setevehook squiter_setevehook(t_squiter *x);
t_squiter_gettimhook squiter_gettimhook(t_squiter *x);
t_squiter_settimhook squiter_settimhook(t_squiter *x);
t_squiter_gettarhook squiter_gettarhook(t_squiter *x);
t_squiter_settarhook squiter_settarhook(t_squiter *x);

void squmpi_sort(t_sq *x);
t_squmpo *squmpi_add(t_sq *x);
void squmpo_reset(t_squmpo *x);
t_squack *squax_add(t_sq *x);
void squack_reset(t_squack *x);

float sq_ticks2msecs(t_sq *x, uint32 tempo);
float sq_msecs2ticks(t_sq *x, uint32 tempo);

void sq_fold_time(t_sq *x);
void sq_unfold_time(t_sq *x);

t_sq *sq_new(void);
void sq_reset(t_sq *x);
void sq_free(t_sq *x);

#endif
