/* Copyright (c) 1997-2004 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER think about abstractions */
/* LATER sort out escaping rules (also revisit binport.c) */
/* LATER quoting */
/* LATER rethink inlet/inlet~ case */

#ifdef UNIX
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#endif
#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "unstable/forky.h"
#include "unstable/fragile.h"
#include "unstable/fringe.h"
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

/* cf binport.c */
#define A_INT  A_DEFFLOAT

/* without access to sys_defaultfont, we just mimic defs from s_main.c */
#ifdef MSW
#define PORT_DEFFONTSIZE  12.
#else
#define PORT_DEFFONTSIZE  10.
#endif

#define PORT_XSTRETCH      1.25
#define PORT_YSTRETCH      1.1
#define PORT_WSTRETCH      1.25

typedef struct _port
{
    t_binbuf  *x_inbb;
    t_binbuf  *x_outbb;
    int        x_nobj;
    int        x_withbogus;
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

static t_symbol *portps_bogus;
static t_symbol *portps_cleanup;
static t_symbol *portps_inlet;
static t_symbol *portps_outlet;

static t_float port_floatarg(t_port *x, int ndx)
{
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
	return (av->a_type == A_FLOAT ? av->a_w.w_float : 0);
    }
    else return (0);
}

static t_int port_intarg(t_port *x, int ndx)
{
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
#ifdef FIXME
	return (av->a_type == A_INT ? av->a_w.w_index : 0);
#else
	if (av->a_type == A_INT)
	    return (av->a_w.w_index);
	else if (av->a_type == A_FLOAT)
	    return ((t_int)av->a_w.w_float);
	else
	    return (0);
#endif
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
    return ((t_float)port_xstretch(port_intarg(x, ndx)));
}

static t_float port_yarg(t_port *x, int ndx)
{
    return ((t_float)port_ystretch(port_intarg(x, ndx)));
}

static t_float port_widtharg(t_port *x, int ndx)
{
    return ((t_float)port_wstretch(port_intarg(x, ndx)));
}

static void port_setxy(t_port *x, int ndx, t_atom *ap)
{
    float f = port_xarg(x, ndx);
    SETFLOAT(ap, f);
    ndx++; ap++;
    f = port_yarg(x, ndx);
    SETFLOAT(ap, f);
}

static t_atom *import_copyatoms(t_atom *out, t_atom *in, int ac)
{
    while (ac-- > 0)
    {
	if (in->a_type == A_INT)
	{
	    out->a_type = A_FLOAT;
	    out++->a_w.w_float = (float)in++->a_w.w_index;
	}
	else *out++ = *in++;
    }
    return (out);
}

static void import_addclassname(t_port *x, char *outname, t_atom *inatom)
{
    t_atom at;
    if (outname)
	SETSYMBOL(&at, gensym(outname));
    else if (inatom->a_type == A_SYMBOL)
    {
	/* LATER bash inatom to lowercase (CHECKME first) */
	t_symbol *insym = inatom->a_w.w_symbol;
	if (insym != &s_bang && insym != &s_float &&
	    insym != &s_symbol && insym != &s_list &&
	    (insym == portps_inlet || insym == portps_outlet ||
	     zgetfn(&pd_objectmaker, insym) == 0))
	     
	{
	    x->x_withbogus = 1;
	    SETSYMBOL(&at, portps_bogus);
	    binbuf_add(x->x_outbb, 1, &at);
	}
	SETSYMBOL(&at, insym);
    }
    else at = *inatom;
    binbuf_add(x->x_outbb, 1, &at);
}

static int import_obj(t_port *x, char *name)
{
    int ndx = (x->x_inmess[1].a_w.w_symbol == gensym("user") ? 3 : 2);
    binbuf_addv(x->x_outbb, "ssff",
		gensym("#X"), gensym("obj"),
		port_xarg(x, ndx), port_yarg(x, ndx + 1));
    import_addclassname(x, name, &x->x_inmess[ndx == 2 ? 6 : 2]);
    binbuf_addsemi(x->x_outbb);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int import_objarg(t_port *x, char *name)
{
    int ndx = (x->x_inmess[1].a_w.w_symbol == gensym("user") ? 3 : 2);
    if (x->x_inatoms > 6
	|| (ndx == 3 && x->x_inatoms > 4))
    {
	t_atom *out = x->x_outmess;
	SETSYMBOL(out, gensym("#X")); out++;
	SETSYMBOL(out, gensym("obj")); out++;
	port_setxy(x, ndx, out);
	binbuf_add(x->x_outbb, 4, x->x_outmess);
	import_addclassname(x, name, &x->x_inmess[ndx == 2 ? 6 : 2]);
	out = import_copyatoms(x->x_outmess, x->x_inmess + 7, x->x_inatoms - 7);
	SETSEMI(out);
	binbuf_add(x->x_outbb, x->x_inatoms - 6, x->x_outmess);
	x->x_nobj++;
	return (PORT_NEXT);
    }
    else return (PORT_CORRUPT);
}

static int imaction_N1_vpatcher(t_port *x, char *arg)
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
    binbuf_addv(x->x_outbb, "ssfffff;",
		gensym("#N"), gensym("canvas"),
		port_xarg(x, 2), port_yarg(x, 3),
		(float)port_xstretch(port_intarg(x, 4) - port_intarg(x, 2)),
		(float)port_ystretch(port_intarg(x, 5) - port_intarg(x, 3)),
		PORT_DEFFONTSIZE);
    return (PORT_NEXT);
}

/* FIXME */
static int imaction_N1_vtable(t_port *x, char *arg)
{
#if 1
    post("vtable \"%s\": size %d, range %d, coords %d %d %d %d, flags %d",
	 port_symbolarg(x, 9)->s_name,
	 port_intarg(x, 2), port_intarg(x, 8),
	 port_intarg(x, 3), port_intarg(x, 4),
	 port_intarg(x, 5), port_intarg(x, 6),
	 port_intarg(x, 7));
#endif
    return (PORT_NEXT);
}

/* FIXME */
static int imaction_N1_picture(t_port *x, char *arg)
{
#if 1
    post("picture");
#endif
    return (PORT_NEXT);
}

static int imaction_P6_patcher(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ss;", portps_cleanup, portps_cleanup);
    x->x_withbogus = 0;
    binbuf_addv(x->x_outbb, "ssffss;",
		gensym("#X"), gensym("restore"),
		port_xarg(x, 2), port_yarg(x, 3),
		gensym("pd"), port_symbolarg(x, 7));
    if (x->x_stackdepth)  /* LATER consider returning PORT_FATAL otherwise */
	x->x_stackdepth--;
    x->x_nobj = x->x_stack[x->x_stackdepth];
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P6_trigger(t_port *x, char *arg)
{
    int i;
    for (i = 7; i < x->x_inatoms; i++)
	if (x->x_inmess[i].a_type == A_SYMBOL &&
	    x->x_inmess[i].a_w.w_symbol == gensym("i"))
	    x->x_inmess[i].a_w.w_symbol = gensym("f");
    return (PORT_OK);
}

static int imaction_P2_scope(t_port *x, char *name)
{
    if (x->x_inatoms > 6)
    {
	t_atom *out = x->x_outmess;
	int i, xpix, ypix;
	SETSYMBOL(out, gensym("#X")); out++;
	SETSYMBOL(out, gensym("obj")); out++;
	port_setxy(x, 3, out);
	xpix = (int)out++->a_w.w_float;
	ypix = (int)out->a_w.w_float;
	binbuf_add(x->x_outbb, 4, x->x_outmess);
	import_addclassname(x, name, &x->x_inmess[2]);
	out = x->x_outmess;
	port_setxy(x, 5, out);
	out++->a_w.w_float -= xpix;
	out++->a_w.w_float -= ypix;
	out = import_copyatoms(out, x->x_inmess + 7, x->x_inatoms - 7);
	SETSEMI(out);
	binbuf_add(x->x_outbb, x->x_inatoms - 4, x->x_outmess);
	x->x_nobj++;
	return (PORT_NEXT);
    }
    else return (PORT_CORRUPT);
}

/* width fontsize fontfamily encoding fontprops red green blue text... */
static int imaction_P1_comment(t_port *x, char *arg)
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
	if (ap->a_type == A_INT)
	{
	    fontsize = ap->a_w.w_index & 0x0ff;
	    fontprops = ap->a_w.w_index >> 8;
	}
	else if (ap->a_type == A_FLOAT)  /* FIXME */
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
	import_copyatoms(x->x_outmess + 13, x->x_inmess + 6, x->x_inatoms - 6);
    }
    else outatoms = 5;
    SETSEMI(x->x_outmess + outatoms);
    binbuf_add(x->x_outbb, outatoms + 1, x->x_outmess);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_message(t_port *x, char *arg)
{
    int i;
    t_atom *out;
    SETSYMBOL(x->x_outmess, gensym("#X"));
    SETSYMBOL(x->x_outmess + 1, gensym("msg"));
    port_setxy(x, 2, x->x_outmess + 2);
    out = import_copyatoms(x->x_outmess + 4, x->x_inmess + 6, x->x_inatoms - 6);
    SETSEMI(out);
    binbuf_add(x->x_outbb, x->x_inatoms - 1, x->x_outmess);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_io(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssff",
		gensym("#X"), gensym("obj"),
		port_xarg(x, 2), port_yarg(x, 3));
    if (x->x_inmess[1].a_w.w_symbol == portps_inlet ||
	x->x_inmess[1].a_w.w_symbol == portps_outlet)
    {
	t_atom at;
	SETSYMBOL(&at, portps_bogus);
	binbuf_add(x->x_outbb, 1, &at);
    }
    binbuf_add(x->x_outbb, 1, &x->x_inmess[1]);
    binbuf_addsemi(x->x_outbb);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_number(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssff;",
		gensym("#X"), gensym("floatatom"),
		port_xarg(x, 2), port_yarg(x, 3));
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_connect(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssiiii;",
		gensym("#X"), gensym("connect"),
		x->x_nobj - port_intarg(x, 2) - 1,
		port_intarg(x, 3),
		x->x_nobj - port_intarg(x, 4) - 1,
		port_intarg(x, 5));
    return (PORT_NEXT);
}

/* FIXME */
static int imaction_T1_set(t_port *x, char *arg)
{
#if 1
    post("set (%d atoms from %d): %d ... %d",
	 x->x_inatoms - 3, port_intarg(x, 2),
	 port_intarg(x, 3), port_intarg(x, x->x_inatoms - 1));
#endif
    return (PORT_NEXT);
}

/* FIXME */
static int imaction_K1_replace(t_port *x, char *arg)
{
#if 1
    post("replace %d", port_intarg(x, 2));
#endif
    return (PORT_NEXT);
}

/* FIXME */
static int imaction_K1_set(t_port *x, char *arg)
{
#if 1
    post("set (%d atoms from %d): %d ... %d",
	 x->x_inatoms - 3, port_intarg(x, 2),
	 port_intarg(x, 3), port_intarg(x, x->x_inatoms - 1));
#endif
    return (PORT_NEXT);
}

typedef int (*t_portaction)(t_port *, char *arg);

typedef struct _portslot
{
    char              *s_name;
    t_portaction       s_action;
    char              *s_actionarg;
    struct _portnode  *s_subtree;
    t_symbol          *s_symbol;
} t_portslot;

typedef struct _portnode  /* a parser's symbol definition, sort of... */
{
    t_portslot  *n_table;
    int          n_nslots;
    int          n_index;
} t_portnode;

#define PORT_NSLOTS(slots)  (sizeof(slots)/sizeof(*(slots)))

static t_portslot imslots__N[] =
{
    { "vpatcher",    imaction_N1_vpatcher, 0, 0, 0 },
    { "vtable",      imaction_N1_vtable, 0, 0, 0 },
    { "picture",     imaction_N1_picture, 0, 0, 0 }
};
static t_portnode imnode__N = { imslots__N, PORT_NSLOTS(imslots__N), 1 };

static t_portslot imslots_newobj[] =
{
    { "patcher",     imaction_P6_patcher, 0, 0, 0 },
    { "p",           imaction_P6_patcher, 0, 0, 0 },
    /* state is embedded in #N vtable...; #T set...; */
    { "table",       import_obj, "Table", 0, 0 }
};
static t_portnode imnode_newobj = { imslots_newobj,
				    PORT_NSLOTS(imslots_newobj), 6 };

/* LATER consider merging newobj and newex */
static t_portslot imslots_newex[] =
{
    { "append",      import_objarg, "Append", 0, 0 },
    { "biquad~",     import_objarg, "Biquad~", 0, 0 },
    { "change",      import_objarg, "Change", 0, 0 },
    { "clip",        import_objarg, "Clip", 0, 0 },
    { "clip~",       import_objarg, "Clip~", 0, 0 },
    { "key",         import_obj, "Key", 0, 0 },
    { "keyup",       import_obj, "Keyup", 0, 0 },
    { "line",        import_objarg, "Line", 0, 0 },
    { "line~",       import_objarg, "Line~", 0, 0 },
    { "poly",        import_objarg, "Poly", 0, 0 },
    { "snapshot~",   import_objarg, "Snapshot~", 0, 0 },
    { "trigger",     imaction_P6_trigger, 0, 0, 0 },
    { "t",           imaction_P6_trigger, 0, 0, 0 },

    /* LATER rethink */
    { "Borax",       import_objarg, "Borax", 0, 0 },
    { "Bucket",      import_objarg, "Bucket", 0, 0 },
    { "Decode",      import_objarg, "Decode", 0, 0 },
    { "Histo",       import_objarg, "Histo", 0, 0 },
    { "MouseState",  import_objarg, "MouseState", 0, 0 },
    { "Peak",        import_objarg, "Peak", 0, 0 },
    { "TogEdge",     import_objarg, "TogEdge", 0, 0 },
    { "Trough",      import_objarg, "Trough", 0, 0 },
    { "Uzi",         import_objarg, "Uzi", 0, 0 }
};
static t_portnode imnode_newex = { imslots_newex,
				   PORT_NSLOTS(imslots_newex), 6 };

static t_portslot imslots_user[] =
{
    { "GSwitch",     import_objarg, "Gswitch", 0, 0 },
    { "GSwitch2",    import_objarg, "Ggate", 0, 0 },
    { "number~",     import_obj, 0, 0, 0 },
    { "scope~",      imaction_P2_scope, "Scope~", 0, 0 },
    { "uslider",     import_obj, "vsl", 0, 0 }  /* LATER range and offset */
};
static t_portnode imnode_user = { imslots_user,
				  PORT_NSLOTS(imslots_user), 2 };

static t_portslot imslots__P[] =
{
    { "comment",     imaction_P1_comment, 0, 0, 0 },
    { "message",     imaction_P1_message, 0, 0, 0 },
    { "newobj",      import_objarg, 0, &imnode_newobj, 0 },
    { "newex",       import_objarg, 0, &imnode_newex, 0 },
    { "inlet",       imaction_P1_io, 0, 0, 0 },
    { "inlet~",      imaction_P1_io, 0, 0, 0 },
    { "outlet",      imaction_P1_io, 0, 0, 0 },
    { "outlet~",     imaction_P1_io, 0, 0, 0 },
    { "number",      imaction_P1_number, 0, 0, 0 },
    { "flonum",      imaction_P1_number, 0, 0, 0 },
    { "button",      import_obj, "bng", 0, 0 },
    { "slider" ,     import_obj, "vsl", 0, 0 },  /* LATER range and offset */
    { "hslider",     import_obj, "hsl", 0, 0 },  /* LATER range and offset */
    { "toggle",      import_obj, "tgl", 0, 0 },
    { "user",        import_objarg, 0, &imnode_user, 0 },
    /* state is embedded in #N vpreset <nslots>; #X append... */
    { "preset",      import_obj, "preset", 0, 0 },
    /* an object created from the "Paste Picture" menu,
       state is embedded in #N picture; #K...; */
    { "vpicture",    import_obj, "vpicture", 0, 0 },
    { "connect",     imaction_P1_connect, 0, 0, 0 },
    { "fasten",      imaction_P1_connect, 0, 0, 0 }
};
static t_portnode imnode__P = { imslots__P, PORT_NSLOTS(imslots__P), 1 };

static t_portslot imslots__T[] =
{
    { "set",         imaction_T1_set, 0, 0, 0 }
};
static t_portnode imnode__T = { imslots__T, PORT_NSLOTS(imslots__T), 1 };

static t_portslot imslots__K[] =
{
    { "replace",     imaction_K1_replace, 0, 0, 0 },
    { "set",         imaction_K1_set, 0, 0, 0 }
};
static t_portnode imnode__K = { imslots__K, PORT_NSLOTS(imslots__K), 1 };

static t_portslot imslots_[] =
{
    { "#N",          0, 0, &imnode__N, 0 },
    { "#P",          0, 0, &imnode__P, 0 },
    { "#T",          0, 0, &imnode__T, 0 },
    { "#K",          0, 0, &imnode__K, 0 }
};
static t_portnode imnode_ = { imslots_, PORT_NSLOTS(imslots_), 0 };

static int port_doline(t_port *x, t_portnode *node)
{
    int nslots = node->n_nslots;
    if (nslots > 0)
    {
	t_portslot *slot = node->n_table;
	t_symbol *insym = port_symbolarg(x, node->n_index);
	char *inname = 0;
secondpass:
	while (nslots--)
	{
	    if (slot->s_symbol == insym
		|| (inname && loud_matchignorecase(inname, slot->s_name)))
	    {
		if (slot->s_subtree)
		{
		    int nobj = x->x_nobj;
		    int result = port_doline(x, slot->s_subtree);
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
	if (!inname)
	{
	    inname = insym->s_name;
	    nslots = node->n_nslots;
	    slot = node->n_table;
	    goto secondpass;
	}
    }
    else bug("port_doline");
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

#define BOGUS_NINLETS   23
#define BOGUS_NOUTLETS  24

typedef struct _bogus
{
    t_object   x_ob;
    t_glist   *x_glist;  /* used also as 'dirty' flag */
    int        x_bound;
    t_inlet   *x_inlets[BOGUS_NINLETS];
    t_outlet  *x_outlets[BOGUS_NOUTLETS];
    t_clock   *x_clock;
} t_bogus;

typedef struct _bogushook
{
    t_pd      x_pd;
    t_pd     *x_who;
    t_glist  *x_glist;  /* used also as 'dirty' flag */
    t_clock  *x_clock;
} t_bogushook;

static t_class *bogus_class;
static t_class *bogushook_class;

static void bogus_tick(t_bogus *x)
{
    if (x->x_bound)
    {
#ifdef PORT_DEBUG
	post("unbinding '%x'", (int)x);
#endif
	pd_unbind((t_pd *)x, portps_cleanup);
	x->x_bound = 0;
    }
}

static void bogushook_tick(t_bogushook *x)
{
    pd_free((t_pd *)x);
}

static void bogus_cleanup(t_bogus *x)
{
    if (x->x_glist)
    {
	t_text *t = (t_text *)x;
	int ac = binbuf_getnatom(t->te_binbuf);
	if (ac)
	{
	    t_atom *av = binbuf_getvec(t->te_binbuf);
	    t_binbuf *bb = binbuf_new();
	    t_inlet **ip;
	    t_outlet **op;
	    int i;
#ifdef PORT_DEBUG
	    startpost("self-adjusting");
	    binbuf_print(t->te_binbuf);
#endif
	    binbuf_add(bb, ac - 1, av + 1);
	    binbuf_free(t->te_binbuf);
	    t->te_binbuf = bb;

	    for (i = BOGUS_NINLETS, ip = x->x_inlets + BOGUS_NINLETS - 1;
		 i ; i--, ip--)
	    {
		if (forky_hasfeeders((t_object *)x, x->x_glist, i, 0))
		    break;
		else
		    inlet_free(*ip);
	    }
#ifdef PORT_DEBUG
	    post("%d inlets deleted", BOGUS_NINLETS - i);
#endif
	    for (i = 0, op = x->x_outlets + BOGUS_NOUTLETS - 1;
		 i < BOGUS_NOUTLETS; i++, op--)
	    {
		if (fragile_outlet_connections(*op))
		    break;
		else
		    outlet_free(*op);
	    }
#ifdef PORT_DEBUG
	    post("%d outlets deleted", i);
#endif
	    glist_retext(x->x_glist, t);
	}
	else bug("bogus_cleanup");
	x->x_glist = 0;
	clock_delay(x->x_clock, 0);
    }
}

static void bogus_free(t_bogus *x)
{
    if (x->x_bound) pd_unbind((t_pd *)x, portps_cleanup);
    if (x->x_clock) clock_free(x->x_clock);
}

static void *bogus_new(t_symbol *s, int ac, t_atom *av)
{
    t_bogus *x = 0;
    t_glist *glist;
    if (glist = canvas_getcurrent())
    {
    	char buf[80];
	int i;
	if (av->a_type == A_SYMBOL)
	{
	    t_pd *z;
	    if (z = forky_newobject(av->a_w.w_symbol, ac - 1, av + 1))
	    {
		t_bogushook *y = (t_bogushook *)pd_new(bogushook_class);
		y->x_who = z;
		y->x_glist = glist;
		pd_bind((t_pd *)y, portps_cleanup);
		y->x_clock = clock_new(y, (t_method)bogushook_tick);
#ifdef PORT_DEBUG
		post("reclaiming %s", av->a_w.w_symbol->s_name);
#endif
		return (z);
	    }
	}
	x = (t_bogus *)pd_new(bogus_class);
	atom_string(av, buf, 80);
	loud_error((t_pd *)x, "unknown class '%s'", buf);
	x->x_glist = glist;
	for (i = 0; i < BOGUS_NINLETS; i++)
	    x->x_inlets[i] = inlet_new((t_object *)x, (t_pd *)x, 0, 0);
	for (i = 0; i < BOGUS_NOUTLETS; i++)
	    x->x_outlets[i] = outlet_new((t_object *)x, &s_anything);
	pd_bind((t_pd *)x, portps_cleanup);
	x->x_bound = 1;
	x->x_clock = clock_new(x, (t_method)bogus_tick);
    }
    return (x);
}

static void bogushook_cleanup(t_bogushook *x)
{
    if (x->x_glist)
    {
	t_text *t = (t_text *)x->x_who;
	int ac = binbuf_getnatom(t->te_binbuf);
	if (ac > 1)
	{
	    int dorecreate = 0;
	    t_atom *av = binbuf_getvec(t->te_binbuf);
	    t_binbuf *bb = binbuf_new();
#ifdef PORT_DEBUG
	    startpost("hook-adjusting");
	    binbuf_print(t->te_binbuf);
#endif
	    ac--; av++;
	    if (av->a_type == A_SYMBOL)
	    {
		if (av->a_w.w_symbol == portps_outlet)
		{
		    if (forky_hasfeeders((t_object *)x->x_who, x->x_glist,
					 0, &s_signal))
		    {
			t_atom at;
			SETSYMBOL(&at, gensym("outlet~"));
			binbuf_add(bb, 1, &at);
			ac--; av++;
			dorecreate = 1;
		    }
		}
		else if (av->a_w.w_symbol == portps_inlet)
		{
		    /* LATER */
		}
	    }
	    if (ac) binbuf_add(bb, ac, av);
	    if (dorecreate) gobj_recreate(x->x_glist, (t_gobj *)t, bb);
	    else
	    {
		binbuf_free(t->te_binbuf);
		t->te_binbuf = bb;
		glist_retext(x->x_glist, t);
	    }
	}
	else bug("bogushook_cleanup");
	x->x_glist = 0;
	clock_delay(x->x_clock, 0);
    }
}

static void bogushook_free(t_bogushook *x)
{
#ifdef PORT_DEBUG
    post("destroing the hook of '%s'", class_getname(*x->x_who));
#endif
    pd_unbind((t_pd *)x, portps_cleanup);
    if (x->x_clock) clock_free(x->x_clock);
}

static void port_checksetup(void)
{
    static int done = 0;
    if (!done)
    {
	port_dochecksetup(&imnode_);

	portps_bogus = gensym("_port.bogus");
	portps_cleanup = gensym("_port.cleanup");
	portps_inlet = gensym("inlet");
	portps_outlet = gensym("outlet");

	if (zgetfn(&pd_objectmaker, portps_bogus) == 0)
	{
	    bogus_class = class_new(portps_bogus,
				    (t_newmethod)bogus_new,
				    (t_method)bogus_free,
				    sizeof(t_bogus), 0, A_GIMME, 0);
	    class_addmethod(bogus_class, (t_method)bogus_cleanup,
			    portps_cleanup, 0);
	    bogushook_class = class_new(gensym("_port.bogushook"), 0,
					(t_method)bogushook_free,
					sizeof(t_bogushook), CLASS_PD, 0);
	    class_addmethod(bogushook_class, (t_method)bogushook_cleanup,
			    portps_cleanup, 0);
	}
	done = 1;
    }
}

static t_port *port_new(void)
{
    t_port *x = (t_port *)getbytes(sizeof(*x));
    x->x_inbb = binbuf_new();
    x->x_outbb = 0;
    x->x_withbogus = 0;
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
    if (portps_cleanup->s_thing)
    {
	/* clean up toplevel glist */
	typedmess(portps_cleanup->s_thing, portps_cleanup, 0, 0);
	/* LATER unbind all bogus objects, and destroy all bogushooks
	   by traversing the portps_cleanup's bindlist, instead of
	   using per-object clocks.  Need to have bindlist traversal
	   in Pd API first...  Otherwise, consider fragilizing this
	   (and fragilizing grab too). */
    }
    if (x->x_outmess != x->x_outini)
	freebytes(x->x_outmess, x->x_outsize * sizeof(*x->x_outmess));
    if (x->x_stack != x->x_stackini)
	freebytes(x->x_stack, x->x_stacksize * sizeof(*x->x_stack));
    freebytes(x, sizeof(*x));
}

static int import_binbuf(t_port *x)
{
    t_atom *av = binbuf_getvec(x->x_inbb);
    int ac = binbuf_getnatom(x->x_inbb);
    int startmess, endmess;
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
	if (port_doline(x, &imnode_) == PORT_FATAL)
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
    ftype = binport_read(x->x_inbb, bufp, buf);
    /* FIXME for BINPORT_MAXTEXT use an int-preserving version of binbuf_read */
    if (ftype == BINPORT_MAXTEXT || ftype == BINPORT_PDFILE)
	failure = binbuf_read(x->x_inbb, bufp, buf, 0);
    else
	failure = (ftype != BINPORT_OK);  /* LATER rethink */
    if (failure)
    {
    	perror(fn);  /* FIXME */
	binbuf_free(x->x_inbb);
    }
    else
    {
	if (ftype == BINPORT_PDFILE) x->x_outbb = x->x_inbb;
	else
	{
#ifdef PORT_DEBUG
	    /* save as .pd (bypass export translation) */
	    binbuf_write(x->x_inbb, "import-debug.pd", "", 0);
#endif
	    x->x_outbb = binbuf_new();
	    if (import_binbuf(x) != PORT_OK)
	    {
		loud_error(0, "%s: import failed", fn);
		x->x_outbb = 0;
	    }		
	    binbuf_free(x->x_inbb);
#ifdef PORT_LOG
	    if (x->x_outbb) binbuf_write(x->x_outbb, "import-result.pd", "", 0);
#endif
	}
	if (x->x_outbb)
	{
	    binbuf_eval(x->x_outbb, 0, 0, 0);
	    binbuf_free(x->x_outbb);
	}
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
