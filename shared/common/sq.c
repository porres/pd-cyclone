/* Copyright (c) 2001-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* squeak and squeal: sequencing utilities, a prototype version
   (the main, 'sq' part of the library) */

#include <stdlib.h>
#include <stdio.h>
#include "m_pd.h"
#include "shared.h"
#include "common/sq.h"

#if 1
#define SQ_VERBOSE
#if 0
#define SQ_DEBUG
#endif
#endif

/* #define SQUMPI_IGNORE */

#define SQUB_NALLOC     32
#define SQUMPI_NALLOC  (32 * sizeof(t_squmpo))
#define SQUAX_NALLOC   (32 * sizeof(t_squack))

#define SQUMPI_DEFAULT  500000  /* 120 bpm in microseconds per beat */

/* Arguments reqcount and elsize as in calloc, returns count */
size_t squb_checksize(void *buf, size_t reqcount, size_t elsize)
{
    t_squb *x = buf;
    size_t reqsize = reqcount * elsize;
    size_t newsize = x->b_bufsize;
    while (newsize < reqsize) newsize *= 2;
    if (newsize == x->b_bufsize)
	return (newsize);
#ifdef SQ_DEBUG
    post("need to resize buffer %x from %d to %d (requested size %d)",
	 (int)x, x->b_bufsize, newsize, reqsize);
#endif
    if (!(x->b_data = resizebytes(x->b_data, x->b_bufsize, newsize)) &&
	/* rather hopeless... */
	!(x->b_data = getbytes(newsize = SQUB_NALLOC * elsize)))
	newsize = 0;
    return ((x->b_bufsize = newsize) / elsize);  /* LATER do it right */
}

/* generic event */

/* tempo map */

/* comparison function used by qsort */
static int squmpi_compare(const void *tp1, const void *tp2)
{
    return (((t_squmpo *)tp1)->te_onset > ((t_squmpo *)tp2)->te_onset ? 1 : -1);
}

void squmpi_sort(t_sq *x)
{
    int i;
    t_squmpo *tp;
    qsort(x->s_tempomap, x->s_ntempi, sizeof(t_squmpo), squmpi_compare);
#if defined SQ_VERBOSE && ! defined SQ_DEBUG
    for (i = x->s_ntempi, tp = x->s_tempomap; i > 0 ; i--, tp++)
	post("tempo %d at %d", tp->te_value, (int)tp->te_onset);
#endif
}

t_squmpo *squmpi_add(t_sq *x)
{
    size_t count = x->s_ntempi + 1;
    t_squmpo *tep;
    if (squb_checksize(x->s_mytempi, count, sizeof(t_squmpo)) < count)
	return (0);
    tep = x->s_tempomap + x->s_ntempi++;
    squmpo_reset(tep);
    return (tep);
}

void squmpo_reset(t_squmpo *x)
{
    x->te_onset = 0;
    x->te_value = SQUMPI_DEFAULT;
}

/* track map */

t_squack *squax_add(t_sq *x)
{
    size_t count = x->s_ntracks + 2;  /* guard point */
    t_squack *trp;
    if (squb_checksize(x->s_mytracks, count, sizeof(t_squack)) < count)
	return (0);
    trp = x->s_trackmap + x->s_ntracks++;
    squack_reset(trp);
    return (trp);
}

void squack_reset(t_squack *x)
{
    x->tr_id = 0;  /* this is no-id */
    x->tr_nevents = 0;
    x->tr_name = 0;
    x->tr_head = 0;
}

/* generic iterator */

void *squiter_new(t_sq *x)
{
    if (x->s_myiter = getbytes(sizeof(*x->s_myiter)))
    {
    }
    return (x->s_myiter);
}

/* routines to access iterator hooks (setting hooks is explicit only) */
t_squiter_seekhook squiter_seekhook(t_squiter *x)
{
    return (x ? (t_squiter_seekhook)x->i_hooks[SQUITER_SEEKHOOK] : 0);
}

t_squiter_incrhook squiter_incrhook(t_squiter *x)
{
    return (x ? (t_squiter_incrhook)x->i_hooks[SQUITER_INCRHOOK] : 0);
}

t_squiter_getevehook squiter_getevehook(t_squiter *x)
{
    return (x ? (t_squiter_getevehook)x->i_hooks[SQUITER_GETEVEHOOK] : 0);
}

t_squiter_setevehook squiter_setevehook(t_squiter *x)
{
    return (x ? (t_squiter_setevehook)x->i_hooks[SQUITER_SETEVEHOOK] : 0);
}

t_squiter_gettimhook squiter_gettimhook(t_squiter *x)
{
    return (x ? (t_squiter_gettimhook)x->i_hooks[SQUITER_GETTIMHOOK] : 0);
}

t_squiter_settimhook squiter_settimhook(t_squiter *x)
{
    return (x ? (t_squiter_settimhook)x->i_hooks[SQUITER_SETTIMHOOK] : 0);
}

t_squiter_gettarhook squiter_gettarhook(t_squiter *x)
{
    return (x ? (t_squiter_gettarhook)x->i_hooks[SQUITER_GETTARHOOK] : 0);
}

t_squiter_settarhook squiter_settarhook(t_squiter *x)
{
    return (x ? (t_squiter_settarhook)x->i_hooks[SQUITER_SETTARHOOK] : 0);
}

/* time conversion */

/* Compute reusable coefficient, rather then repeatedly apply the formula.
   For smpte time:
   d msecs == (d / 1000.) secs == ((d * nframes * nticks) / 1000.) ticks
   or for metrical time:
   d msecs == (d * 1000.) usecs == ((d * 1000.) / tempo) beats
   == ((d * nticks * 1000.) / tempo) ticks
*/
/* LATER ntsc */
float sq_ticks2msecs(t_sq *x, uint32 tempo)
{
    if (x->s_nframes)
	return (1000. / (x->s_nframes * x->s_nticks));
    if (tempo <= 0)
	tempo = x->s_tempo;
    if (tempo <= 0)
	tempo = SQUMPI_DEFAULT;
    return (tempo / (x->s_nticks * 1000.));
}

float sq_msecs2ticks(t_sq *x, uint32 tempo)
{
    if (x->s_nframes)
	return (((x->s_nframes * x->s_nticks) / 1000.));
    if (!tempo)
	tempo = x->s_tempo;
    if (!tempo)
	tempo = SQUMPI_DEFAULT;
    return ((x->s_nticks * 1000.) / tempo);
}

/* transform onset ticks into delta msecs */
void sq_fold_time(t_sq *x)
{
    t_squiter *it = x->s_myiter;
    t_squiter_seekhook seekhook = squiter_seekhook(it);
    t_squiter_incrhook incrhook = squiter_incrhook(it);
    t_squiter_gettimhook gethook = squiter_gettimhook(it);
    t_squiter_settimhook sethook = squiter_settimhook(it);
    int i, ret, nevents = x->s_nevents;

    if (!it || !seekhook(it, 0))
	return;
    if (x->s_nframes)
    {
	float coef = sq_ticks2msecs(x, 0);
	t_float lasttime = 0;
	for (i = 0; i < nevents; i++)
	{
	    if (ret = squiter_inrange(it))
	    {
		t_float thistime = gethook(it, &ret) * coef;
		/* back to delta time */
		if (ret) sethook(it, thistime - lasttime, &ret);
		lasttime = thistime;
	    }
	    if (ret) incrhook(it);
	    else
	    {
		post("sequence folding error: bad iterator");
		break;
	    }
	}
    }
    else  /* apply tempomap */
    {
	float coef = sq_ticks2msecs(x, SQUMPI_DEFAULT);
	int ntempi = x->s_ntempi;
	t_float lasttime = 0, thistime = 0;
	t_float temposince = 0;
	t_float tempoonset = 0;
	int tempondx = 0;
	for (i = 0; i < nevents; i++)
	{
	    if (ret = squiter_inrange(it))
	    {
		t_float thisonset = gethook(it, &ret);
		t_float nexttempoonset;
#ifdef SQUMPI_IGNORE
		thistime = thisonset * coef;
#else
		while (tempondx < ntempi  /* LATER consider using guard point */
		       && (nexttempoonset = x->s_tempo_onset(tempondx))
		       < thisonset)
		{
		    temposince += (nexttempoonset - tempoonset) * coef;
		    tempoonset = nexttempoonset;
		    coef = sq_ticks2msecs(x, x->s_tempo_value(tempondx));
		    tempondx++;
		}
		thistime = temposince + (thisonset - tempoonset) * coef;
#endif
		if (thistime < lasttime)
		{
#ifdef SQ_DEBUG
		    /* FIXME under msvc -- horror! */
		    if (thistime != lasttime)
			post("ndx %d, this-last (%x-%x) %.15f, \
tix %.9f, tsince %.9f, ttix %.9f, coef %.9f",
			     tempondx, (int)thistime, (int)lasttime,
			     thistime - lasttime,
			     thisonset, temposince, tempoonset, coef);
#endif
		    thistime = lasttime;
		}
		/* back to delta time */
		if (ret) sethook(it, thistime - lasttime, &ret);
		lasttime = thistime;
	    }
	    if (ret) incrhook(it);
	    else
	    {
		post("sequence folding error: bad iterator");
		break;
	    }
	}
    }
}

/* transform delta msecs into onset msecs */
/* LATER add an option (or a separate function) for obtaining ticks
   (according to tempomap) */
void sq_unfold_time(t_sq *x)
{
    t_squiter *it = x->s_myiter;
    t_squiter_seekhook seekhook = squiter_seekhook(it);
    t_squiter_incrhook incrhook = squiter_incrhook(it);
    t_squiter_gettimhook gethook = squiter_gettimhook(it);
    t_squiter_settimhook sethook = squiter_settimhook(it);
    int i, ret, nevents = x->s_nevents;
    t_float thisonset = 0;

    if (!it || !seekhook(it, 0))
	return;
    for (i = 0; i < nevents; i++)
    {
	if (ret = squiter_inrange(it))
	{
	    thisonset += gethook(it, &ret);
	    if (ret) sethook(it, thisonset, &ret);
	}
	if (ret) incrhook(it);
	else
	{
	    post("sequence unfolding error: bad iterator");
	    break;
	}
    }
}

void sq_reset(t_sq *x)
{
    x->s_eof = 0;
    x->s_newtrack = 0;
    x->s_anapass = 1;
    x->s_fp = 0;
    x->s_time = 0;
    x->s_tempo = SQUMPI_DEFAULT;
    x->s_track = 0;
}

t_sq *sq_new(void)
{
    t_sq *x = (t_sq *)getbytes(sizeof(*x));
    if (!x)
	goto constructorfailure;

    /* these two are allocated in derived structure constructor */
    x->s_myiter = 0;
    x->s_auxeve = 0;

    if (!(x->s_mytempi = getbytes(sizeof(t_squmpi))))
	goto constructorfailure;
    if (!(x->s_tempomap = getbytes(x->s_mytempi->m_bufsize = SQUMPI_NALLOC)))
	goto constructorfailure;
    x->s_ntempi = 0;
    if (!(x->s_mytracks = getbytes(sizeof(t_squax))))
	goto constructorfailure;
    if (!(x->s_trackmap = getbytes(x->s_mytracks->m_bufsize = SQUAX_NALLOC)))
	goto constructorfailure;
    x->s_ntracks = 0;

    x->s_autoalloc = 0;
    x->s_format = 0;
    x->s_nticks = 192;  /* LATER parametrize this somehow */
    x->s_nframes = 0;

    sq_reset(x);
    return (x);
constructorfailure:
    if (x) sq_free(x);
    return (0);
}

void sq_free(t_sq *x)
{
    if (x->s_mytempi)
    {
	if (x->s_tempomap)
	    freebytes(x->s_tempomap, x->s_mytempi->m_bufsize);
	freebytes(x->s_mytempi, sizeof(t_squmpi));
    }
    if (x->s_mytracks)
    {
	if (x->s_trackmap)
	    freebytes(x->s_trackmap, x->s_mytracks->m_bufsize);
	freebytes(x->s_mytracks, sizeof(t_squax));
    }
    if (x->s_myiter)
	freebytes(x->s_myiter, sizeof(*x->s_myiter));
    freebytes(x, sizeof(*x));
}
