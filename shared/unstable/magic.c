/* Copyright (c) 1997-2005 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Put here bits and pieces likely to break with any new Pd version. */

#include <string.h>
#include "m_pd.h"
#include "unstable/magic.h"
#include "unstable/pd_imp.h"


int magic_class_count(void)
{
    return (pd_objectmaker->c_nmethod);
}


/* This structure is local to g_array.c.  We need it,
   because there is no other way to get into array's graph. */
struct _garray
{
    t_gobj x_gobj;
    t_glist *x_glist;
    /* ... */
};

t_glist *magic_garray_glist(void *arr)
{
    return (((struct _garray *)arr)->x_glist);
}

/* This is local to m_obj.c.
   LATER export write access to o_connections field ('grab' class).
   LATER encapsulate 'traverseoutlet' routines (not in the stable API yet). */
struct _outlet
{
    t_object *o_owner;
    struct _outlet *o_next;
    t_outconnect *o_connections;
    t_symbol *o_sym;
};

/* obj_starttraverseoutlet() replacement */
t_outconnect *magic_outlet_connections(t_outlet *o)
{
    return (o ? o->o_connections : 0);
}

t_outconnect *magic_outlet_nextconnection(t_outconnect *last, t_object **destp, int *innop)
{
    t_inlet *dummy;
    return (obj_nexttraverseoutlet(last, destp, &dummy, innop));
}
