/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __FORKY_H__
#define __FORKY_H__

int forky_hasfeeders(t_object *x, t_glist *glist, int inno, t_symbol *outsym);
t_int forky_getbitmask(int ac, t_atom *av);

#endif
