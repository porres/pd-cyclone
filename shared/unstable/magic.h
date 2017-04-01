/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __MAGIC_H__
#define __MAGIC_H__

t_symbol *magic_class_getexterndir(t_class *c);
int magic_class_count(void);
int magic_class_getnames(t_atom *av, int maxnames);
void magic_class_raise(t_symbol *cname, t_newmethod thiscall);
t_pd *magic_class_mutate(t_symbol *cname, t_newmethod thiscall,
			   int ac, t_atom *av);
t_newmethod magic_class_getalien(t_symbol *cname, t_newmethod thiscall,
				   t_atomtype **argtypesp);
t_pd *magic_class_createobject(t_symbol *cname, t_newmethod callthis,
				 t_atomtype *argtypes, int ac, t_atom *av);
void magic_class_printnames(char *msg, int firstndx, int lastndx);
t_glist *magic_garray_glist(void *arr);
t_outconnect *magic_outlet_connections(t_outlet *o);
t_outconnect *magic_outlet_nextconnection(t_outconnect *last,
					    t_object **destp, int *innop);
t_object *magic_outlet_destination(t_outlet *op,
				     int ntypes, t_symbol **types,
				     t_pd *caller, char *errand);
t_sample *magic_inlet_signalscalar(t_inlet *i);

#endif
