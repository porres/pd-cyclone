/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __WIDGETTYPE_H__
#define __WIDGETTYPE_H__

EXTERN_STRUCT _widgettype;
#define t_widgettype  struct _widgettype

EXTERN_STRUCT _masterwidget;
#define t_masterwidget  struct _masterwidget

t_widgettype *widgettype_get(t_symbol *s);
int widgettype_isdefined(t_widgettype *wt);
t_symbol *widgettype_tkclass(t_widgettype *wt);
t_props *widgettype_getoptions(t_widgettype *wt);
t_props *widgettype_gethandlers(t_widgettype *wt);
t_props *widgettype_getarguments(t_widgettype *wt);
char *widgettype_propname(t_symbol *s);
char *widgettype_getcontents(t_widgettype *wt, int *szp);
int widgettype_evaluate(t_widgettype *wt, t_scriptlet *outsp,
			int visedonly, int ac, t_atom *av, t_props *argprops);
int masterwidget_evaluate(t_scriptlet *outsp, int visedonly,
			  int ac, t_atom *av, t_props *argprops);
void masterwidget_initialize(void);

void widgettype_setup(void);

#endif
