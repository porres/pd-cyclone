/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/dict.h"
#include "common/props.h"
#include "toxy/scriptlet.h"
#include "widgettype.h"

#define WIDGETTYPE_VERBOSE
//#define WIDGETTYPE_DEBUG

struct _widgettype
{
    t_pd          wt_pd;
    t_symbol     *wt_typekey;    /* this is a typemap symbol */
    t_symbol     *wt_tkclass;    /* also 'undefined' flag (gensym symbol) */
    t_symbol     *wt_tkpackage;  /* gensym symbol */
    t_props      *wt_options;
    t_props      *wt_handlers;
    t_props      *wt_arguments;
    t_scriptlet  *wt_iniscript;
};

struct _masterwidget
{
    t_pd           mw_pd;
    t_symbol      *mw_target;
    t_scriptlet   *mw_setupscript;
    t_dict        *mw_typemap;
    t_widgettype  *mw_defaulttype;  /* contains master iniscript */
    t_widgettype  *mw_parsedtype;   /* the type currently parsed, if loading */
    t_binbuf      *mw_bb;           /* auxiliary, LATER remove */
};

static t_class *widgettype_class;
static t_class *masterwidget_class;

static t_masterwidget *masterwidget = 0;

static t_canvas *widgettype_cvhook(t_pd *z)
{
    return (0);
}

static void widgettype_map(t_widgettype *wt, char *cls, char *pkg)
{
    wt->wt_tkclass = (cls ? gensym(cls) : 0);
    wt->wt_tkpackage = (pkg ? gensym(pkg) : 0);
}

static t_widgettype *widgettype_new(t_masterwidget *mw,
				    char *typ, char *cls, char *pkg)
{
    t_widgettype *wt = (t_widgettype *)pd_new(widgettype_class);
    wt->wt_typekey = dict_key(mw->mw_typemap, typ);
    widgettype_map(wt, cls, pkg);
    wt->wt_options = props_new(0, "option", "-", 0, 0);
    wt->wt_handlers = props_new(0, "handler", "@", wt->wt_options, 0);
    wt->wt_arguments = props_new(0, "argument", "#", wt->wt_options, 0);
    wt->wt_iniscript = scriptlet_new((t_pd *)wt, mw->mw_target, mw->mw_target,
				     0, widgettype_cvhook);
    dict_bind(mw->mw_typemap, (t_pd *)wt, wt->wt_typekey);
    return (wt);
}

static t_canvas *masterwidget_cvhook(t_pd *z)
{
    return (0);
}

static t_scriptlet *masterwidget_cmnthook(t_pd *z, char *rc,
					  char sel, char *buf)
{
    t_masterwidget *mw = masterwidget;
    if (!*buf)
	return (0);
    if (sel == '>')
    {
	t_symbol *typekey;
	t_widgettype *typeval;
	char *cls = scriptlet_nextword(buf);
	char *pkg = (cls ? scriptlet_nextword(cls) : 0);
	mw->mw_parsedtype = 0;
	if (!cls)
	    cls = buf;
	typekey = dict_key(mw->mw_typemap, buf);
	typeval = (t_widgettype *)dict_value(mw->mw_typemap, typekey);
	if (z == (t_pd *)mw)
	{  /* default.wid */
	    if (typeval)
	    {
		/* LATER rethink */
		loud_warning((t_pd *)mw, "redefinition of '%s'\
 in \"%s.wid\" file, ignored", buf, rc);
		return (0);
	    }
	}
	else
	{  /* <type>.wid */
	    if (z != (t_pd *)typeval)
	    {
		loud_warning((t_pd *)mw, "alien definition of '%s'\
 in \"%s.wid\" file, ignored", buf, rc);
		return (0);
	    }
	}
	if (pkg)
	    /* carve out a single word as pkg, LATER rethink */
	    scriptlet_nextword(pkg);
	if (typeval)
	    widgettype_map(typeval, cls, pkg);
	else
	    typeval = widgettype_new(mw, buf, cls, pkg);
	mw->mw_parsedtype = typeval;
#ifdef WIDGETTYPE_DEBUG
	post("adding widget type '%s'", typeval->wt_typekey->s_name);
#endif
	return (typeval->wt_iniscript);
    }
    else if (sel == '.')
    {
	if (mw->mw_parsedtype
	    && (*buf == '-' || *buf == '@' || *buf == '#'))
	{
	    t_symbol *empty;
	    int ac;
	    /* LATER get rid of the binbuf thing */
	    binbuf_text(mw->mw_bb, buf, strlen(buf));
	    if (ac = binbuf_getnatom(mw->mw_bb))
	    {
		t_atom *av = binbuf_getvec(mw->mw_bb);
		t_props *pp;
		if (!(empty = props_add(pp = mw->mw_parsedtype->wt_options,
					0, 0, ac, av)) &&
		    !(empty = props_add(pp = mw->mw_parsedtype->wt_handlers,
					0, 0, ac, av)))
		    empty = props_add(pp = mw->mw_parsedtype->wt_arguments,
				      0, 0, ac, av);
		if (empty)
		    loud_warning((t_pd *)mw,
				 "no value given for %s '%s'\
 of a widget type '%s' in \"%s.wid\" file",
				 props_getname(pp), empty->s_name,
				 mw->mw_parsedtype->wt_typekey->s_name, rc);
	    }
	}
    }
    return (0);
}

t_widgettype *widgettype_get(t_symbol *s)
{
    t_widgettype *wt;
    /* default.wid defs are NOT overridden by <type>.wid --
       feature stability comes first, LATER rethink */
    if (wt = (t_widgettype *)dict_value(masterwidget->mw_typemap,
					dict_key(masterwidget->mw_typemap,
						 s->s_name)))
	masterwidget->mw_parsedtype = 0;
    else
    {
	/* first instance of a type not defined in default.wid */
	wt = widgettype_new(masterwidget, s->s_name, 0, 0);
	masterwidget->mw_parsedtype = wt;
    }
    if (masterwidget->mw_parsedtype)
    {
	if (scriptlet_rcload(wt->wt_iniscript, s->s_name, ".wid",
			     masterwidget_cmnthook) == SCRIPTLET_OK)
	{
#ifdef WIDGETTYPE_VERBOSE
	    post("using %s's initializer", s->s_name);
#endif
	}
    }
    return (wt);
}

int widgettype_isdefined(t_widgettype *wt)
{
    return (wt->wt_tkclass != 0);
}

t_symbol *widgettype_tkclass(t_widgettype *wt)
{
    return (wt->wt_tkclass);
}

t_props *widgettype_getoptions(t_widgettype *wt)
{
    return (wt->wt_options);
}

t_props *widgettype_gethandlers(t_widgettype *wt)
{
    return (wt->wt_handlers);
}

t_props *widgettype_getarguments(t_widgettype *wt)
{
    return (wt->wt_arguments);
}

char *widgettype_getcontents(t_widgettype *wt, int *szp)
{
    return (scriptlet_getcontents(wt->wt_iniscript, szp));
}

int widgettype_evaluate(t_widgettype *wt, t_scriptlet *outsp,
			int visedonly, int ac, t_atom *av, t_props *argprops)
{
    return (scriptlet_evaluate(wt->wt_iniscript, outsp,
			       visedonly, ac, av, argprops));
}

int masterwidget_evaluate(t_scriptlet *outsp, int visedonly,
			  int ac, t_atom *av, t_props *argprops)
{
    return (scriptlet_evaluate(masterwidget->mw_defaulttype->wt_iniscript,
			       outsp, visedonly, ac, av, argprops));
}

void widgettype_setup(void)
{
    static int done = 0;
    if (!done)
    {
	widgettype_class = class_new(gensym("widget type"), 0, 0,
				     sizeof(t_widgettype), CLASS_PD, 0);
	masterwidget_class = class_new(gensym("Widget"), 0, 0,
				       sizeof(t_masterwidget), CLASS_PD, 0);
	done = 1;
    }
}

void masterwidget_initialize(void)
{
    t_scriptlet *sp;
    t_symbol *typekey;
    t_widgettype *typeval;
    char buf[MAXPDSTRING];
    if (masterwidget)
	return;
    masterwidget = (t_masterwidget *)pd_new(masterwidget_class);
    sprintf(buf, "mw%x", (int)masterwidget);
    /* never unbound, LATER rethink */
    pd_bind((t_pd *)masterwidget, masterwidget->mw_target = gensym(buf));

    masterwidget->mw_typemap = dict_new(0);

    sp = masterwidget->mw_setupscript =
	scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
		      masterwidget->mw_target, 0, 0);
    masterwidget->mw_parsedtype = 0;
    masterwidget->mw_bb = binbuf_new();

    if (scriptlet_rcload(sp, "default", ".wid",
			 masterwidget_cmnthook) == SCRIPTLET_OK)
    {
#ifdef WIDGETTYPE_VERBOSE
	post("using file 'default.wid'");
#endif
    }
    else
    {
	loud_warning((t_pd *)masterwidget, "missing file 'default.wid'");

	/* no setup scriptlet, LATER use built-in default */
#if 0
	scriptlet_reset(sp);
	scriptlet_addstring(sp, ...
#endif
    }
    typekey = dict_key(masterwidget->mw_typemap, "default");
    if (typeval = (t_widgettype *)dict_value(masterwidget->mw_typemap, typekey))
	masterwidget->mw_defaulttype = typeval;
    else
    {
	/* no master initializer, LATER use built-in default */
	masterwidget->mw_defaulttype =
	    widgettype_new(masterwidget, "default", 0, 0);
	sp = masterwidget->mw_defaulttype->wt_iniscript;
#if 0
	scriptlet_reset(sp);
	scriptlet_addstring(sp, ...
#endif
    }
    sp = scriptlet_new((t_pd *)masterwidget,
		       masterwidget->mw_target, masterwidget->mw_target, 0, 0);
    if (scriptlet_evaluate(masterwidget->mw_setupscript, sp, 0, 0, 0, 0))
	scriptlet_push(sp);
    else
	bug("masterwidget_initialize");
    scriptlet_free(sp);
}
