/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "m_imp.h"  /* FIXME need access to c_externdir... */
#include "g_canvas.h"
#include "common/loud.h"
#include "build_counter"

#ifdef KRZYSZCZ
//#define PDDPLINK_DEBUG
#endif

enum { PDDPLINK_PD, PDDPLINK_HTML };  /* LATER add others */

typedef struct _pddplink
{
    t_object   x_ob;
    t_glist   *x_glist;
    int        x_isboxed;
    t_symbol  *x_dirsym;
    t_symbol  *x_ulink;
    t_atom     x_openargs[2];
    int        x_linktype;
    int        x_ishit;
} t_pddplink;

static t_class *pddplink_class;

/* FIXME need access to glob_pdobject... */
static t_pd *pddplink_pdtarget(t_pddplink *x)
{
    t_pd *pdtarget = gensym("pd")->s_thing;
    if (pdtarget && !strcmp(class_getname(*pdtarget), "pd"))
	return (pdtarget);
    else
	return ((t_pd *)x);  /* internal error */
}

static void pddplink_anything(t_pddplink *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_ishit)
    {
	startpost("pddplink: internal error (%s", (s ? s->s_name : ""));
	postatom(ac, av);
	post(")");
    }
}

static void pddplink_click(t_pddplink *x, t_floatarg xpos, t_floatarg ypos,
			   t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    x->x_ishit = 1;
    switch (x->x_linktype)
    {
    case PDDPLINK_PD:
	typedmess(pddplink_pdtarget(x), gensym("open"), 2, x->x_openargs);
	break;
    case PDDPLINK_HTML:
	sys_vgui("after 0 {::pddp::cliOpen %s}\n", x->x_ulink->s_name);
	break;
    }
    x->x_ishit = 0;
}

#ifdef PDDPLINK_DEBUG
static void pddplink_debug(t_pddplink *x)
{
}
#endif

static void pddplink_free(t_pddplink *x)
{
}

static void *pddplink_new(t_symbol *s1, t_symbol *s2)
{
    t_pddplink *x = (t_pddplink *)pd_new(pddplink_class);
    t_symbol *dirsym;
    x->x_glist = canvas_getcurrent();
    x->x_isboxed = (s2 == gensym("box"));
    x->x_dirsym = canvas_getdir(x->x_glist);  /* FIXME */
    if (!s1 || s1 == &s_)
    {
	x->x_linktype = PDDPLINK_HTML;
	x->x_ulink = gensym("index.html");
    }
    else
    {
	int len = strlen(s1->s_name);
	if (len > 3 && !strcmp(s1->s_name + len - 3, ".pd"))
	    x->x_linktype = PDDPLINK_PD;
	else
	    x->x_linktype = PDDPLINK_HTML;
	x->x_ulink = s1;
    }
    SETSYMBOL(&x->x_openargs[0], x->x_ulink);
    SETSYMBOL(&x->x_openargs[1], x->x_dirsym);
    x->x_ishit = 0;
    if (x->x_isboxed)
    {
	inlet_new((t_object *)x, (t_pd *)x, 0, 0);
	outlet_new((t_object *)x, &s_anything);
    }
    if (x->x_linktype == PDDPLINK_HTML)
	sys_vgui("after 0 {::pddp::srvUse %s}\n", x->x_dirsym->s_name);
    return (x);
}

void pddplink_setup(void)
{
    t_symbol *dirsym;
    post("this is pddplink %s, %s %s build...",
	 PDDP_VERSION, loud_ordinal(PDDP_BUILD), PDDP_RELEASE);
    pddplink_class = class_new(gensym("pddplink"),
			       (t_newmethod)pddplink_new,
			       (t_method)pddplink_free,
			       sizeof(t_pddplink),
			       CLASS_NOINLET | CLASS_PATCHABLE,
			       A_DEFSYM, A_DEFSYM, 0);
    class_addanything(pddplink_class, pddplink_anything);
    class_addmethod(pddplink_class, (t_method)pddplink_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
#ifdef PDDPLINK_DEBUG
    class_addmethod(pddplink_class, (t_method)pddplink_debug,
		    gensym("debug"), 0);
#endif
    dirsym = pddplink_class->c_externdir;  /* FIXME */
    sys_vgui("namespace eval ::pddp {variable theDir [pwd]}; cd %s\n",
	     dirsym->s_name);
    sys_gui("after 0 {source pddpboot.tcl}\n");
}
