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

static char masterwidget_builtin[] =
#include "default.wiq"
;

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

static t_canvas *widgettype_cvhook(t_pd *caller)
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
				     0, 0, widgettype_cvhook);
    dict_bind(mw->mw_typemap, (t_pd *)wt, wt->wt_typekey);
    return (wt);
}

static t_canvas *masterwidget_cvhook(t_pd *caller)
{
    return (0);
}

static t_scriptlet *masterwidget_cmnthook(t_pd *caller, char *rc,
					  char sel, char *buf)
{
    t_masterwidget *mw = masterwidget;
    if (!*buf)
	return (SCRIPTLET_UNLOCK);
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
	if (caller == (t_pd *)mw)
	{  /* default.wid or built-in defaults */
	    if (mw->mw_defaulttype)
	    {  /* no default type in default.wid, extracting built-in one */
		if (typeval != mw->mw_defaulttype)
		    return (SCRIPTLET_LOCK);
	    }
	    else
	    {
		if (typeval)
		{
		    /* LATER rethink */
		    loud_warning((t_pd *)mw, "redefinition of '%s'\
 in \"%s.wid\" file, ignored", buf, rc);
		    return (SCRIPTLET_LOCK);
		}
	    }
	}
	else
	{  /* <type>.wid */
	    if (caller != (t_pd *)typeval)
	    {
		loud_warning((t_pd *)mw, "alien definition of '%s'\
 in \"%s.wid\" file, ignored", buf, rc);
		return (SCRIPTLET_LOCK);
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
    return (SCRIPTLET_UNLOCK);
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
	t_scriptlet *mwsp =
	    scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
			  masterwidget->mw_target, 0, 0, 0);
	if (scriptlet_rcload(mwsp, (t_pd *)wt,
			     s->s_name, ".wid", 0, masterwidget_cmnthook)
	    == SCRIPTLET_OK)
	{
#ifdef WIDGETTYPE_VERBOSE
	    post("using %s's initializer", s->s_name);
#endif
	    if (!scriptlet_isempty(mwsp))
	    {
		t_scriptlet *sp =
		    scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
				  masterwidget->mw_target, 0, 0, 0);
		if (scriptlet_evaluate(mwsp, sp, 0, 0, 0, 0))
		{
		    scriptlet_push(sp);
		    scriptlet_append(masterwidget->mw_setupscript, mwsp);
		}
		else bug("widgettype_get");
		scriptlet_free(sp);
	    }
	}
	scriptlet_free(mwsp);
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

int masterwidget_evaluate(t_scriptlet *outsp, int visedonly,
			  int ac, t_atom *av, t_props *argprops)
{
    return (scriptlet_evaluate(masterwidget->mw_defaulttype->wt_iniscript,
			       outsp, visedonly, ac, av, argprops));
}

char *masterwidget_getcontents(int *szp)
{
    return (scriptlet_getcontents(masterwidget->mw_setupscript, szp));
}

void masterwidget_initialize(void)
{
    int rcresult;
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

    masterwidget->mw_setupscript =
	scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
		      masterwidget->mw_target, 0, 0, 0);
    masterwidget->mw_bb = binbuf_new();
    masterwidget->mw_parsedtype = 0;
    masterwidget->mw_defaulttype = 0;

    rcresult =
	scriptlet_rcload(masterwidget->mw_setupscript, 0, "default", ".wid",
			 masterwidget_builtin, masterwidget_cmnthook);
    if (rcresult == SCRIPTLET_OK)
    {
#ifdef WIDGETTYPE_VERBOSE
	post("using file 'default.wid'");
#endif
    }
    else
    {
	loud_warning((t_pd *)masterwidget,
		     "no file 'default.wid'... using built-in defaults");
    }
    typekey = dict_key(masterwidget->mw_typemap, "default");
    if ((typeval = (t_widgettype *)dict_value(masterwidget->mw_typemap, typekey))
	&& !scriptlet_isempty(masterwidget->mw_setupscript))
    {
	masterwidget->mw_defaulttype = typeval;
	rcresult = SCRIPTLET_OK;
    }
    else if (rcresult == SCRIPTLET_OK)
    {
	/* LATER think about adding only missing part to existing local defs */
	loud_warning((t_pd *)masterwidget, "%s missing in file 'default.wid'",
		     (typeval ? "setup definitions" : "master initializer"));
	masterwidget->mw_defaulttype =
	    widgettype_new(masterwidget, "default", 0, 0);
	scriptlet_reset(masterwidget->mw_setupscript);
	rcresult =
	    scriptlet_rcparse(masterwidget->mw_setupscript, 0, "default",
			      masterwidget_builtin, masterwidget_cmnthook);
    }
    else
    {
	bug("masterwidget_initialize 1");
	rcresult = SCRIPTLET_BADFILE;
    }
    if (rcresult == SCRIPTLET_OK)
    {
	t_scriptlet *sp =
	    scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
			  masterwidget->mw_target, 0, 0, 0);
	if (scriptlet_evaluate(masterwidget->mw_setupscript, sp, 0, 0, 0, 0))
	    scriptlet_push(sp);
	else
	    bug("masterwidget_initialize 2");
	scriptlet_free(sp);
    }
}
