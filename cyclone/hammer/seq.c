/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKED no sharing of data among seq objects having the same creation arg */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "shared.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/sq.h"
#include "common/bifi.h"
#include "common/mifi.h"
#include "hammer/file.h"

#define SEQ_DEBUG

#define SEQ_INISIZE    256  /* LATER rethink */
#define SEQ_EOM        255  /* end of message marker, LATER rethink */

typedef struct _seqevent
{
    int            e_delta;
    unsigned char  e_bytes[4];
} t_seqevent;

typedef struct _seq
{
    t_object       x_ob;
    t_canvas      *x_canvas;
    t_symbol      *x_defname;
    t_hammerfile  *x_filehandle;
    int            x_isplaying;
    int            x_isrecording;
    int            x_playhead;
    float          x_tempo;
    double         x_prevtime;
    double         x_clockdelay;
    unsigned char  x_status;
    int            x_evesize;
    int            x_expectedsize;
    int            x_size;     /* as allocated */
    int            x_nevents;  /* as used */
    t_seqevent    *x_sequence;
    t_seqevent     x_seqini[SEQ_INISIZE];
    t_clock       *x_clock;
    t_outlet      *x_bangout;
} t_seq;

static t_class *seq_class;

static void seq_doclear(t_seq *x, int dofree)
{
    if (dofree && x->x_sequence != x->x_seqini)
    {
	freebytes(x->x_sequence, x->x_size * sizeof(*x->x_sequence));
	x->x_sequence = x->x_seqini;
	x->x_size = SEQ_INISIZE;
    }
    x->x_nevents = 0;
}

static void seq_complete(t_seq *x)
{
    if (x->x_evesize < x->x_expectedsize)
    {
	/* CHECKED no warning if no data after status byte requiring data */
	if (x->x_evesize > 1)
	    post("seq: truncated midi message");  /* CHECKED */
	/* CHECKED nothing stored */
    }
    else
    {
	t_seqevent *ep = &x->x_sequence[x->x_nevents];
	double elapsed = clock_gettimesince(x->x_prevtime);
	ep->e_delta = (int)elapsed;
	x->x_prevtime = clock_getlogicaltime();
	if (x->x_evesize < 4)
	    ep->e_bytes[x->x_evesize] = SEQ_EOM;
	x->x_nevents++;
	if (x->x_nevents >= x->x_size)
	{
	    int nexisting = x->x_size;
	    /* store-ahead scheme, LATER consider using x_currevent */
	    int nrequested = x->x_nevents + 1;
#ifdef SEQ_DEBUG
	    post("growing...");
#endif
	    x->x_sequence =
		grow_withdata(&nrequested, &nexisting,
			      &x->x_size, x->x_sequence,
			      SEQ_INISIZE, x->x_seqini, sizeof(*x->x_sequence));
	    if (nrequested <= x->x_nevents)
		x->x_nevents = 0;
	}
    }
    x->x_evesize = 0;
}

static void seq_checkstatus(t_seq *x, unsigned char c)
{
    if (x->x_status && x->x_evesize > 1)  /* LATER rethink */
	seq_complete(x);
    if (c < 192)
	x->x_expectedsize = 3;
    else if (c < 224)
	x->x_expectedsize = 2;
    else if (c < 240)
	x->x_expectedsize = 3;
    else if (c < 248)
    {
	/* FIXME */
	x->x_expectedsize = -1;
    }
    else
    {
	x->x_sequence[x->x_nevents].e_bytes[0] = c;
	x->x_evesize = x->x_expectedsize = 1;
	seq_complete(x);
	return;
    }
    x->x_status = x->x_sequence[x->x_nevents].e_bytes[0] = c;
    x->x_evesize = 1;
}

static void seq_addbyte(t_seq *x, unsigned char c, int docomplete)
{
    x->x_sequence[x->x_nevents].e_bytes[x->x_evesize++] = c;
    if (x->x_evesize == x->x_expectedsize)
    {
	seq_complete(x);
	if (x->x_status)
	{
	    x->x_sequence[x->x_nevents].e_bytes[0] = x->x_status;
	    x->x_evesize = 1;
	}
    }
    else if (x->x_evesize == 4)
    {
	if (x->x_status != 240)
	    bug("seq_addbyte");
	/* CHECKED sysex is broken into 4-byte packets marked with
	   the actual delta time of last byte received in a packet */
	seq_complete(x);
    }
    else if (docomplete) seq_complete(x);
}

static void seq_endofsysex(t_seq *x)
{
    seq_addbyte(x, 247, 1);
    x->x_status = 0;
}

static void seq_stopplayback(t_seq *x)
{
    /* FIXME */
    /* CHECKED "seq: incomplete sysex" at playback stop, 247 added implicitly */
    /* CHECKME resetting controllers, etc. */
    /* CHECKED bang not sent if playback stopped early */
    clock_unset(x->x_clock);
    x->x_playhead = 0;
    x->x_isplaying = 0;
}

static void seq_stoprecording(t_seq *x)
{
    if (x->x_status == 240)
    {
	post("seq: incomplete sysex");  /* CHECKED */
	seq_endofsysex(x);  /* CHECKED 247 added implicitly */
    }
    else if (x->x_status)
	seq_complete(x);
    /* CHECKED running status used in recording, but not across recordings */
    x->x_status = 0;
    x->x_isrecording = 0;
}

static void seq_clocktick(t_seq *x)
{
    if (x->x_isplaying)
    {
	t_seqevent *ep = &x->x_sequence[x->x_playhead++];
	unsigned char *bp = ep->e_bytes;
nextevent:
	outlet_float(((t_object *)x)->ob_outlet, *bp++);
	if (*bp != SEQ_EOM)
	{
	    outlet_float(((t_object *)x)->ob_outlet, *bp++);
	    if (*bp != SEQ_EOM)
	    {
		outlet_float(((t_object *)x)->ob_outlet, *bp++);
		if (*bp != SEQ_EOM)
		    outlet_float(((t_object *)x)->ob_outlet, *bp++);
	    }
	}
	if (!x->x_isplaying)  /* reentrancy protection */
	    return;
	if (x->x_playhead < x->x_nevents)
	{
	    ep++;
	    if (ep->e_delta <= 0)
		/* continue output in the same scheduler event, LATER rethink */
	    {
		x->x_playhead++;
		bp = ep->e_bytes;
		goto nextevent;
	    }
	    else
	    {
		x->x_clockdelay = ep->e_delta * x->x_tempo;
		if (x->x_clockdelay < 0.)
		    x->x_clockdelay = 0.;
		clock_delay(x->x_clock, x->x_clockdelay);
		x->x_prevtime = clock_getlogicaltime();
	    }
	}
	else
	{
	    seq_stopplayback(x);
	    /* CHECKED bang sent immediately _after_ last byte */
	    outlet_bang(x->x_bangout);  /* LATER think about reentrancy */
	}
    }
}

static void seq_tick(t_seq *x)
{
    /* FIXME */
}

/* CHECKED running status not used in playback */
static void seq_dostart(t_seq *x, float tempo)
{
    if (x->x_isplaying)
    {
	/* CHECKED tempo change */
    	double elapsed = clock_gettimesince(x->x_prevtime);
    	double left = x->x_clockdelay - elapsed;
    	if (left < 0)
	    left = 0;
    	else
	    left *= tempo / x->x_tempo;
    	clock_delay(x->x_clock, x->x_clockdelay = left);
	x->x_prevtime = clock_getlogicaltime();
	x->x_tempo = tempo;
    }
    else
    {
	if (x->x_isrecording)  /* CHECKED 'start' stops recording */
	    seq_stoprecording(x);
	/* CHECKED bang not sent if a sequence is empty */
	if (x->x_nevents)
	{
	    x->x_tempo = tempo;
	    x->x_playhead = 0;
	    x->x_isplaying = 1;
	    /* playback data never sent within the scheduler event of
	       a start message (even for the first delta <= 0), LATER rethink */
	    x->x_clockdelay = x->x_sequence->e_delta * tempo;
	    if (x->x_clockdelay < 0.)
		x->x_clockdelay = 0.;
	    clock_delay(x->x_clock, x->x_clockdelay);
	    x->x_prevtime = clock_getlogicaltime();
	}
    }
}

static void seq_bang(t_seq *x)
{
    seq_dostart(x, 1.);
}

static void seq_float(t_seq *x, t_float f)
{
    if (x->x_isrecording)
    {
	/* CHECKED noninteger and out of range silently truncated */
	unsigned char c = (unsigned char)f;
	if (c < 128)
	{
	    if (x->x_status) seq_addbyte(x, c, 0);
	}
	else if (c != 254)  /* CHECKED active sensing ignored */
	{
	    if (x->x_status == 240)
	    {
		if (c == 247) seq_endofsysex(x);
		else
		{
		    /* CHECKED rt bytes alike */
		    post("seq: unterminated sysex");  /* CHECKED */
		    seq_endofsysex(x);  /* CHECKED 247 added implicitly */
		    seq_checkstatus(x, c);
		}
	    }
	    else if (c != 247) seq_checkstatus(x, c);
	}
    }
}

static void seq_symbol(t_seq *x, t_symbol *s)
{
    loud_nomethod((t_pd *)x, &s_symbol);  /* CHECKED */
}

static void seq_list(t_seq *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_FLOAT) seq_float(x, av->a_w.w_float);
    /* CHECKED anything else/more silently ignored */
}

static void seq_dorecord(t_seq *x)
{
    if (x->x_isplaying)  /* CHECKED 'record' and 'append' stops playback */
	seq_stopplayback(x);
    x->x_isrecording = 1;
    x->x_prevtime = clock_getlogicaltime();
    x->x_status = 0;
    x->x_evesize = 0;
    x->x_expectedsize = -1;  /* LATER rethink */
}

static void seq_record(t_seq *x)
{
    /* CHECKED 'record' resets recording */
    seq_doclear(x, 0);
    seq_dorecord(x);
}

static void seq_append(t_seq *x)
{
    /* CHECKED if isrecording, 'append' resets the timer */
    seq_dorecord(x);
}

static void seq_start(t_seq *x, t_floatarg f)
{
    if (f < 0)
    {
	/* FIXME */
    }
    else
    {
	float tempo = (f == 0 ? 1. : 1024. / f);
	if (tempo < 1e-20)
	    tempo = 1e-20;
	else if (tempo > 1e20)
	    tempo = 1e20;
	seq_dostart(x, tempo);
    }
}

static void seq_stop(t_seq *x)
{
    if (x->x_isplaying)
	seq_stopplayback(x);
    else if (x->x_isrecording)
	seq_stoprecording(x);
}

/* CHECKED first delta time is set permanently (it is stored in a file) */
static void seq_delay(t_seq *x, t_floatarg f)
{
    if (x->x_nevents)
	/* CHECKED signed/unsigned bug, not emulated */
	x->x_sequence->e_delta = (f > 0 ? f : 0);
}

/* CHECKED all delta times are set permanently (they are stored in a file) */
static void seq_hook(t_seq *x, t_floatarg f)
{
    int nevents;
    if (nevents = x->x_nevents)
    {
	t_seqevent *ev = x->x_sequence;
	if (f < 0)
	    f = 0;  /* CHECKED signed/unsigned bug, not emulated */
	while (nevents--) ev++->e_delta *= f;
    }
}

static int seq_dogrowing(t_seq *x, int nevents)
{
    if (nevents > x->x_size)
    {
	int nrequested = nevents;
#ifdef SEQ_DEBUG
	post("growing...");
#endif
	x->x_sequence =
	    grow_nodata(&nrequested, &x->x_size, x->x_sequence,
			SEQ_INISIZE, x->x_seqini, sizeof(*x->x_sequence));
	if (nrequested < nevents)
	{
	    x->x_nevents = 0;
	    return (0);
	}
    }
    x->x_nevents = nevents;
    return (1);
}

static int seq_seekhook(t_squiter *it, int offset)
{
    t_seq *x = (t_seq *)it->i_owner;
    post("seek in %d", x->x_nevents);
    it->i_nelems = x->x_nevents;
    it->i_sequence = x->x_sequence;
    if (offset < 0)
	offset += it->i_nelems;
    if (offset >= 0 && offset < it->i_nelems)
    {
	it->i_element = (t_seqevent *)it->i_sequence + offset;
	it->i_index = offset;
	return (1);
    }
    else return (0);
}

static void seq_incrhook(t_squiter *it)
{
    ((t_seqevent *)it->i_element)++;
    it->i_index++;
}

/* LATER put seq_mfwrite_doit() functionality here */
static void seq_getevehook(t_squiter *it, t_mifi_event *mev, int *ret)
{
    *ret = 1;
}

static void seq_setevehook(t_squiter *it, t_mifi_event *mev, int *ret)
{
    t_seqevent *sev = it->i_element;
    sev->e_delta = mev->e_delay;
    sev->e_bytes[0] = mev->e_status | mev->e_channel;
    sev->e_bytes[1] = mev->e_data[0];
    if (MIFI_ONE_DATABYTE(mev->e_status))
	sev->e_bytes[2] = SEQ_EOM;
    else
    {
	sev->e_bytes[2] = mev->e_data[1];
	sev->e_bytes[3] = SEQ_EOM;
    }
    *ret = 1;
}

static t_float seq_gettimhook(t_squiter *it, int *ret)
{
    t_seqevent *sev = it->i_element;
    *ret = 1;
    return (sev->e_delta);
}

static void seq_settimhook(t_squiter *it, t_float f, int *ret)
{
    t_seqevent *sev = it->i_element;
    sev->e_delta = f;
    *ret = 1;
}

static t_symbol *seq_gettarhook(t_squiter *it, int *ret)
{
    *ret = 1;
    return (0);
}

static void seq_settarhook(t_squiter *it, t_symbol *s, int *ret)
{
    *ret = 1;
}

static int seq_make_iterator(t_seq *x, t_mifi_stream *stp)
{
    t_squiter *it = squiter_new(stp);
    if (it)
    {
	it->i_owner = x;
	it->i_nelems = x->x_nevents;
	it->i_sequence = it->i_element = x->x_sequence;
	it->i_index = 0;
	it->i_hooks[SQUITER_SEEKHOOK] = (t_squiterhook)seq_seekhook;
	it->i_hooks[SQUITER_INCRHOOK] = (t_squiterhook)seq_incrhook;
	it->i_hooks[SQUITER_GETEVEHOOK] = (t_squiterhook)seq_getevehook;
	it->i_hooks[SQUITER_SETEVEHOOK] = (t_squiterhook)seq_setevehook;
	it->i_hooks[SQUITER_GETTIMHOOK] = (t_squiterhook)seq_gettimhook;
	it->i_hooks[SQUITER_SETTIMHOOK] = (t_squiterhook)seq_settimhook;
	it->i_hooks[SQUITER_GETTARHOOK] = (t_squiterhook)seq_gettarhook;
	it->i_hooks[SQUITER_SETTARHOOK] = (t_squiterhook)seq_settarhook;
	return (1);
    }
    else return (0);
}

static t_mifi_stream *seq_makestream(t_seq *x)
{
    t_mifi_stream *stp = 0;
    if (stp = mifi_stream_new())
    {
        if (seq_make_iterator(x, stp))
	    return (stp);
	else
	    mifi_stream_free(stp);
    }
    return (0);
}

static int seq_comparehook(const void *e1, const void *e2)
{
    return (((t_seqevent *)e1)->e_delta > ((t_seqevent *)e2)->e_delta ? 1 : -1);
}

/* FIXME */
static int seq_mfread(t_seq *x, char *path)
{
    int result = 0;
    t_mifi_stream *stp = 0;
    if (!(stp = seq_makestream(x)) ||
	!mifi_read_start(stp, path, "", 0))
	goto readfailed;
#ifdef SEQ_DEBUG
    if (stp->s_nframes)
	post("midifile (format %d): %d tracks, %d ticks (%d smpte frames)",
	     stp->s_format, stp->s_hdtracks, stp->s_nticks, stp->s_nframes);
    else
	post("midifile (format %d): %d tracks, %d ticks per beat",
	     stp->s_format, stp->s_hdtracks, stp->s_nticks);
#endif
    if (mifi_read_analyse(stp) != MIFI_READ_EOF ||
	!seq_dogrowing(x, stp->s_nevents) ||
	!mifi_read_restart(stp) ||
	mifi_read_doit(stp) != MIFI_READ_EOF)
	goto readfailed;
    squmpi_sort(stp);
    qsort(x->x_sequence, stp->s_nevents, sizeof(*x->x_sequence),
	  seq_comparehook);
    sq_fold_time(stp);
#ifdef SEQ_DEBUG
    post("finished reading %d events from midifile", stp->s_nevents);
#endif
    result = 1;
readfailed:
    if (stp)
    {
	mifi_read_end(stp);
	mifi_stream_free(stp);
    }
    return (result);
}

/* FIXME */
static int seq_mfwrite_doit(t_seq *x, t_mifi_stream *stp)
{
    t_mifi_event *mev = stp->s_auxeve;
    t_seqevent *sev = x->x_sequence;
    int nevents = x->x_nevents;
    while (nevents--)
    {
	unsigned char *bp = sev->e_bytes;
	int i;
	mev->e_delay = (uint32)(sev->e_delta * stp->s_timecoef);
	mev->e_status = *bp & 0xf0;
	mev->e_channel = *bp & 0x0f;
	/* FIXME sysex continuation */
	for (i = 0, bp++; i < 3 && *bp != SEQ_EOM; i++, bp++)
	    mev->e_data[i] = *bp;
	if (!mifi_write_event(stp, mev))
	    return (0);
	sev++;
    }
    return (1);
}

/* FIXME */
static int seq_mfwrite(t_seq *x, char *path)
{
    int result = 0;
    t_mifi_stream *stp = 0;
    if (!(stp = seq_makestream(x)))
	goto writefailed;
    stp->s_ntracks = 1;
    stp->s_hdtracks = 1;
    stp->s_format = 0;
    if (!mifi_write_start(stp, path, ""))
	goto writefailed;
    mifi_event_settext(stp->s_auxeve, MIFI_META_TRACKNAME, "seq-track");
    if (!mifi_write_start_track(stp) ||
	!mifi_write_event(stp, stp->s_auxeve) ||
	!seq_mfwrite_doit(x, stp) ||
	!mifi_write_adjust_track(stp, 0))
	goto writefailed;
    result = 1;
writefailed:
    if (stp)
    {
	mifi_write_end(stp);
	mifi_stream_free(stp);
    }
    return (result);
}

/* FIXME */
/* CHECKED absolute timestamps, semi-terminated, verified */
static int seq_frombinbuf(t_seq *x, t_binbuf *bb)
{
    int nevents = 0;
    int ac = binbuf_getnatom(bb);
    t_atom *av = binbuf_getvec(bb);
    while (ac--)
	if (av++->a_type == A_SEMI)  /* FIXME parsing */
	    nevents++;
    if (nevents)
    {
	t_seqevent *ep;
	float prevtime = 0;
	int i = -1;
	if (!seq_dogrowing(x, nevents))
	    return (0);
	nevents = 0;
	ac = binbuf_getnatom(bb);
	av = binbuf_getvec(bb);
	ep = x->x_sequence;
	while (ac--)
	{
	    if (av->a_type == A_FLOAT)
	    {
		if (i < 0)
		{
		    ep->e_delta = av->a_w.w_float - prevtime;
		    prevtime = av->a_w.w_float;
		    i = 0;
		}
		else if (i < 4)
		    ep->e_bytes[i++] = av->a_w.w_float;
		/* CHECKME else */
	    }
	    else if (av->a_type == A_SEMI && i > 0)
	    {
		if (i < 4)
		    ep->e_bytes[i] = SEQ_EOM;
		nevents++;
		ep++;
		i = -1;
	    }
	    /* CHECKME else */
	    av++;
	}
	x->x_nevents = nevents;
    }
    return (nevents);
}

static void seq_tobinbuf(t_seq *x, t_binbuf *bb)
{
    int nevents = x->x_nevents;
    t_seqevent *ep = x->x_sequence;
    t_atom at[5];
    float timestamp = 0;
    while (nevents--)
    {
	unsigned char *bp = ep->e_bytes;
	int i;
	t_atom *ap = at;
	timestamp += ep->e_delta;
	SETFLOAT(ap, timestamp);  /* CHECKED same for sysex continuation */
	ap++;
	SETFLOAT(ap, *bp);
	for (i = 0, ap++, bp++; i < 3 && *bp != SEQ_EOM; i++, ap++, bp++)
	    SETFLOAT(ap, *bp);
	binbuf_add(bb, i + 2, at);
	binbuf_addsemi(bb);
	ep++;
    }
}

static void seq_textread(t_seq *x, char *path)
{
    t_binbuf *bb;
    bb = binbuf_new();
    if (binbuf_read(bb, path, "", 0))
    {
	/* CHECKED no complaint, open dialog presented */
	hammerpanel_open(x->x_filehandle);  /* LATER rethink */
    }
    else
    {
	int nlines = seq_frombinbuf(x, bb);
	if (nlines < 0)
	    /* CHECKED "bad MIDI file (truncated)" alert, even if a text file */
	    loud_error((t_pd *)x, "bad text file (truncated)");
	else if (nlines == 0)
	{
	    /* CHECKED no complaint, sequence erased, LATER rethink */
	}
    }
    binbuf_free(bb);
}

static void seq_textwrite(t_seq *x, char *path)
{
    t_binbuf *bb;
    bb = binbuf_new();
    seq_tobinbuf(x, bb);
    /* CHECKED empty sequence stored as an empty file */
    if (binbuf_write(bb, path, "", 0))
    {
	/* CHECKME complaint and FIXME */
	loud_error((t_pd *)x, "error writing text file");
    }
    binbuf_free(bb);
}

static void seq_doread(t_seq *x, t_symbol *fn, int creation)
{
    char buf[MAXPDSTRING];
    if (x->x_canvas)
	canvas_makefilename(x->x_canvas, fn->s_name, buf, MAXPDSTRING);
    else
    {
    	strncpy(buf, fn->s_name, MAXPDSTRING);
    	buf[MAXPDSTRING-1] = 0;
    }
    if (creation)
    {
	/* loading during object creation -- CHECKED no warning if a file
	   specified with an arg does not exist, LATER rethink */
	FILE *fp;
	char path[MAXPDSTRING];
	sys_bashfilename(buf, path);
	if (!(fp = fopen(path, "r")))
	    return;
	fclose(fp);
    }
    /* CHECKED all cases: arg or not, message and creation */
    post("seq: reading %s", fn->s_name);
    if (!seq_mfread(x, buf))
	seq_textread(x, buf);
}

static void seq_dowrite(t_seq *x, t_symbol *fn)
{
    char buf[MAXPDSTRING], *dotp;
    if (x->x_canvas)
	canvas_makefilename(x->x_canvas, fn->s_name, buf, MAXPDSTRING);
    else
    {
    	strncpy(buf, fn->s_name, MAXPDSTRING);
    	buf[MAXPDSTRING-1] = 0;
    }
    post("seq: writing %s", fn->s_name);  /* CHECKED arg or not */
    /* save as text for any extension other then ".mid" */
    if ((dotp = strrchr(fn->s_name, '.')) && strcmp(dotp + 1, "mid"))
	seq_textwrite(x, buf);
    else  /* save as mf for ".mid" or no extension at all, LATER rethink */
	seq_mfwrite(x, buf);
}

static void seq_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    seq_doread((t_seq *)z, fn, 0);
}

static void seq_writehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    seq_dowrite((t_seq *)z, fn);
}

static void seq_read(t_seq *x, t_symbol *s)
{
    if (s && s != &s_)
	seq_doread(x, s, 0);
    else  /* CHECKED no default */
	hammerpanel_open(x->x_filehandle);
}

static void seq_write(t_seq *x, t_symbol *s)
{
    if (s && s != &s_)
	seq_dowrite(x, s);
    else  /* CHECKED creation arg is a default */
	hammerpanel_save(x->x_filehandle,
			 canvas_getdir(x->x_canvas), x->x_defname);
}

static void seq_print(t_seq *x)
{
    int nevents = x->x_nevents;
    startpost("midiseq:");  /* CHECKED */
    if (nevents)
    {
	t_seqevent *ep = x->x_sequence;
	int truncated;
	if (nevents > 16)
	    nevents = 16, truncated = 1;
	else
	    truncated = 0;
	while (nevents--)
	{
	    unsigned char *bp = ep->e_bytes;
	    int i;
	    if (*bp < 128 || *bp == 247)
		/* CHECKED (sysex continuation) */
		startpost("\n(%d)->", ep->e_delta);
	    else
		startpost("\n(%d)", ep->e_delta);
	    /* CHECKED space-separated, no semi */
	    postfloat((float)*bp);
	    for (i = 0, bp++; i < 3 && *bp != SEQ_EOM; i++, bp++)
		postfloat((float)*bp);
	    ep++;
	}
	endpost();
	if (truncated) post("...");  /* CHECKED */
    }
    else post(" no sequence");  /* CHECKED */
}

static void seq_free(t_seq *x)
{
    if (x->x_clock) clock_free(x->x_clock);
    hammerfile_free(x->x_filehandle);
    if (x->x_sequence != x->x_seqini)
	freebytes(x->x_sequence, x->x_size * sizeof(*x->x_sequence));
}

static void *seq_new(t_symbol *s)
{
    t_seq *x = (t_seq *)pd_new(seq_class);
    static int warned = 0;
    if (!warned)
    {
	loud_warning((t_pd *)x, "seq is not ready yet");
	warned = 1;
    }
    x->x_canvas = canvas_getcurrent();
    x->x_filehandle = hammerfile_new((t_pd *)x, 0,
				     seq_readhook, seq_writehook, 0);
    x->x_tempo = 1.;
    x->x_prevtime = 0;
    x->x_size = SEQ_INISIZE;
    x->x_nevents = 0;
    x->x_sequence = x->x_seqini;
    outlet_new((t_object *)x, &s_anything);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    if (s && s != &s_)
    {
	x->x_defname = s;  /* CHECKME if 'read' changes this */
	seq_doread(x, s, 1);
    }
    else x->x_defname = &s_;
    x->x_clock = clock_new(x, (t_method)seq_clocktick);
    return (x);
}

void seq_setup(void)
{
    seq_class = class_new(gensym("seq"),
			  (t_newmethod)seq_new,
			  (t_method)seq_free,
			  sizeof(t_seq), 0,
			  A_DEFSYM, 0);
    class_addbang(seq_class, seq_bang);
    class_addfloat(seq_class, seq_float);
    /* CHECKED symbol rejected */
    class_addsymbol(seq_class, seq_symbol);
    /* CHECKED 1st atom of a list accepted if a float, ignored if a symbol */
    class_addlist(seq_class, seq_list);
    class_addmethod(seq_class, (t_method)seq_record,
		    gensym("record"), 0);
    class_addmethod(seq_class, (t_method)seq_append,
		    gensym("append"), 0);
    class_addmethod(seq_class, (t_method)seq_start,
		    gensym("start"), A_DEFFLOAT, 0);
    class_addmethod(seq_class, (t_method)seq_stop,
		    gensym("stop"), 0);
    class_addmethod(seq_class, (t_method)seq_tick,
		    gensym("tick"), 0);
    class_addmethod(seq_class, (t_method)seq_delay,
		    gensym("delay"), A_FLOAT, 0);  /* CHECKED arg obligatory */
    class_addmethod(seq_class, (t_method)seq_hook,
		    gensym("hook"), A_FLOAT, 0);  /* CHECKED arg obligatory */
    class_addmethod(seq_class, (t_method)seq_read,
		    gensym("read"), A_DEFSYM, 0);
    class_addmethod(seq_class, (t_method)seq_write,
		    gensym("write"), A_DEFSYM, 0);
    class_addmethod(seq_class, (t_method)seq_print,
		    gensym("print"), 0);
    hammerfile_setup(seq_class, 0);
}
