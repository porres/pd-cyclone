/* Copyright (c) 2003-2004 krzYszcz and others.
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
char *widgettype_getinitializer(t_widgettype *wt, int *szp);
char *widgettype_getconstructor(t_widgettype *wt, int *szp);
char *widgettype_getdestructor(t_widgettype *wt, int *szp);
int widgettype_ievaluate(t_widgettype *wt, t_scriptlet *outsp,
			 int visedonly, int ac, t_atom *av, t_props *argprops);
int widgettype_cevaluate(t_widgettype *wt, t_scriptlet *outsp,
			 int visedonly, int ac, t_atom *av, t_props *argprops);
int widgettype_devaluate(t_widgettype *wt, t_scriptlet *outsp,
			 int visedonly, int ac, t_atom *av, t_props *argprops);
void widgettype_setup(void);

char *masterwidget_getinitializer(int *szp);
char *masterwidget_getcontents(int *szp);
int masterwidget_ievaluate(t_scriptlet *outsp, int visedonly,
			   int ac, t_atom *av, t_props *argprops);
void masterwidget_validate(void);

#endif
