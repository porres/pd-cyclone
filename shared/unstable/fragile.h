/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __FRAGILE_H__
#define __FRAGILE_H__

int fragile_class_count(void);
void fragile_class_printnames(char *msg, int firstndx, int lastndx);
t_glist *fragile_garray_glist(void *arr);
t_outconnect *fragile_outlet_connections(t_outlet *o);
t_outconnect *fragile_outlet_nextconnection(t_outconnect *last,
					    t_object **destp, int *innop);
t_object *fragile_outlet_destination(t_outlet *op,
				     int ntypes, t_symbol **types,
				     t_pd *caller, char *errand);
t_sample *fragile_inlet_signalscalar(t_inlet *i);

#endif
