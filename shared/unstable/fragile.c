/* Copyright (c) 1997-2003 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Put here bits and pieces likely to break with any new Pd version. */

#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "unstable/pd_imp.h"
#include "unstable/fragile.h"

int fragile_class_count(void)
{
    return (pd_objectmaker->c_nmethod);
}

void fragile_class_printnames(char *msg, int firstndx, int lastndx)
{
    t_methodentry *mp = pd_objectmaker->c_methods;
    int ndx, len = strlen(msg);
    startpost(msg);
    for (ndx = firstndx, mp += ndx; ndx <= lastndx; ndx++, mp++)
    {
	t_symbol *s = mp->me_name;
	if (s && s->s_name[0] != '_')
	{
	    int l = 1 + strlen(s->s_name);
	    if ((len += l) > 66)
	    {
		endpost();
		startpost("   ");
		len = 3 + l;
	    }
	    poststring(s->s_name);
	}
    }
    endpost();
}

/* This structure is local to g_array.c.  We need it,
   because there is no other way to get into array's graph. */
struct _garray
{
    t_gobj x_gobj;
    t_glist *x_glist;
    /* ... */
};

t_glist *fragile_garray_glist(void *arr)
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
t_outconnect *fragile_outlet_connections(t_outlet *o)
{
    return (o ? o->o_connections : 0);
}

t_outconnect *fragile_outlet_nextconnection(t_outconnect *last,
					    t_object **destp, int *innop)
{
    t_inlet *dummy;
    return (obj_nexttraverseoutlet(last, destp, &dummy, innop));
}

/* silent, if caller is empty */
t_object *fragile_outlet_destination(t_outlet *op,
				     int ntypes, t_symbol **types,
				     t_pd *caller, char *errand)
{
    t_object *booty = 0;
    t_symbol *badtype = 0;
    int count = 0;
    t_outconnect *tobooty = fragile_outlet_connections(op);
    while (tobooty)
    {
	t_object *ob;
	int inno;
	count++;
	tobooty = fragile_outlet_nextconnection(tobooty, &ob, &inno);
	if (ob && inno == 0)
	{
	    /* LATER ask for class_getname()'s symbol version */
	    t_symbol **tp, *dsttype = gensym(class_getname(*(t_pd *)ob));
	    int i;
	    for (i = 0, tp = types; i < ntypes; i++, tp++)
	    {
		if (*tp == dsttype)
		{
		    booty = ob;
		    break;
		}
		else badtype = dsttype;
	    }
	}
    }
    if (booty)
    {
	if (count > 1 && caller)
	    loud_warning(caller, "multiple targets");
    }
    else if (caller)
    {
	if (badtype)
	    loud_error(caller, "bad target type '%s'", badtype->s_name);
	else
	    loud_error(caller, "no target");
	if (errand)
	    loud_errand(caller, errand);
    }
    return (booty);
}

/* These are local to m_obj.c. */
union inletunion
{
    t_symbol *iu_symto;
    t_gpointer *iu_pointerslot;
    t_float *iu_floatslot;
    t_symbol **iu_symslot;
    t_sample iu_floatsignalvalue;
};

struct _inlet
{
    t_pd i_pd;
    struct _inlet *i_next;
    t_object *i_owner;
    t_pd *i_dest;
    t_symbol *i_symfrom;
    union inletunion i_un;
};

/* simplified obj_findsignalscalar(), works for non-left inlets */
t_sample *fragile_inlet_signalscalar(t_inlet *i)
{
    return (&i->i_un.iu_floatsignalvalue);
}
