/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __FRAGILE_H__
#define __FRAGILE_H__

int fragile_class_count(void);
void fragile_class_printnames(char *msg, int firstndx, int lastndx);
t_glist *fragile_garray_glist(void *arr);
t_outconnect *fragile_outlet_connections(t_outlet *o);

#endif
