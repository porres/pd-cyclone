/* Copyright (c) 2007 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"
#include "sickle/sic.h"
#include "riddle.h"

/* obj_starttraverseoutlet, obj_nexttraverseoutlet, obj_noutlets,
   obj_nsiginlets, obj_nsigoutlets, obj_siginletindex, obj_sigoutletindex,
   obj_issignalinlet, obj_issignaloutlet */
#include "m_imp.h"

/* struct _glist, canvas_class */
#include "g_canvas.h"

#define RIDDLE_DEBUG

typedef struct _rdenvironment
{
    t_pd         re_pd;
    t_rdpool    *re_graphpools;
    t_rdbuffer  *re_writers;
    t_rdbuffer  *re_readers;  /* list of orphaned readers */
    t_clock     *re_updatedspclock;
} t_rdenvironment;

static t_class *rdenvironment_class = 0;

struct _rdsource
{
    t_riddle  *ri_riddle;
    int        ri_sourcecount;
    t_symbol  *ri_pattern;
    t_symbol  *ri_newpattern;
    int        ri_block;  /* if non-zero pattern: largest expected block */
    int        ri_newblock;
};

struct _rdsink
{
    t_riddle  *ro_riddle;
    int        ro_outno;
    t_symbol  *ro_pattern;
    int        ro_block;      /* if non-zero pattern: largest expected block */
    t_atom     ro_outbuf[3];  /* siginno, pattern, block */
    int        ro_isready;
    int        ro_isstrict;   /* if set: non-riddle sinks are rejected */
};

#define RDPOOL_INISIZE  64

typedef struct _rdlink
{
    t_symbol  *rl_key;
    int        rl_size;
    int        rl_maxsize;
    t_float   *rl_data;
    t_float    rl_inidata[RDPOOL_INISIZE];
    struct _rdlink  *rl_next;
} t_rdlink;

struct _rdpool
{
    t_canvas  *rp_graph;
    int        rp_refcount;
    t_rdlink  *rp_links;
    t_rdpool  *rp_prev;
    t_rdpool  *rp_next;
};

#define RDBUFFER_INISIZE  1024

struct _rdbuffer
{
    t_symbol    *rb_name;
    t_riddle    *rb_owner;
    int          rb_phase;
    t_rdbuffer  *rb_prev;
    t_rdbuffer  *rb_next;

    /* common part, copied from writer to all its readers
       immediately after any change */
    int          rb_nframes;
    int          rb_framesize;
    int          rb_npoints;
    int          rb_maxphase;
    t_float     *rb_buf;

    /* writer-specific part */
    t_rdbuffer  *rb_readers;
    int          rb_bufsize;
    t_float     *rb_bufini;
};

/* these are filled in riddle_setup() */
static t_symbol *rdps__reblock = 0;
static t_symbol *rdps__idle = 0;
static t_symbol *rdps__ = 0;

void riddlebug_post(t_riddle *rd, char *pfx, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    if (fmt)
    {
	va_start(ap, fmt);
	vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s \"%s\" (%x): %s\n",
		pfx, class_getname(*(t_pd *)rd), (int)rd, buf);
    }
    else fprintf(stderr, "%s \"%s\" (%x)\n",
		 pfx, class_getname(*(t_pd *)rd), (int)rd);
#ifdef MSW
    fflush(stderr);
#endif
}

static void riddle_updatedsptick(t_rdenvironment *re)
{
    canvas_update_dsp();
}

static void rdenvironment_anything(t_rdenvironment *re,
				   t_symbol *s, int ac, t_atom *av)
{
    /* FIXME */
}

static t_rdenvironment *rdenvironment_provide(void)
{
    t_rdenvironment *re;
    t_symbol *ps__riddle = gensym("_riddle");
    t_symbol *ps_hashriddle = gensym("#riddle");
    if (ps_hashriddle->s_thing)
    {
	char *cname = class_getname(*ps_hashriddle->s_thing);
	if (strcmp(cname, ps__riddle->s_name))
	{
	    /* FIXME protect against the danger of someone else
	       (e.g. receive) binding to #riddle */
	    loudbug_bug("rdenvironment_provide");
	}
	else
	{
	    /* FIXME compatibility test */
	    rdenvironment_class = *ps_hashriddle->s_thing;
	    return ((t_rdenvironment *)ps_hashriddle->s_thing);
	}
    }
    rdenvironment_class = class_new(ps__riddle, 0, 0,
				    sizeof(t_rdenvironment),
				    CLASS_PD | CLASS_NOINLET, 0);
    class_addanything(rdenvironment_class, rdenvironment_anything);
    re = (t_rdenvironment *)pd_new(rdenvironment_class);  /* never freed */
    re->re_graphpools = 0;
    re->re_writers = 0;
    re->re_readers = 0;
    re->re_updatedspclock = clock_new(re, (t_method)riddle_updatedsptick);
    pd_bind((t_pd *)re, ps_hashriddle);  /* never unbound */
    return (re);
}

static t_rdpool *rdpool_attach(t_rdenvironment *re, t_canvas *graph)
{
    t_rdpool *rp;
    for (rp = re->re_graphpools; rp; rp = rp->rp_next)
	if (rp->rp_graph == graph)
	    break;
    if (rp)
	rp->rp_refcount++;
    else
    {
	rp = getbytes(sizeof(*rp));
	rp->rp_graph = graph;
	rp->rp_refcount = 1;
	rp->rp_links = 0;
	rp->rp_prev = 0;
	if (re->re_graphpools)
	    re->re_graphpools->rp_prev = rp;
	rp->rp_next = re->re_graphpools;
	re->re_graphpools = rp;
    }
    return (rp);
}

static void rdpool_detach(t_rdpool *rp, t_rdenvironment *re)
{
    if (rp->rp_refcount > 1)
	rp->rp_refcount--;
    else
    {
	while (rp->rp_links)
	{
	    t_rdlink *rl = rp->rp_links;
	    rp->rp_links = rl->rl_next;
	    if (rl->rl_data)
		freebytes(rl->rl_data, rl->rl_size * sizeof(*rl->rl_data));
	    freebytes(rl, sizeof(*rl));
	}
	if (rp->rp_prev)
	    rp->rp_prev->rp_next = rp->rp_next;
	else
	    re->re_graphpools = rp->rp_next;
	if (rp->rp_next)
	    rp->rp_next->rp_prev = rp->rp_prev;
	freebytes(rp, sizeof(*rp));
    }
}

static void rdpool_setthis(t_rdpool *rp,
			   t_symbol *key, int size, t_float *data)
{
    t_rdlink *rl;
    for (rl = rp->rp_links; rl; rl = rl->rl_next)
	if (rl->rl_key == key)
	    break;
    if (!rl)
    {
	rl = getbytes(sizeof(*rl));
	rl->rl_key = key;
	rl->rl_maxsize = RDPOOL_INISIZE;
	rl->rl_data = rl->rl_inidata;
	rl->rl_next = rp->rp_links;
	rp->rp_links = rl;
    }
    if (size > rl->rl_maxsize)
	rl->rl_data = grow_nodata(&size, &rl->rl_maxsize, rl->rl_data,
				  RDPOOL_INISIZE, rl->rl_inidata,
				  sizeof(*rl->rl_data));
    rl->rl_size = size;
    memcpy(rl->rl_data, data, size * sizeof(*rl->rl_data));
}

static void rdpool_proliferate(t_rdpool *rphead, t_gobj *g,
			       t_symbol *key, int size, t_float *data)
{
    for (; g; g = g->g_next)
    {
        if (pd_class((t_pd *)g) == canvas_class)
	{
	    t_canvas *graph = (t_canvas *)g;
	    t_rdpool *rp;
	    for (rp = rphead; rp; rp = rp->rp_next)
		if (rp->rp_graph == graph)
		    break;
	    if (rp)
		rdpool_setthis(rp, key, size, data);
            rdpool_proliferate(rphead, graph->gl_list, key, size, data);
	}
    }
}

/* LATER setcache */
void rdpool_set(t_rdpool *rp, t_symbol *key, int size, t_float *data)
{
    t_rdenvironment *re = rdenvironment_provide();
    rdpool_setthis(rp, key, size, data);
    rdpool_proliferate(re->re_graphpools, (t_gobj *)rp->rp_graph,
		       key, size, data);
}

/* LATER setcache */
void rdpool_setbygraph(t_canvas *graph, t_symbol *key, int size, t_float *data)
{
    t_rdenvironment *re = rdenvironment_provide();
    t_rdpool *rp;
    for (rp = re->re_graphpools; rp; rp = rp->rp_next)
	if (rp->rp_graph == graph)
	    break;
    if (rp)
	rdpool_setthis(rp, key, size, data);
    rdpool_proliferate(re->re_graphpools, (t_gobj *)graph, key, size, data);
}

t_float *rdpool_get(t_rdpool *rp, t_symbol *key, int *sizep)
{
    t_rdlink *rl;
    for (rl = rp->rp_links; rl; rl = rl->rl_next)
    {
	if (rl->rl_key == key)
	{
	    *sizep = rl->rl_size;
	    return (rl->rl_data);
	}
    }
    return (0);
}

t_float *riddle_getlink(t_riddle *rd, t_symbol *key, int *sizep)
{
    return (rdpool_get(rd->rd_graphpool, key, sizep));
}

/* LATER traversal api */
t_canvas *riddle_getgraph(t_riddle *rd, int sigoutno)
{
    t_outlet *op;
    int outno = rd->rd_outslots[sigoutno].ro_outno;
    t_outconnect *oc = obj_starttraverseoutlet((t_object *)rd, &op, outno);
    while (oc)
    {
	t_object *dst;
	t_inlet *ip;
	int inno;
	oc = obj_nexttraverseoutlet(oc, &dst, &ip, &inno);
	if (dst)
	{
	    int siginno = obj_siginletindex(dst, inno);
	    if (siginno < 0)
	    {
		/* should not happen, LATER rethink */
		break;
	    }
	    else if (pd_class((t_pd *)dst) != canvas_class)
	    {
		loud_error((t_pd *)rd, "invalid connection (not to a canvas)");
		break;
	    }
	    else return ((t_canvas *)dst);
	}
    }
    return (0);
}


void riddle_updatedsp(void)
{
    t_rdenvironment *re = rdenvironment_provide();
    loud_warning((t_pd *)re, 0, "...trying to reconstruct the dsp chain");
    clock_delay(re->re_updatedspclock, 0);
}

int riddle_getsr(t_riddle *rd)
{
    return (rd->rd_graphsr);
}

int riddle_getgraphblock(t_riddle *rd)
{
    return (rd->rd_graphblock);
}

int riddle_getsourceblock(t_riddle *rd, int siginno)
{
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
    {
	loudbug_bug("riddle_getsourceblock");
	return (0);
    }
    else
    {
	t_rdsource *slot = (siginno >= 0 ?
			    rd->rd_inslots + siginno :
			    rd->rd_remoteslots - ++siginno);
	return (slot->ri_pattern ? 0 : slot->ri_block);
    }
}

t_symbol *riddle_getsourcepattern(t_riddle *rd, int siginno, int *maxblockp)
{
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
    {
	loudbug_bug("riddle_getsourcepattern");
	return (0);
    }
    else
    {
	t_rdsource *slot = (siginno >= 0 ?
			    rd->rd_inslots + siginno :
			    rd->rd_remoteslots - ++siginno);
	if (maxblockp)
	    *maxblockp = slot->ri_block;
	return (slot->ri_pattern);
    }
}

int riddle_getoutblock(t_riddle *rd, int sigoutno)
{
    if (sigoutno >= rd->rd_nsigoutlets || sigoutno < 0)
    {
	loudbug_bug("riddle_getoutblock");
	return (0);
    }
    else return (rd->rd_outslots[sigoutno].ro_pattern ?
		 0 : rd->rd_outslots[sigoutno].ro_block);
}

t_symbol *riddle_getoutpattern(t_riddle *rd, int sigoutno, int *maxblockp)
{
    if (sigoutno >= rd->rd_nsigoutlets || sigoutno < 0)
    {
	loudbug_bug("riddle_getoutpattern");
	return (0);
    }
    else
    {
	if (maxblockp)
	    *maxblockp = rd->rd_outslots[sigoutno].ro_block;
	return (rd->rd_outslots[sigoutno].ro_pattern);
    }
}

static void riddle_setsourceblock(t_riddle *rd, int siginno, int newblock)
{
    int slotno = (siginno < 0 ? rd->rd_nsiginlets - siginno - 1 : siginno);
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setsourceblock", "%d (%d) %d",
		   siginno, slotno, newblock);
#endif
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
	loudbug_bug("riddle_setsourceblock");
    else if (newblock <= 0)
	loud_error((t_pd *)rd,
		   "invalid source block on inlet %d: %d", siginno, newblock);
    else
    {
	t_rdsource *slot = rd->rd_inslots + slotno;
	/* LATER if (slot->ri_newpattern) complain */
	if (newblock == slot->ri_newblock)
	    slot->ri_sourcecount++;
	else if (slot->ri_sourcecount > 0)
	    loud_error((t_pd *)rd,
		       "source block mismatch on inlet %d: %d != %d",
		       siginno, newblock, slot->ri_newblock);
	else
	{
	    slot->ri_newblock = newblock;
	    slot->ri_sourcecount = 1;
	    if (siginno < 0)
		rd->rd_remotesource = 1;
	}
    }
}

/* LATER pattern validation and normalization (e.g. xXXX -> aAAA */
static void riddle_setsourcepattern(t_riddle *rd, int siginno,
				    t_symbol *newpattern, int maxblock)
{
    int slotno = (siginno < 0 ? rd->rd_nsiginlets - siginno - 1 : siginno);
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setsourcepattern", "%d (%d) %s %d",
		   siginno, slotno, newpattern->s_name, maxblock);
#endif
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
	loudbug_bug("riddle_setsourcepattern");
    else
    {
	t_rdsource *slot = rd->rd_inslots + slotno;
	if (newpattern == slot->ri_newpattern)
	    slot->ri_sourcecount++;
	else if (slot->ri_sourcecount > 0)
	{
	    if (slot->ri_newpattern)
		loud_error((t_pd *)rd,
			   "source pattern mismatch on inlet %d: %s != %s",
			   siginno, newpattern->s_name,
			   slot->ri_newpattern->s_name);
	    else
		loud_error((t_pd *)rd,
			   "source pattern/block mismatch on inlet %d");
	}
	else
	{
	    slot->ri_newpattern = newpattern;
	    if (maxblock)
	    {
		if (maxblock > slot->ri_newblock)
		    slot->ri_newblock = maxblock;
	    }
	    else slot->ri_newblock = rd->rd_graphblock;
	    slot->ri_sourcecount = 1;
	    if (siginno < 0)
		rd->rd_remotesource = 1;
	}
    }
}

void riddle_setoutblock(t_riddle *rd, int sigoutno, int block)
{
    t_rdsink *slot;
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setoutblock", "%d %d", sigoutno, block);
#endif
    if (sigoutno >= rd->rd_nsigoutlets)
	loudbug_bug("riddle_setoutblock");
    else if (sigoutno < 0)
    {
	for (sigoutno = 0, slot = rd->rd_outslots;
	     sigoutno < rd->rd_nsigoutlets; sigoutno++, slot++)
	{
	    slot->ro_pattern = 0;
	    slot->ro_block = block;
	    slot->ro_outbuf[1].a_w.w_symbol = rdps__;
	    slot->ro_outbuf[2].a_w.w_float = (t_float)block;
	    slot->ro_isready = 1;
	}
    }
    else
    {
	slot = rd->rd_outslots + sigoutno;
	slot->ro_pattern = 0;
	slot->ro_block = block;
	slot->ro_outbuf[1].a_w.w_symbol = rdps__;
	slot->ro_outbuf[2].a_w.w_float = (t_float)block;
	slot->ro_isready = 1;
    }
}

void riddle_setoutpattern(t_riddle *rd, int sigoutno,
			  t_symbol *pattern, int maxblock)
{
    t_rdsink *slot;
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setoutpattern", "%d %s %d",
		   sigoutno, pattern->s_name, maxblock);
#endif
    if (sigoutno >= rd->rd_nsigoutlets)
	loudbug_bug("riddle_setoutpattern");
    else if (sigoutno < 0)
    {
	for (sigoutno = 0, slot = rd->rd_outslots;
	     sigoutno < rd->rd_nsigoutlets; sigoutno++, slot++)
	{
	    slot->ro_pattern = pattern;
	    slot->ro_block = maxblock;
	    slot->ro_outbuf[1].a_w.w_symbol = pattern;
	    slot->ro_outbuf[2].a_w.w_float = (t_float)maxblock;
	    slot->ro_isready = 1;
	}
    }
    else
    {
	slot = rd->rd_outslots;
	slot->ro_pattern = pattern;
	slot->ro_block = maxblock;
	slot->ro_outbuf[1].a_w.w_symbol = pattern;
	slot->ro_outbuf[2].a_w.w_float = (t_float)maxblock;
	slot->ro_isready = 1;
    }
}

int riddle_checksourceblock(t_riddle *rd, int siginno, int reqblock)
{
    int block = riddle_getsourceblock(rd, siginno);
    if (block == reqblock)
	return (1);
    else
    {
	if (!rd->rd_wasdisabled && rd->rd_inslots[siginno].ri_sourcecount)
	    loud_error((t_pd *)rd,
		       "invalid source block on inlet %d: %d (%d expected)",
		       siginno, block, reqblock);
	rd->rd_disabled = 1;
	return (0);
    }
}

/* LATER pattern normalization (e.g. xXXX -> aAAA */
int riddle_checksourcepattern(t_riddle *rd, int siginno,
			      t_symbol *reqpattern, int *maxblockp)
{
    t_symbol *pattern = riddle_getsourcepattern(rd, siginno, maxblockp);
    if (pattern == reqpattern)
	return (1);
    else
    {
	if (!rd->rd_wasdisabled && rd->rd_inslots[siginno].ri_sourcecount)
	{
	    if (pattern)
		loud_error((t_pd *)rd,
			   "wrong source pattern on inlet %d: %s (%s expected)",
			   siginno, pattern->s_name, reqpattern->s_name);
	    else
		loud_error((t_pd *)rd,
			   "invalid source on inlet %d: pattern %s expected",
			   siginno, reqpattern->s_name);
	}
	rd->rd_disabled = 1;
	return (0);
    }
}

int riddle_checkanysource(t_riddle *rd, int siginno)
{
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
	loudbug_bug("riddle_checkanysource");
    else
    {
	t_rdsource *slot = (siginno >= 0 ?
			    rd->rd_inslots + siginno :
			    rd->rd_remoteslots - ++siginno);
	if (slot->ri_sourcecount > 0)
	    return (1);
    }
    rd->rd_disabled = 1;
    return (0);
}

int riddle_isdisabled(t_riddle *rd)
{
    return (rd->rd_disabled);
}

void riddle_disable(t_riddle *rd)
{
    rd->rd_disabled = 1;
}

static int riddle_validatesinks(t_riddle *rd)
{
    t_object *x = (t_object *)rd;
    int sigoutno, outno, nouts = obj_noutlets(x);
    for (sigoutno = 0, outno = 0; outno < nouts; outno++)
    {
	if (obj_issignaloutlet(x, outno))
	{
	    if (sigoutno < rd->rd_nsigoutlets)
	    {
		if (rd->rd_outslots[sigoutno].ro_outno != outno)
		{
		    if (rd->rd_outslots[sigoutno].ro_outno < 0)
			rd->rd_outslots[sigoutno].ro_outno = outno;
		    else
		    {
			loudbug_bug("riddle_validatesinks 1");
			return (0);
		    }
		}
	    }
	    else
	    {
		loudbug_bug("riddle_validatesinks 2");
		/* LATER grow */
		return (0);
	    }
	    sigoutno++;
	}
    }
    if (sigoutno < rd->rd_nsigoutlets)
    {
	loudbug_bug("riddle_validatesinks 3");
	/* LATER shrink */
	return (0);
    }
    return (1);
}

static int rdsink_push(t_rdsink *slot)
{
    int result = 1;
    t_object *x = (t_object *)slot->ro_riddle;
    t_outlet *op;
    t_outconnect *oc = obj_starttraverseoutlet(x, &op, slot->ro_outno);
    while (oc)
    {
	t_object *dst;
	t_inlet *ip;
	int inno;
	oc = obj_nexttraverseoutlet(oc, &dst, &ip, &inno);
	if (dst)
	{
	    int siginno = obj_siginletindex(dst, inno);
	    if (siginno < 0)
	    {
		/* should not happen, LATER rethink */
	    }
	    /* LATER unify isriddle test as zgetfn(..., gensym("dspblock")) */
	    else if (zgetfn((t_pd *)dst, rdps__reblock))
	    {
		slot->ro_outbuf->a_w.w_float = (t_float)siginno;
		typedmess((t_pd *)dst, rdps__reblock, 3, slot->ro_outbuf);
	    }
	    else if (slot->ro_isstrict)
	    {
		char *dstname = class_getname(*(t_pd *)dst);
		if (strcmp(dstname, "print~"))
		{
		    loud_error((t_pd *)x, "not a riddle: \"%s\"", dstname);
		    result = 0;
		}
	    }
	}
    }
    return (result);
}

void riddle_mute(t_riddle *rd, t_signal **sp)
{
    int i, j, nouts = obj_nsigoutlets((t_object *)rd);
    t_rdsink *slot = rd->rd_outslots;
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "MUTE", 0);
#endif
    if (rd->rd_nsigoutlets != nouts)
    {
	loudbug_bug("riddle_mute");
	riddle_validatesinks(rd);
	if (rd->rd_nsigoutlets != nouts)
	    return;
    }
    i = 0;
    j = obj_nsiginlets((t_object *)rd);
    while (nouts--)
    {
	slot->ro_pattern = 0;
	slot->ro_block = sp[j]->s_n;
	slot->ro_outbuf[1].a_w.w_symbol = rdps__;
	slot->ro_outbuf[2].a_w.w_float = (t_float)slot->ro_block;
	slot->ro_isready = 1;
	dsp_add_zero(sp[j]->s_vec, sp[j]->s_n);
	i++; j++;
	slot++;
    }
}

static void riddle_dsp(t_riddle *rd, t_signal **sp)
{
    int failed = 0, unarmed = 1, doreblock = 0;
    int oldgraphsr = rd->rd_graphsr;
    int oldgraphblock = rd->rd_graphblock;
    int inslotno, ninslots = rd->rd_nsiginlets + rd->rd_nremoteslots;
    int outslotno;
    t_rdsource *inslot;
    t_rdsink *outslot;

#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "\nriddle_dsp", 0);
    for (inslotno = 0, inslot = rd->rd_inslots;
	 inslotno < ninslots; inslotno++, inslot++)
	loudbug_post("%d sources: %d reblocks of %d -> %d",
		     inslotno, inslot->ri_sourcecount,
		     inslot->ri_block, inslot->ri_newblock);
#endif

    rd->rd_graphsr = (int)sp[0]->s_sr;
    rd->rd_graphblock = sp[0]->s_n;

    if (rd->rd_wasdisabled = rd->rd_disabled)
    {
	rd->rd_disabled = 0;
	if (rd->rd_blockfn)
	    doreblock = 1;
	else
	{
	    loudbug_bug("riddle_dsp 2");
	    goto muteandreset;
	}
    }

    /* verify all source slots */
    for (inslotno = 0, inslot = rd->rd_inslots;
	 inslotno < ninslots; inslotno++, inslot++)
    {
	if (inslot->ri_newblock > rd->rd_graphblock)
	{
	    loud_error((t_pd *)rd,
		       "inslot %d: source block too large (%d > %d)",
		       inslotno, inslot->ri_newblock, rd->rd_graphblock);
	    failed = 1;
	}
	else if (inslot->ri_sourcecount <= 0)
	{
	    if (rd->rd_remotesource)  /* this is an `issticky' flag -- it tells
					 if unconfirmed sourceblock should be
					 preserved or bashed to graphblock */
	    {
		inslot->ri_newpattern = inslot->ri_pattern;
		if (inslot->ri_block > 0)
		    inslot->ri_newblock = inslot->ri_block;
		else
		    inslot->ri_newblock = rd->rd_graphblock;
	    }
	    else
	    {
		inslot->ri_newpattern = 0;
		inslot->ri_newblock = rd->rd_graphblock;
	    }
	}
	else if (inslot->ri_newblock <= 0)  /* should not happen */
	{
	    loudbug_bug("riddle_dsp 1");
	    failed = 1;
	}
    }
    if (failed)
	goto muteandreset;

    /* determine outblock sizes -- blockfn fires on the very first call
       to riddle_dsp(), and then after any change of block or sr,
       and each time the object is disabled...
       LATER reconsider reblocking during every dsp call... */
    if (!doreblock && rd->rd_blockfn)
    {
	if (rd->rd_graphsr != oldgraphsr ||
	    rd->rd_graphblock != oldgraphblock)
	    doreblock = 1;
	else for (inslotno = 0, inslot = rd->rd_inslots;
		  inslotno < ninslots; inslotno++, inslot++)
	{
	    if (inslot->ri_newpattern != inslot->ri_pattern ||
		inslot->ri_newblock != inslot->ri_block)
	    {
		doreblock = 1;
		break;
	    }
	}
    }

    if (doreblock || !rd->rd_blockfn)
    {
	for (outslotno = 0, outslot = rd->rd_outslots;
	     outslotno < rd->rd_nsigoutlets; outslotno++, outslot++)
	{
	    outslot->ro_pattern = 0;
	    outslot->ro_block = 0;
	    outslot->ro_isready = 0;
	}
	for (inslotno = 0, inslot = rd->rd_inslots;
	     inslotno < ninslots; inslotno++, inslot++)
	{
	    inslot->ri_pattern = inslot->ri_newpattern;
	    inslot->ri_block = inslot->ri_newblock;
	}
    }

    if (doreblock)
    {
#ifdef RIDDLE_DEBUG
	riddlebug_post(rd, "REBLOCK", 0);
#endif
	rd->rd_blockfn(rd);
	if (rd->rd_disabled)
	    goto muteandreset;
    }

    for (outslotno = 0, outslot = rd->rd_outslots;
	 outslotno < rd->rd_nsigoutlets; outslotno++, outslot++)
    {
	if (outslot->ro_block < 0)
	{
	    loudbug_bug("riddle_dsp 3");
	    failed = 1;
	}
	else if (outslot->ro_block == 0)
	    outslot->ro_block = rd->rd_graphblock;
    }
    if (failed)
	goto muteandreset;

#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "PUSH", 0);
#endif
    if (riddle_validatesinks(rd))
    {
	for (outslotno = 0, outslot = rd->rd_outslots;
	     outslotno < rd->rd_nsigoutlets; outslotno++, outslot++)
	    if (outslot->ro_isready && rdsink_push(outslot) == 0)
		failed = 1;
    }
    else failed = 1;
    if (failed)
	goto muteandreset;

    if (rd->rd_dspfn)
    {
	rd->rd_dspfn(rd, sp);
	unarmed = 0;
    }
    else loudbug_bug("riddle_dsp 4");

muteandreset:
    if (unarmed)
    {
	rd->rd_disabled = 1;
	riddle_mute(rd, sp);
    }
    for (inslotno = 0, inslot = rd->rd_inslots;
	 inslotno < ninslots; inslotno++, inslot++)
    {
	inslot->ri_newpattern = 0;
	inslot->ri_newblock = 0;
	inslot->ri_sourcecount = 0;
    }
    rd->rd_remotesource = 0;
}

static void riddle__reblock(t_riddle *rd,
			    t_symbol *pattern, t_floatarg f1, t_floatarg f2)
{
    if (pattern == rdps__)
	riddle_setsourceblock(rd, (int)f1, (int)f2);
    /* LATER replace with full validation done in riddle_setsourcepattern() */
    else if (pattern && *pattern->s_name == 'a')
	riddle_setsourcepattern(rd, (int)f1, pattern, (int)f2);
    else
	loud_error((t_pd *)rd, "bad arguments to '_reblock'");
}

static void riddle_free(t_riddle *rd)
{
    int nslots;
    t_gotfn freefn = zgetfn((t_pd *)rd, gensym("_free"));
    if (freefn)
	freefn(rd);

    rdpool_detach(rd->rd_graphpool, rdenvironment_provide());

    nslots = rd->rd_nsiginlets + rd->rd_nremoteslots;
    if (rd->rd_inslots)
	freebytes(rd->rd_inslots, nslots * sizeof(*rd->rd_inslots));
    if (rd->rd_outslots)
	freebytes(rd->rd_outslots,
		  rd->rd_nsigoutlets * sizeof(*rd->rd_outslots));
}

typedef t_pd *(*t_newgimme)(t_symbol *s, int argc, t_atom *argv);

static void *riddle_new(t_symbol *s, int ac, t_atom *av)
{
    /* IFBUILTIN remove: this is a bad hack */
    t_rdenvironment *re = rdenvironment_provide();
    t_newgimme newfn = (t_newgimme)zgetfn((t_pd *)re, s);
    if (!newfn)
    {
	loudbug_bug("riddle_new 1");
	return (0);
    }
    else
    {
	t_riddle *rd = (t_riddle *)newfn(s, ac, av);
	int i, nslots;
	t_rdsource *inslot;
	t_rdsink *outslot;
	if (!rd)
	    return (0);
	rd->rd_disabled = 0;
	rd->rd_wasdisabled = 0;
	rd->rd_blockfn = (t_rdblockfn)zgetfn((t_pd *)rd, gensym("dspblock"));
	rd->rd_dspfn = (t_rddspfn)zgetfn((t_pd *)rd, gensym("_dsp"));
	if (!rd->rd_dspfn)
	    loudbug_bug("riddle_new 2");

	rd->rd_graphpool = rdpool_attach(re, canvas_getcurrent());

	rd->rd_graphsr = (int)sys_getsr();
	rd->rd_graphblock = sys_getblksize();
	rd->rd_nsiginlets = obj_nsiginlets((t_object *)rd);
	rd->rd_nsigoutlets = obj_nsigoutlets((t_object *)rd);
	nslots = rd->rd_nsiginlets + rd->rd_nremoteslots;

	rd->rd_inslots = getbytes(nslots * sizeof(*rd->rd_inslots));
	for (i = 0, inslot = rd->rd_inslots; i < nslots; i++, inslot++)
	{
	    inslot->ri_riddle = rd;
	    inslot->ri_sourcecount = 0;
	    inslot->ri_pattern = 0;
	    inslot->ri_newpattern = 0;
	    inslot->ri_block = 0;
	    inslot->ri_newblock = 0;
	}
	rd->rd_remoteslots = rd->rd_inslots + rd->rd_nsiginlets;

	rd->rd_outslots =
	    getbytes(rd->rd_nsigoutlets * sizeof(*rd->rd_outslots));
	for (i = 0, outslot = rd->rd_outslots;
	     i < rd->rd_nsigoutlets; i++, outslot++)
	{
	    outslot->ro_riddle = rd;
	    outslot->ro_outno = -1;
	    outslot->ro_pattern = 0;
	    outslot->ro_block = 0;
	    outslot->ro_outbuf[0].a_type = A_FLOAT;
	    outslot->ro_outbuf[1].a_type = A_SYMBOL;
	    outslot->ro_outbuf[1].a_w.w_symbol = rdps__;
	    outslot->ro_outbuf[2].a_type = A_FLOAT;
	    outslot->ro_isready = 0;
	    outslot->ro_isstrict = 0;  /* LATER rethink */
	}
	riddle_validatesinks(rd);

	/* currently, rd->rd_nremoteslots is incremented in
	   rdbuffer_newreader(), LATER rethink */

	rd->rd_remotesource = 0;
	return (rd);
    }
}

/* IFBUILTIN remove: classes would use explicit class_addmethod calls */
t_class *riddle_setup(t_symbol *name, t_newmethod newfn, t_method freefn,
		      size_t sz, t_method floatfn,
		      t_rdblockfn blockfn, t_rddspfn dspfn)
{
    t_class *c = class_new(name, (t_newmethod)riddle_new,
			   (t_method)riddle_free, sz, 0, A_GIMME, 0);

    /* IFBUILTIN remove: this is a bad hack */
    t_rdenvironment *re = rdenvironment_provide();
    class_addmethod(*(t_pd *)re, (t_method)newfn, name, 0);

    if (strlen(name->s_name) < 60)
    {
	char rdstr[64];
	sprintf(rdstr, "rd.%s", name->s_name);
	class_addcreator((t_newmethod)riddle_new, gensym(rdstr), A_GIMME, 0);
	class_addmethod(*(t_pd *)re, (t_method)newfn, gensym(rdstr), 0);
    }

    rdps__reblock = gensym("_reblock");
    rdps__idle = gensym("_idle");
    rdps__ = gensym("_");

    sic_setup(c, riddle_dsp, floatfn);
    class_addmethod(c, (t_method)blockfn, gensym("dspblock"), 0);
    /* IFBUILTIN "_dsp" -> "dsp" */
    class_addmethod(c, (t_method)dspfn, gensym("_dsp"), 0);
    /* IFBUILTIN remove these two */
    class_addmethod(c, (t_method)newfn, gensym("_new"), 0);
    class_addmethod(c, (t_method)freefn, gensym("_free"), 0);
    class_addmethod(c, (t_method)riddle__reblock,
		    rdps__reblock, A_FLOAT, A_SYMBOL, A_FLOAT, 0);
    return (c);
}

static t_rdbuffer *rdenvironment_getbuffer(t_rdenvironment *re, t_symbol *name)
{
    t_rdbuffer *rb = re->re_writers;
    while (rb)
    {
	if (rb->rb_name == name)
	    return (rb);
	rb = rb->rb_next;
    }
    return (0);
}

t_rdbuffer *riddle_getbuffer(t_symbol *name)
{
    t_rdenvironment *re = rdenvironment_provide();
    return (rdenvironment_getbuffer(re, name));
}

t_rdbuffer *rdbuffer_getwriter(t_rdbuffer *rb)
{
    t_rdenvironment *re = rdenvironment_provide();
    return (rdenvironment_getbuffer(re, rb->rb_name));
}

static void rdbuffer_updatereaders(t_rdbuffer *rb)
{
    t_rdbuffer *reader;
    for (reader = rb->rb_readers; reader; reader = reader->rb_next)
    {
	reader->rb_nframes = rb->rb_nframes;
	reader->rb_framesize = rb->rb_framesize;
	reader->rb_npoints = rb->rb_npoints;
	reader->rb_maxphase = rb->rb_maxphase;
	reader->rb_buf = rb->rb_buf;
	reader->rb_phase = 0;  /* LATER adjust */
	if (reader->rb_owner)
	    /* FIXME -1 - reader->rb_slotno */
	    riddle_setsourceblock(reader->rb_owner, -1, rb->rb_framesize);
    }
}

/* FIXME return the actually allocated size */
/* LATER optionally use old contents by zero-padding, interpolating, etc. */
void rdbuffer_validate(t_rdbuffer *rb, int nblock)
{
    if (rb->rb_bufini == 0)
    {
	loudbug_bug("rdbuffer_validate 1");
	return;
    }
    rb->rb_npoints = nblock * rb->rb_nframes;
    if (rb->rb_npoints > rb->rb_bufsize)
    {
	int reqsize = rb->rb_npoints;
	/* LATER use grow_withdata() */
	rb->rb_buf = grow_nodata(&rb->rb_npoints, &rb->rb_bufsize, rb->rb_buf,
				 RDBUFFER_INISIZE, rb->rb_bufini,
				 sizeof(*rb->rb_buf));
	if (rb->rb_npoints != reqsize)
	{
	    rb->rb_nframes = rb->rb_npoints / nblock;
	    if (rb->rb_nframes < 1)
	    {
		loudbug_bug("rdbuffer_validate 2");
		rb->rb_nframes = 1;
		nblock = rb->rb_npoints;
	    }
	}
    }
    /* LATER convert old buffer's contents of rb->rb_framesize * rb->rb_nframes
       points into the new buffer of nblock * rb->rb_nframes points */
    memset(rb->rb_buf, 0, rb->rb_npoints * sizeof(*rb->rb_buf));
    rb->rb_phase = 0;  /* LATER adjust */
    rb->rb_maxphase = rb->rb_npoints - nblock;
    rb->rb_framesize = nblock;
    rdbuffer_updatereaders(rb);
}

void rdbuffer_reset(t_rdbuffer *rb)
{
    if (rb->rb_bufini)
	memset(rb->rb_buf, 0, rb->rb_npoints * sizeof(*rb->rb_buf));
    rb->rb_phase = 0;
}

int rdbuffer_getframesize(t_rdbuffer *rb)
{
    return (rb->rb_framesize);
}

t_float *rdbuffer_gethead(t_rdbuffer *rb)
{
    return (rb->rb_buf + rb->rb_phase);
}

void rdbuffer_stephead(t_rdbuffer *rb)
{
    rb->rb_phase += rb->rb_framesize;
    if (rb->rb_phase > rb->rb_maxphase)
	rb->rb_phase = 0;
}

void rdbuffer_movehead(t_rdbuffer *rb, int nframes)
{
    if (rb->rb_nframes <= 0)
    {
	loudbug_bug("rdbuffer_movehead");
    }
    else if (nframes > 0)
    {
	if (nframes >= rb->rb_nframes)
	    nframes = rb->rb_nframes - 1;
	rb->rb_phase += nframes * rb->rb_framesize;
	while (rb->rb_phase > rb->rb_maxphase)
	    rb->rb_phase -= rb->rb_npoints;
    }
    else if (nframes < 0)
    {
	nframes = -nframes;
	if (nframes >= rb->rb_nframes)
	    nframes = rb->rb_nframes - 1;
	rb->rb_phase -= nframes * rb->rb_framesize;
	while (rb->rb_phase < 0)
	    rb->rb_phase += rb->rb_npoints;
    }
}

void rdbuffer_delayhead(t_rdbuffer *rb, int nframes)
{
    if (rb->rb_bufini)
	loudbug_bug("rdbuffer_delayhead 1");
    else
    {
	t_rdbuffer *writer = riddle_getbuffer(rb->rb_name);
	if (writer)
	{
	    rb->rb_phase = writer->rb_phase;
	    rdbuffer_movehead(rb, -nframes);
	}
	else loudbug_bug("rdbuffer_delayhead 2");
    }
}

void rdbuffer_free(t_rdbuffer *rb)
{
    if (rb->rb_bufini)
    {
	if (rb->rb_buf != rb->rb_bufini)
	    freebytes(rb->rb_buf, rb->rb_bufsize * sizeof(*rb->rb_buf));
	if (rb->rb_name)
	{
	    t_rdbuffer *reader;
	    t_rdenvironment *re = rdenvironment_provide();
	    /* remove from the environment */
	    if (rb->rb_next)
		rb->rb_next->rb_prev = rb->rb_prev;
	    if (rb->rb_prev)
		rb->rb_prev->rb_next = rb->rb_next;
	    else
		re->re_writers = rb->rb_next;
	    /* move all readers to the orphanage */
	    if (reader = rb->rb_readers)
	    {
		while (reader->rb_next)
		    reader = reader->rb_next;
		if (re->re_readers)
		    re->re_readers->rb_prev = reader;
		reader->rb_next = re->re_readers;
		re->re_readers = rb->rb_readers;
	    }
	}
    }
    else
    {
	if (rb->rb_name)
	{
	    /* remove from writer's list or orphanage */
	    if (rb->rb_next)
		rb->rb_next->rb_prev = rb->rb_prev;
	    if (rb->rb_prev)
		rb->rb_prev->rb_next = rb->rb_next;
	    else
	    {
		t_rdenvironment *re = rdenvironment_provide();
		t_rdbuffer *writer = rdenvironment_getbuffer(re, rb->rb_name);
		if (writer)
		    writer->rb_readers = rb->rb_next;
		else
		    re->re_readers = rb->rb_next;
	    }
	}
    }
    freebytes(rb, sizeof(*rb));
}

t_rdbuffer *rdbuffer_new(t_riddle *owner, t_symbol *name, int nframes)
{
    t_rdbuffer *rb = (t_rdbuffer *)getbytes(sizeof(*rb));
    rb->rb_name = (name && *name->s_name ? name : 0);
    rb->rb_owner = owner;
    rb->rb_phase = 0;
    rb->rb_nframes = (nframes > 1 ? nframes : 1);
    rb->rb_framesize = 0;
    rb->rb_npoints = 0;
    rb->rb_maxphase = 0;
    rb->rb_bufsize = RDBUFFER_INISIZE;
    rb->rb_bufini = getbytes(rb->rb_bufsize * sizeof(*rb->rb_bufini));
    rb->rb_buf = rb->rb_bufini;
    rb->rb_readers = 0;
    rb->rb_prev = 0;
    rb->rb_next = 0;
    if (rb->rb_name)
    {
	t_rdenvironment *re = rdenvironment_provide();
	if (rdenvironment_getbuffer(re, rb->rb_name))
	{
	    loud_error((t_pd *)owner, "duplicate buffer name \"%s\"",
		       rb->rb_name->s_name);
	}
	else
	{
	    t_rdbuffer *reader;
	    /* store in the environment */
	    if (re->re_writers)
		re->re_writers->rb_prev = rb;
	    rb->rb_next = re->re_writers;
	    re->re_writers = rb;
	    /* recover readers from the orphanage */
	    for (reader = re->re_readers; reader; reader = reader->rb_next)
	    {
		if (reader->rb_name == rb->rb_name)
		{
		    if (reader->rb_next)
			reader->rb_next->rb_prev = reader->rb_prev;
		    if (reader->rb_prev)
			reader->rb_prev->rb_next = reader->rb_next;
		    else
			re->re_readers = reader->rb_next;
		    if (rb->rb_readers)
			rb->rb_readers->rb_prev = reader;
		    reader->rb_next = rb->rb_readers;
		    rb->rb_readers = reader;
		}
	    }
	}
    }
    return (rb);
}

t_rdbuffer *rdbuffer_newreader(t_riddle *owner, t_symbol *name)
{
    t_rdbuffer *rb = (t_rdbuffer *)getbytes(sizeof(*rb));
    /* FIXME do not rely on pd_new() callocing owner->rd_nremoteslots to zero */
    owner->rd_nremoteslots++;
    rb->rb_name = (name && *name->s_name ? name : 0);
    rb->rb_owner = owner;
    rb->rb_phase = 0;
    rb->rb_nframes = 0;
    rb->rb_framesize = 0;
    rb->rb_npoints = 0;
    rb->rb_maxphase = 0;
    rb->rb_buf = 0;
    rb->rb_bufini = 0;
    rb->rb_prev = 0;
    rb->rb_next = 0;
    if (rb->rb_name)
    {
	t_rdenvironment *re = rdenvironment_provide();
	t_rdbuffer *writer = rdenvironment_getbuffer(re, rb->rb_name);
	if (writer)
	{
	    /* register to the writer */
	    if (writer->rb_readers)
		writer->rb_readers->rb_prev = rb;
	    rb->rb_next = writer->rb_readers;
	    writer->rb_readers = rb;
	}
	else
	{
	    /* store in the orphanage */
	    if (re->re_readers)
		re->re_readers->rb_prev = rb;
	    rb->rb_next = re->re_readers;
	    re->re_readers = rb;
	}
    }
    return (rb);
}

int riddle_erbfill(int nbands, int *buf, int nblock, int sr)
{
    static double coef = 9.293902;  /* 21.4 / log(10) */
    double df = (double)sr / (double)nblock;
    double fmax = .5 * (nblock + 1) * df;
    double fc = df;
    int i, erbcount = 0, bincount = 0, lastbin = 0;
    int bufsize = nbands + 1;
    while (erbcount < nbands && fc < fmax)
    {
	/* the formula is taken from ~jos/bbt
	   (the results slightly differ from moore-glasberg's demos) */
	double erbnumber = coef * log(.00437 * fc + 1.);
	bincount++;
	if ((int)erbnumber > erbcount)  /* LATER rethink */
	{
	    buf[erbcount++] = bincount - lastbin;
	    lastbin = bincount;
	}
	fc += df;
    }
    for (i = erbcount; i < bufsize; i++)
	buf[i] = 0;
    return (erbcount);
}
