/* Copyright (c) 1997-2003 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKME inlet/outlet vs inlet~/outlet~ */
/* LATER think about abstractions */
/* LATER sort out escaping rules (also revisit binport.c) */
/* LATER quoting */
/* LATER resolve object names (preserve connections of unknown dummies?) */

#ifdef UNIX
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#endif
#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/binport.h"
#include "common/port.h"

//#define PORT_DEBUG
#define PORT_LOG

#define PORT_INISTACK  256  /* LATER rethink */
#define PORT_INISIZE   512  /* LATER rethink */

enum { PORT_OK,
       PORT_NEXT,  /* next line, please */
       PORT_UNKNOWN, PORT_CORRUPT, PORT_FATAL };

#define PORT_DEFFONTSIZE  10.
#define PORT_XSTRETCH      1.25
#define PORT_YSTRETCH      1.1
#define PORT_WSTRETCH      1.25

typedef struct _port
{
    t_binbuf  *x_oldbb;
    t_binbuf  *x_newbb;
    int        x_nobj;
    int        x_inatoms;
    t_atom    *x_inmess;
    int        x_outsize;
    int        x_outatoms;
    t_atom    *x_outmess;
    t_atom     x_outini[PORT_INISIZE];
    int        x_stacksize;
    int        x_stackdepth;
    int       *x_stack;
    int        x_stackini[PORT_INISTACK];
} t_port;

static t_float port_floatarg(t_port *x, int ndx)
{
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
	return (av->a_type == A_FLOAT ? av->a_w.w_float : 0);
    }
    else return (0);
}

static t_symbol *port_symbolarg(t_port *x, int ndx)
{
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
	return (av->a_type == A_SYMBOL ? av->a_w.w_symbol : &s_);
    }
    else return (&s_);
}

static int port_xstretch(float f)
{
    return ((int)(f * PORT_XSTRETCH + 0.5));
}

static int port_ystretch(float f)
{
    return ((int)(f * PORT_YSTRETCH + 0.5));
}

static int port_wstretch(float f)
{
    return ((int)(f * PORT_WSTRETCH + 0.5));
}

static t_float port_xarg(t_port *x, int ndx)
{
    return ((t_float)port_xstretch(port_floatarg(x, ndx)));
}

static t_float port_yarg(t_port *x, int ndx)
{
    return ((t_float)port_ystretch(port_floatarg(x, ndx)));
}

static t_float port_widtharg(t_port *x, int ndx)
{
    return ((t_float)port_wstretch(port_floatarg(x, ndx)));
}

static void port_setxy(t_port *x, int ndx, t_atom *ap)
{
    float f = port_xarg(x, ndx);
    SETFLOAT(ap, f);
    ndx++; ap++;
    f = port_yarg(x, ndx);
    SETFLOAT(ap, f);
}

static int import_obj(t_port *x, char *name)
{
    int ndx = (x->x_inmess[1].a_w.w_symbol == gensym("user") ? 3 : 2);
    binbuf_addv(x->x_newbb, "ssffs;",
		gensym("#X"), gensym("obj"),
		port_xarg(x, ndx), port_yarg(x, ndx + 1),
		(name ? gensym(name) :
		 x->x_inmess[ndx == 2 ? 6 : 2].a_w.w_symbol));
    x->x_nobj++;
    return (PORT_NEXT);
}

static int import_objarg(t_port *x, char *name)
{
    int ndx = (x->x_inmess[1].a_w.w_symbol == gensym("user") ? 3 : 2);
    if (x->x_inatoms > 6)
    {
	t_atom *in = x->x_inmess + ndx + 4;
	t_atom *out = x->x_outmess;
	SETSYMBOL(out, gensym("#X")); out++;
	SETSYMBOL(out, gensym("obj")); out++;
	port_setxy(x, ndx, out); out++; out++;
	if (name)
	{
	    SETSYMBOL(out, gensym(name)); out++;
	    if (ndx == 2) in++;
	}
	else *out++ = (ndx == 2 ? *in++ : x->x_inmess[2]);
	for (ndx = 7; ndx < x->x_inatoms; ndx++)
	    *out++ = *in++;
	SETSEMI(out);
	binbuf_add(x->x_newbb, x->x_inatoms - 1, x->x_outmess);
	x->x_nobj++;
	return (PORT_NEXT);
    }
    else return (PORT_CORRUPT);
}

static int imaction_vpatcher(t_port *x, char *arg)
{
    if (x->x_stackdepth >= x->x_stacksize)
    {
	int rqsz = x->x_stackdepth + 1;
	int sz = rqsz;
	x->x_stack = grow_withdata(&rqsz, &x->x_stackdepth,
				   &x->x_stacksize, x->x_stack,
				   PORT_INISTACK, x->x_stackini,
				   sizeof(*x->x_stack));
	if (rqsz != sz)
	{
	    post("too many embedded patches");
	    return (PORT_FATAL);
	}
    }
    x->x_stack[x->x_stackdepth++] = x->x_nobj;
    x->x_nobj = 0;
    binbuf_addv(x->x_newbb, "ssfffff;",
		gensym("#N"), gensym("canvas"),
		port_xarg(x, 2), port_yarg(x, 3),
		(float)port_xstretch(port_floatarg(x, 4) - port_floatarg(x, 2)),
		(float)port_ystretch(port_floatarg(x, 5) - port_floatarg(x, 3)),
		PORT_DEFFONTSIZE);
    return (PORT_NEXT);
}

static int imaction_patcher(t_port *x, char *arg)
{
    binbuf_addv(x->x_newbb, "ssffss;",
		gensym("#X"), gensym("restore"),
		port_xarg(x, 2), port_yarg(x, 3),
		gensym("pd"), port_symbolarg(x, 7));
    if (x->x_stackdepth)  /* LATER consider returning PORT_FATAL otherwise */
	x->x_stackdepth--;
    x->x_nobj = x->x_stack[x->x_stackdepth];
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_trigger(t_port *x, char *arg)
{
    int i;
    for (i = 7; i < x->x_inatoms; i++)
	if (x->x_inmess[i].a_type == A_SYMBOL &&
	    x->x_inmess[i].a_w.w_symbol == gensym("i"))
	    x->x_inmess[i].a_w.w_symbol = gensym("f");
    return (PORT_OK);
}

static int imaction_scope(t_port *x, char *name)
{
    if (x->x_inatoms > 6)
    {
	t_atom *in = x->x_inmess + 7;
	t_atom *out = x->x_outmess;
	int i, xpix, ypix;
	SETSYMBOL(out, gensym("#X")); out++;
	SETSYMBOL(out, gensym("obj")); out++;
	port_setxy(x, 3, out);
	xpix = (int)out++->a_w.w_float;
	ypix = (int)out++->a_w.w_float;
	if (name)
	{
	    SETSYMBOL(out, gensym(name)); out++;
	}
	else *out++ = x->x_inmess[2];
	port_setxy(x, 5, out);
	out++->a_w.w_float -= xpix;
	out++->a_w.w_float -= ypix;
	for (i = 7; i < x->x_inatoms; i++)
	    *out++ = *in++;
	SETSEMI(out);
	binbuf_add(x->x_newbb, x->x_inatoms + 1, x->x_outmess);
	x->x_nobj++;
	return (PORT_NEXT);
    }
    else return (PORT_CORRUPT);
}

/* width fontsize fontfamily encoding fontprops red green blue text... */
static int imaction_comment(t_port *x, char *arg)
{
    int outatoms;
    SETSYMBOL(x->x_outmess, gensym("#X"));
    SETSYMBOL(x->x_outmess + 1, gensym("obj"));
    port_setxy(x, 2, x->x_outmess + 2);
    SETSYMBOL(x->x_outmess + 4, gensym("comment"));
    if (x->x_inatoms > 5)
    {
	int i, fontsize, fontprops;
	float width = port_widtharg(x, 4);
	t_atom *ap = x->x_inmess + 5;
	SETFLOAT(x->x_outmess + 5, width);
	if (ap->a_type == A_FLOAT)
	{
	    fontsize = ((int)ap->a_w.w_float) & 0x0ff;
	    fontprops = ((int)ap->a_w.w_float) >> 8;
	}
	else fontsize = 10, fontprops = 0;
	SETFLOAT(x->x_outmess + 6, fontsize);
	SETSYMBOL(x->x_outmess + 7, gensym("helvetica"));
	SETSYMBOL(x->x_outmess + 8, gensym("?"));
	SETFLOAT(x->x_outmess + 9, fontprops);
	SETFLOAT(x->x_outmess + 10, 0);
	SETFLOAT(x->x_outmess + 11, 0);
	SETFLOAT(x->x_outmess + 12, 0);
	outatoms = x->x_inatoms + 7;
	for (i = 13; i < outatoms ; i++)
	    x->x_outmess[i] = x->x_inmess[i - 7];
    }
    else outatoms = 5;
    SETSEMI(x->x_outmess + outatoms);
    binbuf_add(x->x_newbb, outatoms + 1, x->x_outmess);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_message(t_port *x, char *arg)
{
    int i;
    SETSYMBOL(x->x_outmess, gensym("#X"));
    SETSYMBOL(x->x_outmess + 1, gensym("msg"));
    port_setxy(x, 2, x->x_outmess + 2);
    for (i = 6; i < x->x_inatoms; i++)
	x->x_outmess[i-2] = x->x_inmess[i];
    SETSEMI(x->x_outmess + x->x_inatoms - 2);
    binbuf_add(x->x_newbb, x->x_inatoms - 1, x->x_outmess);
    x->x_nobj++;
    return (PORT_NEXT);
}

/* FIXME this is no longer true */
static int imaction_inlet(t_port *x, char *arg)
{
    return (import_obj(x, (port_floatarg(x, 5) ? "inlet~" : "inlet")));
}

/* FIXME this is no longer true  */
static int imaction_outlet(t_port *x, char *arg)
{
    return (import_obj(x, (port_floatarg(x, 5) ? "outlet~" : "outlet")));
}

static int imaction_number(t_port *x, char *arg)
{
    binbuf_addv(x->x_newbb, "ssff;",
		gensym("#X"), gensym("floatatom"),
		port_xarg(x, 2), port_yarg(x, 3));
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_connect(t_port *x, char *arg)
{
    binbuf_addv(x->x_newbb, "ssffff;",
		gensym("#X"), gensym("connect"),
		x->x_nobj - port_floatarg(x, 2) - 1,
		port_floatarg(x, 3),
		x->x_nobj - port_floatarg(x, 4) - 1,
		port_floatarg(x, 5));
    return (PORT_NEXT);
}

typedef int (*t_portaction)(t_port *, char *arg);

typedef struct _portslot
{
    char              *s_name;
    int                s_index;
    t_portaction       s_action;
    char              *s_actionarg;
    struct _portnode  *s_subtree;
    t_symbol          *s_symbol;
} t_portslot;

typedef struct _portnode  /* a parser's symbol definition, sort of... */
{
    t_portslot  *n_table;
    int          n_nslots;
} t_portnode;

#define PORT_NSLOTS(slots)  (sizeof(slots)/sizeof(*(slots)))

static t_portslot imslots__N[] =
{
    { "vpatcher",   1, imaction_vpatcher, 0, 0, 0 }
};
static t_portnode imnode__N = { imslots__N, PORT_NSLOTS(imslots__N) };

static t_portslot imslots_newobj[] =
{
    { "patcher",    6, imaction_patcher, 0, 0, 0 },
    { "p",          6, imaction_patcher, 0, 0, 0 },
    /* state is embedded in #N vtable...; #T set...; */
    { "table",      6, import_obj, "Table", 0, 0 }
};
static t_portnode imnode_newobj = { imslots_newobj,
				    PORT_NSLOTS(imslots_newobj) };

/* LATER consider merging newobj and newex */
static t_portslot imslots_newex[] =
{
    { "append",     6, import_objarg, "Append", 0, 0 },
    { "biquad~",    6, import_objarg, "Biquad~", 0, 0 },
    { "change",     6, import_objarg, "Change", 0, 0 },
    { "clip",       6, import_objarg, "Clip", 0, 0 },
    { "clip~",      6, import_objarg, "Clip~", 0, 0 },
    { "key",        6, import_obj, "Key", 0, 0 },
    { "keyup",      6, import_obj, "Keyup", 0, 0 },
    { "line",       6, import_objarg, "Line", 0, 0 },
    { "line~",      6, import_objarg, "Line~", 0, 0 },
    { "poly",       6, import_objarg, "Poly", 0, 0 },
    { "snapshot~",  6, import_objarg, "Snapshot~", 0, 0 },
    { "trigger",    6, imaction_trigger, 0, 0, 0 },
    { "t",          6, imaction_trigger, 0, 0, 0 }
};
static t_portnode imnode_newex = { imslots_newex,
				   PORT_NSLOTS(imslots_newex) };

static t_portslot imslots_user[] =
{
    { "GSwitch",    2, import_objarg, "Gswitch", 0, 0 },
    { "GSwitch2",   2, import_objarg, "Ggate", 0, 0 },
    { "number~",    2, import_obj, 0, 0, 0 },
    { "scope~",     2, imaction_scope, "Scope~", 0, 0 },
    { "uslider",    2, import_obj, "vsl", 0, 0 }  /* LATER range and offset */
};
static t_portnode imnode_user = { imslots_user,
				  PORT_NSLOTS(imslots_user) };

static t_portslot imslots__P[] =
{
    { "comment",    1, imaction_comment, 0, 0, 0 },
    { "message",    1, imaction_message, 0, 0, 0 },
    { "newobj",     1, import_objarg, 0, &imnode_newobj, 0 },
    { "newex",      1, import_objarg, 0, &imnode_newex, 0 },
    { "inlet",      1, imaction_inlet, 0, 0, 0 },
    { "inlet~",     1, imaction_inlet, 0, 0, 0 },
    { "outlet",     1, imaction_outlet, 0, 0, 0 },
    { "outlet~",    1, imaction_outlet, 0, 0, 0 },
    { "number",     1, imaction_number, 0, 0, 0 },
    { "flonum",     1, imaction_number, 0, 0, 0 },
    { "button",     1, import_obj, "bng", 0, 0 },
    { "slider" ,    1, import_obj, "vsl", 0, 0 },  /* LATER range and offset */
    { "hslider",    1, import_obj, "hsl", 0, 0 },  /* LATER range and offset */
    { "toggle",     1, import_obj, "tgl", 0, 0 },
    { "user",       1, import_objarg, 0, &imnode_user, 0 },
    /* state is embedded in #N vpreset <nslots>; #X append... */
    { "preset",     1, import_obj, "preset", 0, 0 },
    /* an object created from the "Paste Picture" menu,
       state is embedded in #N picture; #K...; */
    { "vpicture",   1, import_obj, "vpicture", 0, 0 },
    { "connect",    1, imaction_connect, 0, 0, 0 },
    { "fasten",     1, imaction_connect, 0, 0, 0 }
};
static t_portnode imnode__P = { imslots__P, PORT_NSLOTS(imslots__P) };

static t_portslot imslots_[] =
{
    { "#N",         0, 0, 0, &imnode__N, 0 },
    { "#P",         0, 0, 0, &imnode__P, 0 }
};
static t_portnode imnode_ = { imslots_, PORT_NSLOTS(imslots_) };

static int port_doit(t_port *x, t_portnode *node)
{
    int nslots = node->n_nslots;
    if (nslots > 0)
    {
	t_portslot *slot = node->n_table;
	t_symbol *s = port_symbolarg(x, slot->s_index);
	while (nslots--)
	{
	    if (slot->s_symbol == s)
	    {
		if (slot->s_subtree)
		{
		    int nobj = x->x_nobj;
		    int result = port_doit(x, slot->s_subtree);
		    if (result == PORT_FATAL || result == PORT_CORRUPT ||
			result == PORT_NEXT)
			return (result);
		}
		if (slot->s_action)
		    return (slot->s_action(x, slot->s_actionarg));
		else
		    return (PORT_OK);  /* LATER rethink */
	    }
	    slot++;
	}
    }
    else bug("port_doit");
    return (PORT_UNKNOWN);
}

static void port_dochecksetup(t_portnode *node)
{
    t_portslot *slots = node->n_table;
    int i, nslots = node->n_nslots;
    for (i = 0; i < nslots; i++)
    {
	t_portnode *subtree = slots[i].s_subtree;
	slots[i].s_symbol = gensym(slots[i].s_name);
	if (subtree)
	    port_dochecksetup(subtree);
    }
}

static void port_checksetup(void)
{
    static int done = 0;
    if (!done)
    {
	port_dochecksetup(&imnode_);
	done = 1;
    }
}

static t_port *port_new(void)
{
    t_port *x = (t_port *)getbytes(sizeof(*x));
    x->x_oldbb = binbuf_new();
    x->x_outsize = PORT_INISIZE;
    x->x_outatoms = 0;
    x->x_outmess = x->x_outini;
    x->x_stacksize = PORT_INISTACK;
    x->x_stackdepth = 0;
    x->x_stack = x->x_stackini;
    return (x);
}

static void port_free(t_port *x)
{
    if (x->x_outmess != x->x_outini)
	freebytes(x->x_outmess, x->x_outsize * sizeof(*x->x_outmess));
    if (x->x_stack != x->x_stackini)
	freebytes(x->x_stack, x->x_stacksize * sizeof(*x->x_stack));
    freebytes(x, sizeof(*x));
}

static int import_binbuf(t_port *x)
{
    t_atom *av = binbuf_getvec(x->x_oldbb);
    int ac = binbuf_getnatom(x->x_oldbb);
    int startmess, endmess;
    x->x_newbb = binbuf_new();
    for (startmess = 0; startmess < ac; startmess = endmess + 1)
    {
	t_atom *mess = av + startmess, *ap;
	int i;
    	for (endmess = startmess, ap = mess;
	     ap->a_type != A_SEMI; endmess++, ap++)
	    if (endmess == ac)
		return (PORT_CORRUPT);  /* no final semi */
	if (endmess == startmess || endmess == startmess + 1
	    || mess->a_type != A_SYMBOL || mess[1].a_type != A_SYMBOL)
	{
	    startmess = endmess + 1;
	    continue;
	}
	if (mess[1].a_w.w_symbol == gensym("hidden"))
	{
	    t_symbol *sel = mess[1].a_w.w_symbol;
	    mess[1].a_w.w_symbol = mess->a_w.w_symbol;
	    startmess++;
	    mess++;
	    if (endmess == startmess + 1 || mess[1].a_type != A_SYMBOL)
	    {
		startmess = endmess + 1;
		continue;
	    }
	}
	x->x_inatoms = endmess - startmess;
	x->x_inmess = mess;
	if ((i = x->x_inatoms + 16) > x->x_outsize)  /* LATER rethink */
	{
	    int sz = i;
	    x->x_outmess = grow_nodata(&sz, &x->x_outsize, x->x_outmess,
				       PORT_INISIZE, x->x_outini,
				       sizeof(*x->x_outmess));
	    if (sz != i)
	    {
		startmess = endmess + 1;
		continue;  /* LATER rethink */
	    }
	}

	/* dollar signs in file translate to symbols,
	   LATER rethink, also #-signs */
	for (i = 0, ap = x->x_inmess; i < x->x_inatoms; i++, ap++)
	{
	    if (ap->a_type == A_DOLLAR)
	    {
		char buf[100];
		sprintf(buf, "$%d", ap->a_w.w_index);
		SETSYMBOL(ap, gensym(buf));
	    }
	    else if (ap->a_type == A_DOLLSYM)
	    {
		char buf[100];
		sprintf(buf, "$%s", ap->a_w.w_symbol->s_name);
		SETSYMBOL(ap, gensym(buf));
	    }
	}
	if (port_doit(x, &imnode_) == PORT_FATAL)
	    return (PORT_FATAL);
    }
    return (PORT_OK);
}

void import_max(char *fn, char *dir)
{
    t_port *x;
    int failure, fd, ftype;
    char buf[MAXPDSTRING], *bufp;
    t_pd *stackp = 0;
    int dspstate = canvas_suspend_dsp();
    port_checksetup();
    if ((fd = open_via_path(dir, fn, "", buf, &bufp, MAXPDSTRING, 0)) < 0)
    {
    	loud_error(0, "%s: can't open", fn);
    	return;
    }
    else close (fd);

    x = port_new();
    glob_setfilename(0, gensym(bufp), gensym(buf));
    ftype = binport_read(x->x_oldbb, bufp, buf);
    if (ftype == BINPORT_MAXTEXT || ftype == BINPORT_PDFILE)
	failure = binbuf_read(x->x_oldbb, bufp, buf, 0);
    else
	failure = (ftype != BINPORT_OK);  /* LATER rethink */
    if (failure)
    {
    	perror(fn);  /* FIXME */
	binbuf_free(x->x_oldbb);
    }
    else
    {
	if (ftype == BINPORT_PDFILE) x->x_newbb = x->x_oldbb;
	else
	{
#ifdef PORT_DEBUG
	    binbuf_write(x->x_oldbb, "import-debug.pd", "", 0);
#endif
	    import_binbuf(x);
	    binbuf_free(x->x_oldbb);
#ifdef PORT_LOG
	    binbuf_write(x->x_newbb, "import-result.pd", "", 0);
#endif
	}
	binbuf_eval(x->x_newbb, 0, 0, 0);
	binbuf_free(x->x_newbb);
    }
    port_free(x);

    glob_setfilename(0, &s_, &s_);
    canvas_resume_dsp(dspstate);
    while ((stackp != s__X.s_thing) && (stackp = s__X.s_thing))
    	vmess(stackp, gensym("pop"), "i", 1);
#if 0  /* LATER */
    pd_doloadbang();
#endif
}
