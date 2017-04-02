// from old fragile (bits and pieces likely to break with any new Pd version.)

#include "m_pd.h"
#include "unstable/magic.h"
#include "unstable/pd_imp.h" // needed?
#include <string.h> // needed?

struct _outlet // local to m_obj.c.
{
    t_object *o_owner;
    struct _outlet *o_next;
    t_outconnect *o_connections;
    t_symbol *o_sym;
};
/* LATER export write access to o_connections field ('grab' class).
LATER encapsulate 'traverseoutlet' routines (not in the stable API yet). */


t_outconnect *magic_outlet_connections(t_outlet *o) // obj_starttraverseoutlet() replacement
    {
    return (o ? o->o_connections : 0);
    }

t_outconnect *magic_outlet_nextconnection(t_outconnect *last, t_object **destp, int *innop)
    {
    t_inlet *dummy;
    return (obj_nexttraverseoutlet(last, destp, &dummy, innop));
    }
