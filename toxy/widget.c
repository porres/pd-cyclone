/* Copyright (c) 2003-2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER consider supporting a special @ini handler, also think about
   differentiating 'ini' from 'vis' */
/* LATER think about reloading method for .wid files */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "unstable/forky.h"
#include "hammer/file.h"
#include "common/props.h"
#include "toxy/scriptlet.h"
#include "widgettype.h"
#include "build_counter"

/* our proxy of the text_class (not in the API), LATER do not cheat */
static t_class *makeshift_class;

//#define WIDGET_DEBUG
//#define TOW_DEBUG

enum { WIDGET_NOVIS = 0, WIDGET_PUSHVIS, WIDGET_REVIS };

typedef struct _towentry
{
    struct _tow       *te_tow;
    struct _towentry  *te_next;
} t_towentry;

typedef struct _widgetentry
{
    struct _widget       *we_widget;
    struct _widgetentry  *we_next;
} t_widgetentry;

typedef struct _widget
{
    t_object       x_ob;
    t_glist       *x_glist;       /* containing glist */
    t_widgettype  *x_typedef;
    t_symbol      *x_type;        /* 1st creation arg: our type */
    t_symbol      *x_tkclass;     /* Tk widget class */
    t_symbol      *x_name;        /* 2nd creation arg: our name (common tag) */
    t_symbol      *x_cbtarget;    /* same, mangled (a target, and a tag) */
    t_symbol      *x_rptarget;    /* same, further mangled */
    t_symbol      *x_cvpathname;  /* see widget_getcvpathname() */
    t_symbol      *x_cvtarget;    /* for gui commands to be (re)sent to */
    t_symbol      *x_varname;     /* tcl variable holding our data */
    t_props       *x_options;     /* instance options */
    t_props       *x_handlers;    /* instance handlers */
    t_props       *x_arguments;   /* instance arguments */
    t_scriptlet   *x_iniscript;   /* instance initializer */
    t_scriptlet   *x_optscript;   /* option scriptlet */
    t_scriptlet   *x_auxscript;   /* auxiliary scriptlet */
    t_scriptlet   *x_transient;   /* output buffer */
    t_hammerfile  *x_filehandle;
    int            x_width;
    int            x_height;
    t_symbol      *x_background;
    int            x_hasstate;
    int            x_disabled;
    int            x_selected;
    int            x_update;      /* see widget_update() */
    int            x_vised;
    t_clock       *x_transclock;
    t_towentry    *x_towlist;
} t_widget;

typedef struct _tow
{
    t_object   x_ob;
    t_glist   *x_glist;      /* containing glist */
    t_symbol  *x_cvremote;   /* null if containing glist is our destination */
    t_symbol  *x_cvname;
    t_symbol  *x_type;       /* 2nd creation arg: widget's type */
    t_symbol  *x_name;       /* 3rd creation arg: widget's name */
    t_widgettype   *x_typedef;
    t_widgetentry  *x_widgetlist;
    struct _tow    *x_next;  /* next in the global towlist */
} t_tow;

static t_class *widget_class;
static t_class *tow_class;

/* Global towlist, searched in widget_attach().  There is no global widgetlist,
   because a destination glist is searched instead in tow_attach(). */
static t_tow *towlist = 0;

static t_symbol *widgetps_mouse;
static t_symbol *widgetps_motion;
static t_symbol *widgetps_atbang;
static t_symbol *widgetps_atfloat;
static t_symbol *widgetps_atsymbol;
static t_symbol *widgetps_atstore;
static t_symbol *widgetps_atrestore;

static char *widget_propsresolver(t_pd *owner, int ac, t_atom *av)
{
    t_widget *x = (t_widget *)owner;
    int len;
    scriptlet_reset(x->x_auxscript);
    if (scriptlet_add(x->x_auxscript, 1, 0, ac, av))
	return (scriptlet_getcontents(x->x_auxscript, &len));
    else
	return (0);
}

static t_canvas *widget_cvhook(t_pd *caller)
{
    return (glist_getcanvas(((t_widget *)caller)->x_glist));
}

/* LATER move to scriptlet.c, use the scriptlet interface (.^) */
static t_symbol *widget_getcvpathname(t_widget *x, t_glist *glist)
{
    t_canvas *cv;
    if (glist && glist != x->x_glist)
    {
	bug("widget_getcvpathname");
	x->x_glist = glist;
    }
    cv = glist_getcanvas(x->x_glist);
    if (cv == x->x_glist)
	return (x->x_cvpathname);  /* we are not in a gop */
    else
    {
	char buf[32];
	sprintf(buf, ".x%x.c", (int)cv);
	return (gensym(buf));
    }
}

/* LATER use the scriptlet interface (.-) */
static t_symbol *widget_getmypathname(t_widget *x, t_glist *glist)
{
    char buf[64];
    t_symbol *cvpathname = widget_getcvpathname(x, glist);
    sprintf(buf, "%s.%s%x", cvpathname->s_name, x->x_name->s_name, (int)x);
    return (gensym(buf));
}

static void widget_postatoms(char *msg, int ac, t_atom *av)
{
    startpost(msg);
    while (ac--)
    {
	if (av->a_type == A_FLOAT)
	    postfloat(av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
	    poststring(av->a_w.w_symbol->s_name);
	av++;
    }
    endpost();
}

/* If Tk widget creation fails, gui will send the '_failure' message
   to the Pd widget object, asking the receiving object to transform
   itself into a regular text object.  Due to the 'bindlist' corruption
   danger, this cannot be done directly from the '_failure' call, but
   has to be scheduled through a 'transclock', instead.  When the clock
   fires, the widget object creates, and glist_adds a 'makeshift' text
   object, then glist_deletes itself. */

/* this lock prevents glist_noselect() from reevaluating failure boxes */
static int widget_transforming = 0;

/* LATER also bind this to F4 or something */
static void widget_transtick(t_widget *x)
{
    t_text *newt, *oldt = (t_text *)x;
    t_binbuf *bb = binbuf_new();
    int nopt, nhnd, narg;
    t_atom *opt = props_getall(x->x_options, &nopt);
    t_atom *hnd = props_getall(x->x_handlers, &nhnd);
    t_atom *arg = props_getall(x->x_arguments, &narg);
    if (widget_transforming++)
	bug("widget_transtick");
    binbuf_addv(bb, "sss", gensym("widget"), x->x_type, x->x_name);
    if (narg) binbuf_add(bb, narg, arg);
    if (nopt) binbuf_add(bb, nopt, opt);
    if (nhnd) binbuf_add(bb, nhnd, hnd);
    canvas_setcurrent(x->x_glist);
    newt = (t_text *)pd_new(makeshift_class);
    newt->te_width = 0;
    newt->te_type = T_OBJECT;
    newt->te_binbuf = bb;
    newt->te_xpix = oldt->te_xpix;
    newt->te_ypix = oldt->te_ypix;
    glist_add(x->x_glist, &newt->te_g);
    if (glist_isvisible(x->x_glist))
    {
	glist_noselect(x->x_glist);
	glist_select(x->x_glist, &newt->te_g);
	gobj_activate(&newt->te_g, x->x_glist, 1);
	x->x_glist->gl_editor->e_textdirty = 1;  /* force evaluation */
    }
    canvas_unsetcurrent(x->x_glist);
    canvas_dirty(x->x_glist, 1);
    glist_delete(x->x_glist, (t_gobj *)x);
    widget_transforming--;
}

/* FIXME x_glist field validation against glist parameter (all handlers) */

static void widget_getrect(t_gobj *z, t_glist *glist,
			   int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_widget *x = (t_widget *)z;
    float x1, y1, x2, y2;
    x1 = text_xpix((t_text *)x, glist);
    y1 = text_ypix((t_text *)x, glist);
    x2 = x1 + x->x_width;
    y2 = y1 + x->x_height;
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void widget_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_widget *x = (t_widget *)z;
    t_text *t = (t_text *)z;
#if 0
    post("displace %d %d (%d %d -> %d %d)",
	 dx, dy, t->te_xpix, t->te_ypix, t->te_xpix + dx, t->te_ypix + dy);
#endif
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
	sys_vgui("%s move %s %d %d\n", widget_getcvpathname(x, glist)->s_name,
		 x->x_cbtarget->s_name, dx, dy);
    canvas_fixlinesfor(glist_getcanvas(glist), t);
}

/* LATER handle subitems */
static void widget_select(t_gobj *z, t_glist *glist, int flag)
{
    t_widget *x = (t_widget *)z;
    char *mypathname = widget_getmypathname(x, glist)->s_name;
    if (flag)
    {
	sys_vgui("%s config -bg blue %s\n", mypathname,
		 (x->x_hasstate ? "-state disabled" : ""));
	x->x_selected = 1;
    }
    else
    {
	if (x->x_disabled)
	    sys_vgui("%s config -bg %s\n", mypathname,
		     (x->x_background ? x->x_background->s_name : "gray"));
	else
	    sys_vgui("%s config -bg %s %s\n", mypathname,
		     (x->x_background ? x->x_background->s_name : "gray"),
		     (x->x_hasstate ? "-state normal" : ""));
	x->x_selected = 0;
    }
}

static void widget_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void widget_pushoptions(t_widget *x, int doit)
{
    char *mypathname = widget_getmypathname(x, x->x_glist)->s_name;
    if (scriptlet_evaluate(x->x_optscript, x->x_transient, 0,
			   0, 0, x->x_arguments))
    {
#ifdef WIDGET_DEBUG
	int sz;
	char *dp = scriptlet_getcontents(x->x_transient, &sz);
	post("vis: \"%s\"", dp);
#endif
	if (doit)
	{
	    sys_vgui("%s config ", mypathname);
	    scriptlet_push(x->x_transient);
	}
	else scriptlet_vpush(x->x_transient, "itemoptions");
    }
    else if (!scriptlet_isempty(x->x_optscript))
	bug("widget_pushoptions");
}

static void widget_pushinits(t_widget *x)
{
    if (masterwidget_evaluate(x->x_transient, 0, 0, 0, x->x_arguments))
	scriptlet_vpush(x->x_transient, "masterinits");
    else
	bug("widget_pushinits (master)");
    if (widgettype_isdefined(x->x_typedef))
    {
	int sz;
	if (widgettype_evaluate(x->x_typedef, x->x_transient, 0,
				0, 0, x->x_arguments))
	    scriptlet_vpush(x->x_transient, "typeinits");
	else if (*widgettype_getcontents(x->x_typedef, &sz) && sz > 0)
	    bug("widget_pushinits (type)");
    }
    if (scriptlet_evaluate(x->x_iniscript, x->x_transient, 0,
			   0, 0, x->x_arguments))
	scriptlet_vpush(x->x_transient, "iteminits");
    else if (!scriptlet_isempty(x->x_iniscript))
	bug("widget_pushinits (instance)");
}

static void widget_getconfig(t_widget *x)
{
    sys_vgui("::toxy::item_getconfig %s %s\n",
	     widget_getmypathname(x, x->x_glist)->s_name,
	     x->x_cbtarget->s_name);
}

static void widget_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_widget *x = (t_widget *)z;
    t_text *t = (t_text *)z;
    char *cvpathname = widget_getcvpathname(x, glist)->s_name;
    char *mypathname = widget_getmypathname(x, glist)->s_name;
    x->x_update = WIDGET_NOVIS;
    if (vis)
    {
	float px1 = text_xpix((t_text *)x, glist);
	float py1 = text_ypix((t_text *)x, glist);
#ifndef PD_MINOR_VERSION
	rtext_new(glist, t, glist->gl_editor->e_rtext, 0);
#endif
	widget_pushoptions(x, 0);
	widget_pushinits(x);
	sys_vgui("::toxy::item_vis %s %s %s %s %s %s %g %g\n",
		 x->x_tkclass->s_name, mypathname,
		 x->x_cbtarget->s_name, x->x_name->s_name,
		 x->x_varname->s_name, cvpathname, px1, py1);
	x->x_vised = 1;
    }
    else
    {
#ifndef PD_MINOR_VERSION
	t_rtext *rt = glist_findrtext(glist, t);
	if (rt) rtext_free(rt);
#endif
	x->x_vised = 0;
    }
}

static void widget_save(t_gobj *z, t_binbuf *bb)
{
    t_widget *x = (t_widget *)z;
    t_text *t = (t_text *)x;
    int nopt, nhnd, narg;
    t_atom *opt = props_getall(x->x_options, &nopt);
    t_atom *hnd = props_getall(x->x_handlers, &nhnd);
    t_atom *arg = props_getall(x->x_arguments, &narg);
    binbuf_addv(bb, "ssiisss", gensym("#X"), gensym("obj"),
		(int)t->te_xpix, (int)t->te_ypix, gensym("widget"),
		x->x_type, x->x_name);
    if (narg) binbuf_add(bb, narg, arg);
    if (nopt) binbuf_add(bb, nopt, opt);
    if (nhnd) binbuf_add(bb, nhnd, hnd);
    binbuf_addsemi(bb);
}

/* FIXME */
static void widget_properties(t_gobj *z, t_glist *glist)
{
    t_widget *x = (t_widget *)z;
    t_atom *ap;
    int ac, nleft;
    char *head = scriptlet_getcontents(x->x_optscript, &nleft);
    char buf[MAXPDSTRING + 1];
    buf[MAXPDSTRING] = 0;
    sprintf(buf, "%s %s", x->x_type->s_name, x->x_name->s_name);
    hammereditor_open(x->x_filehandle, buf);
    while (nleft > 0)
    {
	if (nleft > MAXPDSTRING)
	{
	    strncpy(buf, head, MAXPDSTRING);
	    head += MAXPDSTRING;
	    nleft -= MAXPDSTRING;
	}
	else
	{
	    strncpy(buf, head, nleft);
	    buf[nleft] = 0;
	    nleft = 0;
	}
	hammereditor_append(x->x_filehandle, buf);
    }
    scriptlet_reset(x->x_auxscript);
    ap = props_getall(x->x_handlers, &ac);
    if (ac) scriptlet_add(x->x_auxscript, 0, 0, ac, ap);
    head = scriptlet_getcontents(x->x_auxscript, &nleft);
    hammereditor_append(x->x_filehandle, "\n");
    while (nleft > 0)
    {
	if (nleft > MAXPDSTRING)
	{
	    strncpy(buf, head, MAXPDSTRING);
	    head += MAXPDSTRING;
	    nleft -= MAXPDSTRING;
	}
	else
	{
	    strncpy(buf, head, nleft);
	    buf[nleft] = 0;
	    nleft = 0;
	}
	hammereditor_append(x->x_filehandle, buf);
    }
}

static t_widgetbehavior widget_behavior =
{
    widget_getrect,
    widget_displace,
    widget_select,
    0,
    widget_delete,
    widget_vis,
    0,
    FORKY_WIDGETPADDING
};

static void widget_novis(t_widget *x)
{
    sys_vgui("::toxy::item_destroy %s %s\n",
	     widget_getmypathname(x, x->x_glist)->s_name, x->x_varname->s_name);
}

static void widget_update(t_widget *x, t_props *op)
{
    if (op == x->x_options)
    {
	t_atom *ap;
	int ac;
	scriptlet_reset(x->x_optscript);
	ap = props_getall(widgettype_getoptions(x->x_typedef), &ac);
	if (ac) scriptlet_add(x->x_optscript, 0, 0, ac, ap);
	ap = props_getall(x->x_options, &ac);
	if (ac) scriptlet_add(x->x_optscript, 0, 0, ac, ap);
	if (x->x_update &&
	    glist_isvisible(x->x_glist))  /* FIXME the condition */
	{
	    if (x->x_update == WIDGET_REVIS)
	    {
		widget_novis(x);
		widget_vis((t_gobj *)x, x->x_glist, 1);
	    }
	    else if (x->x_update == WIDGET_PUSHVIS)
	    {
		widget_pushoptions(x, 1);
		widget_getconfig(x);
	    }
	    x->x_update = WIDGET_NOVIS;
	}
    }
    else
    {
	/* LATER cache handlers */
    }
}

static t_symbol *widget_addprops(t_widget *x, t_props *op, int single,
				 t_symbol *s, int ac, t_atom *av)
{
    if (op)
    {
	t_symbol *empty;
	empty = props_add(op, single, s, ac, av);
	if (empty)
	    loud_error((t_pd *)x, "no value given for %s '%s'",
		       props_getname(op), empty->s_name);
	widget_update(x, op);
	return (empty);
    }
    else
    {
	bug("widget_addprops");
	return (0);
    }
}

static t_symbol *widget_addmessage(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    t_symbol *empty;
    if (!(empty = widget_addprops(x, x->x_arguments, 0, s, ac, av)) &&
	!(empty = widget_addprops(x, x->x_handlers, 0, s, ac, av)))
	empty = widget_addprops(x, x->x_options, 0, s, ac, av);
    return (empty);
}

static void widget_anything(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (s && s != &s_)
    {
	if (*s->s_name == '-' || *s->s_name == '@' || *s->s_name == '#')
	{
	    t_symbol *empty;
	    /* FIXME mixed messages */
	    if (*s->s_name == '-')
		x->x_update = WIDGET_PUSHVIS;
	    else if (*s->s_name == '#')
		x->x_update = WIDGET_REVIS;
	    else
		x->x_update = WIDGET_NOVIS;
	    if (empty = widget_addmessage(x, s, ac, av))
		loud_errand((t_pd *)x,
			    "(use 'remove %s' if that is what you want).",
			    empty->s_name);
	}
	else
	{
	    /* LATER cache this */
	    int hlen;
	    t_atom *hp;
	    t_symbol *sel;
	    char buf[MAXPDSTRING];
	    buf[0] = '@';
	    strcpy(buf + 1, s->s_name);
	    sel = gensym(buf);
	    if (((hp = props_getone(x->x_handlers, sel, &hlen)) ||
		 (hp = props_getone(widgettype_gethandlers(x->x_typedef),
				    sel, &hlen)))
		&& hlen > 1)
	    {
		scriptlet_reset(x->x_auxscript);
		scriptlet_add(x->x_auxscript, 0, 0, hlen - 1, hp + 1);
		if (scriptlet_evaluate(x->x_auxscript, x->x_transient, 1,
				       ac, av, x->x_arguments))
		    scriptlet_push(x->x_transient);
	    }
	    else loud_nomethod((t_pd *)x, s);
	}
    }
}

/* LATER cache this */
static void widget_bang(t_widget *x)
{
    int ac;
    t_atom *av;
    if ((av = props_getone(x->x_handlers, widgetps_atbang, &ac)) ||
	(av = props_getone(widgettype_gethandlers(x->x_typedef),
			   widgetps_atbang, &ac)))
    {
	if (ac > 1)
	{
	    scriptlet_reset(x->x_transient);
	    scriptlet_add(x->x_transient, 1, 1, ac - 1, av + 1);
	    scriptlet_push(x->x_transient);
	}
    }
}

/* LATER cache this */
static void widget_float(t_widget *x, t_float f)
{
    int ac;
    t_atom *av;
    if ((av = props_getone(x->x_handlers, widgetps_atfloat, &ac)) ||
	(av = props_getone(widgettype_gethandlers(x->x_typedef),
			   widgetps_atfloat, &ac)))
    {
	if (ac > 1)
	{
	    t_atom at;
	    SETFLOAT(&at, f);
	    scriptlet_reset(x->x_auxscript);
	    scriptlet_add(x->x_auxscript, 0, 0, ac - 1, av + 1);
	    if (scriptlet_evaluate(x->x_auxscript, x->x_transient, 1,
				   1, &at, x->x_arguments))
		scriptlet_push(x->x_transient);
	}
    }
}

/* LATER cache this */
static void widget_symbol(t_widget *x, t_symbol *s)
{
    int ac;
    t_atom *av;
    if ((av = props_getone(x->x_handlers, widgetps_atsymbol, &ac)) ||
	(av = props_getone(widgettype_gethandlers(x->x_typedef),
			   widgetps_atsymbol, &ac)))
    {
	if (ac > 1)
	{
	    t_atom at;
	    SETSYMBOL(&at, s);
	    scriptlet_reset(x->x_auxscript);
	    scriptlet_add(x->x_auxscript, 0, 0, ac - 1, av + 1);
	    if (scriptlet_evaluate(x->x_auxscript, x->x_transient, 1,
				   1, &at, x->x_arguments))
		scriptlet_push(x->x_transient);
	}
    }
}

static void widget_store(t_widget *x, t_symbol *s)
{
    if (s == &s_)
	s = x->x_varname;
    /* FIXME */
}

static void widget_restore(t_widget *x, t_symbol *s)
{
    if (s == &s_)
	s = x->x_varname;
    /* FIXME */
}

static void widget_set(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    t_symbol *prp;
    if (ac && av->a_type == A_SYMBOL && (prp = av->a_w.w_symbol))
    {
	t_symbol *empty = 0;
	ac--; av++;
	if (*prp->s_name == '-')
	{
	    x->x_update = WIDGET_PUSHVIS;
	    empty = widget_addprops(x, x->x_options, 1, prp, ac, av);
	}
	else if (*prp->s_name == '@')
	    empty = widget_addprops(x, x->x_handlers, 1, prp, ac, av);
	else if (*prp->s_name == '#')
	    empty = widget_addprops(x, x->x_arguments, 1, prp, ac, av);
	if (empty)
	    loud_errand((t_pd *)x,
			"(use 'remove %s' if that is what you want).",
			empty->s_name);
    }
    else loud_messarg((t_pd *)x, s);
}

static void widget_remove(t_widget *x, t_symbol *s)
{
    if (s)
    {
	t_props *op;
	if (*s->s_name == '-')
	    op = x->x_options;
	else if (*s->s_name == '@')
	    op = x->x_handlers;
	else if (*s->s_name == '#')
	    op = x->x_arguments;
	else
	    op = 0;
	if (op && props_remove(op, s))
	{
	    if (op == x->x_options) x->x_update = WIDGET_REVIS;
	    widget_update(x, op);
	}
	else loud_warning((t_pd *)x, 0, "%s %s has not been specified",
			  props_getname(op), s->s_name);
    }
}

static void widget_ini(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	scriptlet_reset(x->x_iniscript);
	scriptlet_add(x->x_iniscript, 0, 0, ac, av);
    }
}
static void widget_tot(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	scriptlet_reset(x->x_auxscript);
	scriptlet_add(x->x_auxscript, 1, 1, ac, av);
	if (scriptlet_evaluate(x->x_auxscript, x->x_transient, 1,
			       0, 0, x->x_arguments))
	    scriptlet_push(x->x_transient);
    }
}

static void widget_refresh(t_widget *x)
{
    x->x_update = WIDGET_REVIS;
    widget_update(x, x->x_options);
    widget_update(x, x->x_handlers);
    widget_update(x, x->x_arguments);
}

static void widget__failure(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
#if 0
    /* moved to the gui side, in order to alow special chars in error message */
    startpost("tcl error:");
    postatom(ac, av);
    endpost();
#endif
    loud_error((t_pd *)x, "creation failure");
    x->x_vised = 0;
    clock_delay(x->x_transclock, 0);
}

/* LATER handle subitems */
static void widget__config(t_widget *x, t_symbol *target, t_symbol *bg,
			   t_floatarg fw, t_floatarg fh, t_floatarg fst)
{
#ifdef WIDGET_DEBUG
    post("config %x %s \"%s\" %g %g",
	 (int)x, target->s_name, bg->s_name, fw, fh);
#endif
    x->x_width = (int)fw;
    x->x_height = (int)fh;
    if (bg != &s_) x->x_background = bg;
    x->x_hasstate = ((int)fst == 0);
    canvas_fixlinesfor(glist_getcanvas(x->x_glist), (t_text *)x);  /* FIXME */
}

static void widget__value(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
#ifdef WIDGET_DEBUG
    startpost("value:");
    postatom(ac, av);
    endpost();
#endif
    /* FIXME */
}

static void widget__callback(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac == 1)
    {
	if (av->a_type == A_FLOAT)
	    outlet_float(((t_object *)x)->ob_outlet, av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
	    outlet_symbol(((t_object *)x)->ob_outlet, av->a_w.w_symbol);
    }
    else if (ac)
    {
	if (av->a_type == A_FLOAT)
	    outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
	else if (av->a_type == A_SYMBOL)
	    outlet_anything(((t_object *)x)->ob_outlet,
			    av->a_w.w_symbol, ac - 1, av + 1);
    }
    else outlet_bang(((t_object *)x)->ob_outlet);
}

/* see also widget_select() */
/* LATER handle subitems */
static void widget__inout(t_widget *x, t_floatarg f)
{
    int disable = (int)f && x->x_glist->gl_edit;
    if (x->x_disabled)
    {
	if (!disable)
	{
	    if (!x->x_selected)
	    {
		char *mypathname = widget_getmypathname(x, x->x_glist)->s_name;
		if (x->x_hasstate)
		    sys_vgui("%s config -state normal\n", mypathname);
	    }
	    x->x_disabled = 0;
	}
    }
    else if (disable)
    {
	if (!x->x_selected)
	{
	    char *mypathname = widget_getmypathname(x, x->x_glist)->s_name;
	    if (x->x_hasstate)
		sys_vgui("%s config -state disabled\n", mypathname);
	}
	x->x_disabled = 1;
    }
}

static void widget__click(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac != 4)
    {
	loud_error((t_pd *)x, "bad arguments to the '%s' method", s->s_name);
	return;
    }
    if (x->x_glist->gl_havewindow)  /* LATER calculate on-parent coords */
    {
	if (x->x_cvtarget->s_thing)
	    /* LATER rethink */
	    typedmess(x->x_cvtarget->s_thing, widgetps_mouse, ac, av);
	else
	    typedmess((t_pd *)x->x_glist, widgetps_mouse, ac, av);
	widget__inout(x, 2.);
    }
}

/* LATER think how to grab the mouse when dragging */
static void widget__motion(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac != 3)
    {
	loud_error((t_pd *)x, "bad arguments to the '%s' method", s->s_name);
	return;
    }
    if (x->x_glist->gl_havewindow)  /* LATER calculate on-parent coords */
    {
#if 0
	post("motion %g %g", av[0].a_w.w_float, av[1].a_w.w_float);
#endif
	if (x->x_cvtarget->s_thing)
	    /* LATER rethink */
	    typedmess(x->x_cvtarget->s_thing, widgetps_motion, ac, av);
	else
	    typedmess((t_pd *)x->x_glist, widgetps_motion, ac, av);
    }
}

int widget_iswidget(t_gobj *g, t_symbol *type, t_symbol *name)
{
    if (*(t_pd *)g == widget_class)
    {
	t_widget *x = (t_widget *)g;
	return ((!type || type == x->x_type) &&
		(!name || name == x->x_name));
    }
    else return (0);
}

#ifdef WIDGET_DEBUG
static void widget_debug(t_widget *x)
{
    t_symbol *pn = widget_getcvpathname(x, 0);
    t_symbol *mn = widget_getmypathname(x, 0);
    int sz, i, nopt;
    t_atom *ap;
    char *bp, *key;
    post("containing glist: %x", x->x_glist);
    post("cv pathname%s %s", (pn ? ":" : ""), (pn ? pn->s_name : "unknown"));
    post("my pathname%s %s", (mn ? ":" : ""), (mn ? mn->s_name : "unknown"));
    if (ap = props_getall(widgettype_getoptions(x->x_typedef), &nopt))
	widget_postatoms("default options:", nopt, ap);
    if (ap = props_getall(x->x_options, &nopt))
	widget_postatoms("instance options:", nopt, ap);
    if (ap = props_getall(widgettype_gethandlers(x->x_typedef), &nopt))
	widget_postatoms("default handlers:", nopt, ap);
    if (ap = props_getall(x->x_handlers, &nopt))
	widget_postatoms("instance handlers:", nopt, ap);
    if (ap = props_getall(widgettype_getarguments(x->x_typedef), &nopt))
	widget_postatoms("default arguments:", nopt, ap);
    if (ap = props_getall(x->x_arguments, &nopt))
	widget_postatoms("instance arguments:", nopt, ap);
    post("dictionary:");
    bp = props_firstvalue(x->x_arguments, &key);
    while (bp)
    {
	post("\t%s: \"%s\"", key, bp);
	bp = props_nextvalue(x->x_arguments, &key);
    }
    bp = scriptlet_getcontents(x->x_transient, &sz);
    post("transient buffer (size %d):\n\"%s\"", sz, bp);
    bp = scriptlet_getcontents(x->x_optscript, &sz);
    post("option buffer (size %d):\n\"%s\"", sz, bp);
    bp = widgettype_getcontents(x->x_typedef, &sz);
    post("type initializer (size %d):\n\"%s\"", sz, bp);
    bp = scriptlet_getcontents(x->x_iniscript, &sz);
    post("instance initializer (size %d):\n\"%s\"", sz, bp);
    bp = masterwidget_getcontents(&sz);
    post("setup definitions (size %d):\n\"%s\"", sz, bp);
}
#endif

static void widget_attach(t_widget *x);
static void widget_detach(t_widget *x);

/* FIXME for all gui objects use a single sink (with a single clock) */
static void gui_unbind(t_pd *x, t_symbol *s)
{
    t_guiconnect *gc = guiconnect_new(0, s);
    guiconnect_notarget(gc, 1000.);
    pd_unbind(x, s);
}

static void widget_free(t_widget *x)
{
    widget_novis(x);
    gui_unbind((t_pd *)x, x->x_cbtarget);
    gui_unbind((t_pd *)x, x->x_rptarget);
    props_freeall(x->x_options);
    scriptlet_free(x->x_iniscript);
    scriptlet_free(x->x_optscript);
    scriptlet_free(x->x_auxscript);
    scriptlet_free(x->x_transient);
    hammerfile_free(x->x_filehandle);
    if (x->x_transclock) clock_free(x->x_transclock);
    widget_detach(x);
}

static void *widget_new(t_symbol *s, int ac, t_atom *av)
{
    t_widget *x;
    char buf[MAXPDSTRING];
    if (widget_transforming)
	return (0);
    masterwidget_initialize();
    x = (t_widget *)pd_new(widget_class);
    x->x_type = 0;
    x->x_name = 0;
    if (ac && av->a_type == A_SYMBOL)
    {
	x->x_type = av->a_w.w_symbol;
	ac--; av++;
    }
    if (ac && av->a_type == A_SYMBOL)
    {
	x->x_name = av->a_w.w_symbol;
	ac--; av++;
    }
    /* LATER think about anonymous widgets (single arg, or '.') */
    if (!x->x_type || x->x_type == &s_ ||
	!x->x_name || x->x_name == &s_)
    {
	loud_error((t_pd *)x, "bad arguments for a widget");
	loud_errand((t_pd *)x,
		    "expecting \"widget <type> <name> [properties]\"");
	return (0);
    }
    sprintf(buf, "%s%x", x->x_name->s_name, (int)x);
    pd_bind((t_pd *)x, x->x_cbtarget = gensym(buf));
    sprintf(buf, "%s%x.rp", x->x_name->s_name, (int)x);
    pd_bind((t_pd *)x, x->x_rptarget = gensym(buf));

    x->x_glist = canvas_getcurrent();
    x->x_typedef = widgettype_get(x->x_type);
    if (!(x->x_tkclass = widgettype_tkclass(x->x_typedef)))
	x->x_tkclass = x->x_type;

    sprintf(buf, ".x%x.c", (int)x->x_glist);
    x->x_cvpathname = gensym(buf);
    sprintf(buf, ".x%x", (int)x->x_glist);
    x->x_cvtarget = gensym(buf);
    sprintf(buf, "::toxy::v%x", (int)x);
    x->x_varname = gensym(buf);

    x->x_iniscript = scriptlet_new((t_pd *)x, x->x_rptarget, x->x_cbtarget,
				   x->x_name, x->x_glist, widget_cvhook);
    x->x_optscript = scriptlet_new((t_pd *)x, x->x_rptarget, x->x_cbtarget,
				   x->x_name, x->x_glist, widget_cvhook);
    x->x_auxscript = scriptlet_new((t_pd *)x, x->x_rptarget, x->x_cbtarget,
				   x->x_name, x->x_glist, widget_cvhook);
    x->x_transient = scriptlet_new((t_pd *)x, x->x_rptarget, x->x_cbtarget,
				   x->x_name, x->x_glist, widget_cvhook);

    x->x_options = props_new((t_pd *)x, "option", "-", 0, 0);
    x->x_handlers = props_new((t_pd *)x, "handler", "@", x->x_options, 0);
    x->x_arguments = props_new((t_pd *)x, "argument", "#", x->x_options,
			       widget_propsresolver);

    outlet_new((t_object *)x, &s_anything);
    /* LATER consider estimating these, based on widget class and options.
       The default used to be 50x50, which confused people wanting widgets
       in small gops, of size exactly as specified by the 'coords' message,
       but finding gops stretched, to accomodate the widget's default area. */
    x->x_width = 5;
    x->x_height = 5;
    props_clone(x->x_arguments, widgettype_getarguments(x->x_typedef));
    widget_addmessage(x, 0, ac, av);
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, 0, 0, 0);
    x->x_transclock = clock_new(x, (t_method)widget_transtick);
    x->x_background = 0;
    x->x_hasstate = 0;
    x->x_update = WIDGET_NOVIS;
    x->x_disabled = 0;
    x->x_vised = 0;
    widget_attach(x);
    return (x);
}

static t_glist *tow_getglist(t_tow *x, int complain)
{
    t_glist *glist =
	(x->x_cvremote ?
	 (t_glist *)pd_findbyclass(x->x_cvremote, canvas_class) : x->x_glist);
    if (!glist && complain)
	loud_error((t_pd *)x, "bad canvas name '%s'", x->x_cvname->s_name);
    return (glist);
}

static void tow_bang(t_tow *x)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	widget_bang(we->we_widget);
}

static void tow_float(t_tow *x, t_float f)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	widget_float(we->we_widget, f);
}

static void tow_symbol(t_tow *x, t_symbol *s)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	widget_symbol(we->we_widget, s);
}

static void tow_anything(t_tow *x, t_symbol *s, int ac, t_atom *av)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	typedmess((t_pd *)we->we_widget, s, ac, av);
}

static void tow__callback(t_tow *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac == 1)
    {
	if (av->a_type == A_FLOAT)
	    outlet_float(((t_object *)x)->ob_outlet, av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
	    outlet_symbol(((t_object *)x)->ob_outlet, av->a_w.w_symbol);
    }
    else if (ac)
    {
	if (av->a_type == A_FLOAT)
	    outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
	else if (av->a_type == A_SYMBOL)
	    outlet_anything(((t_object *)x)->ob_outlet,
			    av->a_w.w_symbol, ac - 1, av + 1);
    }
    else outlet_bang(((t_object *)x)->ob_outlet);
}

static void tow_widgetattach(t_tow *x, t_widget *w)
{
    t_towentry *te = getbytes(sizeof(*te));
    t_widgetentry *we = getbytes(sizeof(*we));
    te->te_tow = x;
    te->te_next = w->x_towlist;
    w->x_towlist = te;
    we->we_widget = w;
    we->we_next = x->x_widgetlist;
    x->x_widgetlist = we;
    pd_bind((t_pd *)x, w->x_cbtarget);
#ifdef TOW_DEBUG
    post("%s widget '%s' attached", w->x_type->s_name, w->x_cbtarget->s_name);
#endif
}

static void tow_widgetdetach(t_tow *x, t_widget *w)
{
    t_widgetentry *we1, *we2;
    for (we1 = 0, we2 = x->x_widgetlist; we2; we2 = we2->we_next)
    {
	if (we2->we_widget == w)
	{
#ifdef TOW_DEBUG
	    post("%s widget '%s' detached by widget's destructor",
		 w->x_type->s_name, w->x_cbtarget->s_name);
#endif
	    pd_unbind((t_pd *)x, w->x_cbtarget);
	    if (we1)
		we1->we_next = we2->we_next;
	    else
		x->x_widgetlist = we2->we_next;
	    freebytes(we2, sizeof(*we2));
	    return;
	}
	we1 = we2;
    }
    bug("tow_widgetdetach");
}

static void widget_attach(t_widget *x)
{
    t_tow *t;
    for (t = towlist; t; t = t->x_next)
	if (x->x_glist == tow_getglist(t, 0) &&
	    t->x_type == x->x_type && t->x_name == x->x_name)
	    tow_widgetattach(t, x);
}

static void widget_detach(t_widget *x)
{
    t_towentry *te;
    while (te = x->x_towlist)
    {
	x->x_towlist = te->te_next;
	tow_widgetdetach(te->te_tow, x);
	freebytes(te, sizeof(*te));
    }
}

static void tow_attach(t_tow *x)
{
    t_glist *glist = tow_getglist(x, 0);
    if (glist)
    {
	t_gobj *g;
	for (g = glist->gl_list; g; g = g->g_next)
	{
	    if (*(t_pd *)g == widget_class)
	    {
		t_widget *w = (t_widget *)g;
		if (w->x_type == x->x_type && w->x_name == x->x_name)
		    tow_widgetattach(x, w);
	    }
	}
#ifdef TOW_DEBUG
	if (!x->x_widgetlist)
	    post("%s widget '%s' not found",
		 x->x_type->s_name, x->x_name->s_name);
#endif
    }
#ifdef TOW_DEBUG
    else post("glist '%s' not found", x->x_cvname->s_name);
#endif
}

static void tow_detach(t_tow *x)
{
    t_widgetentry *we;
    while (we = x->x_widgetlist)
    {
	t_widget *w = we->we_widget;
	t_towentry *te1, *te2;
	x->x_widgetlist = we->we_next;
	pd_unbind((t_pd *)x, w->x_cbtarget);
	freebytes(we, sizeof(*we));
	for (te1 = 0, te2 = w->x_towlist; te2; te2 = te2->te_next)
	{
	    if (te2->te_tow == x)
	    {
#ifdef TOW_DEBUG
		post("%s widget '%s' detached by tow's destructor",
		     w->x_type->s_name, w->x_cbtarget->s_name);
#endif
		if (te1)
		    te1->te_next = te2->te_next;
		else
		    w->x_towlist = te2->te_next;
		freebytes(te2, sizeof(*te2));
		break;
	    }
	    te1 = te2;
	}
	if (!te2) bug("tow_detach");
    }
}

#ifdef TOW_DEBUG
static void tow_debug(t_tow *x)
{
    t_widgetentry *we;
    post("attached widgets:");
    for (we = x->x_widgetlist; we; we = we->we_next)
    {
	t_widget *w = we->we_widget;
	t_towentry *te;
	int other = 0, found = 0;
	startpost("\t%s %s", w->x_type->s_name, w->x_cbtarget->s_name);
	for (te = w->x_towlist; te; te = te->te_next)
	    if (te->te_tow == x)
		found++;
	    else
		other++;
	post(" (%d other tow%s)", other, (other == 1 ? "" : "s"));
	if (found != 1) post("BUG: listed %d times in widget's towlist", found);
    }
}
#endif

static void tow_free(t_tow *x)
{
    t_tow *t1, *t2;
#ifdef TOW_DEBUG
    startpost("updating towlist...");
#endif
    for (t1 = 0, t2 = towlist; t2; t2 = t2->x_next)
    {
	if (t2 == x)
	{
	    if (t1)
		t1->x_next = t2->x_next;
	    else
		towlist = t2->x_next;
#ifdef TOW_DEBUG
	    post("ok");
#endif
	    break;
	}
	t1 = t2;
    }
    tow_detach(x);
}

static void *tow_new(t_symbol *s1, t_symbol *s2, t_symbol *s3)
{
    t_tow *x = (t_tow *)pd_new(tow_class);
    char buf[64];
    x->x_glist = canvas_getcurrent();
    if (s1 && s1 != &s_ && strcmp(s1->s_name, "."))
	x->x_cvremote = canvas_makebindsym(x->x_cvname = s1);
    else
    {
	x->x_cvremote = 0;
	x->x_cvname = x->x_glist->gl_name;
    }
    x->x_type = s2;
    x->x_name = s3;
    outlet_new((t_object *)x, &s_anything);
    x->x_widgetlist = 0;
    x->x_next = towlist;
    towlist = x;
    tow_attach(x);
    return (x);
}

void widget_setup(void)
{
    post("beware! this is widget %s, %s %s build...",
	 TOXY_VERSION, loud_ordinal(TOXY_BUILD), TOXY_RELEASE);
    widgetps_mouse = gensym("mouse");
    widgetps_motion = gensym("motion");
    widgetps_atbang = gensym("@bang");
    widgetps_atfloat = gensym("@float");
    widgetps_atsymbol = gensym("@symbol");
    widgetps_atstore = gensym("@store");
    widgetps_atrestore = gensym("@restore");
    widgettype_setup();
    widget_class = class_new(gensym("widget"),
			     (t_newmethod)widget_new,
			     (t_method)widget_free,
			     sizeof(t_widget), 0, A_GIMME, 0);
    class_setwidget(widget_class, &widget_behavior);
    forky_setsavefn(widget_class, widget_save);
    forky_setpropertiesfn(widget_class, widget_properties);
    class_addbang(widget_class, widget_bang);
    class_addfloat(widget_class, widget_float);
    class_addsymbol(widget_class, widget_symbol);
    class_addanything(widget_class, widget_anything);
    class_addmethod(widget_class, (t_method)widget_store,
		    gensym("store"), A_DEFSYMBOL, 0);
    class_addmethod(widget_class, (t_method)widget_restore,
		    gensym("restore"), A_DEFSYMBOL, 0);
    class_addmethod(widget_class, (t_method)widget_set,
		    gensym("set"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget_remove,
		    gensym("remove"), A_SYMBOL, 0);
    class_addmethod(widget_class, (t_method)widget_ini,
		    gensym("ini"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget_tot,
		    gensym("tot"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget_refresh,
		    gensym("refresh"), 0);
    class_addmethod(widget_class, (t_method)widget__failure,
		    gensym("_failure"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget__config,
		    gensym("_config"),
		    A_SYMBOL, A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(widget_class, (t_method)widget__value,
		    gensym("_value"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget__callback,
		    gensym("_cb"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget__inout,
		    gensym("_inout"), A_FLOAT, 0);
    class_addmethod(widget_class, (t_method)widget__click,
		    gensym("_click"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget__motion,
		    gensym("_motion"), A_GIMME, 0);
#ifdef WIDGET_DEBUG
    class_addmethod(widget_class, (t_method)widget_debug,
		    gensym("debug"), 0);
#endif
    hammerfile_setup(widget_class, 0);

    makeshift_class = class_new(gensym("text"), 0, 0,
				sizeof(t_text),
				CLASS_NOINLET | CLASS_PATCHABLE, 0);

    tow_class = class_new(gensym("tow"),
			  (t_newmethod)tow_new,
			  (t_method)tow_free,
			  sizeof(t_tow), 0, A_SYMBOL, A_SYMBOL, A_SYMBOL, 0);
    class_addbang(tow_class, tow_bang);
    class_addfloat(tow_class, tow_float);
    class_addsymbol(tow_class, tow_symbol);
    class_addanything(tow_class, tow_anything);
    class_addmethod(tow_class, (t_method)tow__callback,
		    gensym("_cb"), A_GIMME, 0);
#ifdef TOW_DEBUG
    class_addmethod(tow_class, (t_method)tow_debug,
		    gensym("debug"), 0);
#endif
}
