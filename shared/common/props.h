/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __PROPS_H__
#define __PROPS_H__

EXTERN_STRUCT _props;
#define t_props  struct _props

typedef char *(*t_propsresolver)(t_pd *, int, t_atom *);

t_symbol *props_add(t_props *pp, t_symbol *s, int ac, t_atom *av);
int props_remove(t_props *pp, t_symbol *s);
void props_clone(t_props *to, t_props *from);
char *props_getvalue(t_props *pp, char *key);
char *props_firstvalue(t_props *pp, char **keyp);
char *props_nextvalue(t_props *pp, char **keyp);
t_atom *props_getone(t_props *pp, t_symbol *s, int *npp);
t_atom *props_getall(t_props *pp, int *npp);
char *props_getname(t_props *pp);
void props_freeall(t_props *pp);
t_props *props_new(t_pd *owner, char *name, char *thisdelim,
		   t_props *otherprops, t_propsresolver resolver);

#endif
