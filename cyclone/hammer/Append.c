/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"

#define APPEND_INISIZE     32  /* LATER rethink */
#define APPEND_MAXSIZE    256

typedef struct _append
{
    t_object  x_ob;
    int       x_size;    /* as allocated */
    int       x_natoms;  /* as used */
    t_atom   *x_message;
    t_atom   *x_messbuf;
    t_atom    x_messini[APPEND_INISIZE];
    int       x_entered;
    int       x_auxsize;
    t_atom   *x_auxbuf;
} t_append;

static t_class *append_class;

/* Usually a preallocation method is used, except in special cases of:
   1) reentrant output request, or 2) an output request which would cause
   resizing to more than MAXSIZE (no such limit for a 'set' message).
   In both special cases, a temporary output buffer is allocated.
   A separately preallocated output buffer is not used, thus avoiding
   memcpying of the stored message (a small performance gain when the
   preallocation method is chosen).  Instead, self-invoked 'set'
   messages are postponed, using an auxiliary buffer.
*/

/* Any Append's output goes through outlet_anything() -> typedmess() */

static void append_setnatoms(t_append *x, int natoms)
{
    x->x_message = x->x_messbuf + x->x_size - natoms;
    x->x_natoms = natoms;
}

static void append_anything(t_append *x, t_symbol *s, int ac, t_atom *av)
{
    int reentered = x->x_entered;
    int prealloc = !reentered;
    int ntotal = x->x_natoms + ac;
    t_atom *buf;
    x->x_entered = 1;
    if (prealloc && ntotal > x->x_size)
    {
	if (ntotal > APPEND_MAXSIZE)
	    prealloc = 0;
	else
	{
	    int nrequested = ntotal;
	    x->x_messbuf = grow_withtail(&nrequested, &x->x_natoms,
					 (char **)&x->x_message,
					 &x->x_size, x->x_messbuf,
					 APPEND_INISIZE, x->x_messini,
					 sizeof(*x->x_message));
	    prealloc = (nrequested == ntotal);
	}
    }
    if (prealloc)
    {
	buf = x->x_message - ac;
	if (ac)
	    memcpy(buf, av, ac * sizeof(*buf));
	outlet_anything(((t_object *)x)->ob_outlet, s, ntotal, buf);
    }
    else
    {
	/* LATER consider using the stack if ntotal <= MAXSTACK */
	if (buf = getbytes(ntotal * sizeof(*buf)))
	{
	    if (ac)
		memcpy(buf, av, ac * sizeof(*buf));
	    if (x->x_natoms)
		memcpy(buf + ac, x->x_message, x->x_natoms * sizeof(*buf));
	    outlet_anything(((t_object *)x)->ob_outlet, s, ntotal, buf);
	    freebytes(buf, ntotal * sizeof(*buf));
	}
    }
    if (!reentered)
    {
	x->x_entered = 0;
	if (x->x_auxbuf)
	{
	    if (x->x_auxsize <= x->x_size)
	    {
		append_setnatoms(x, x->x_auxsize / 2);
		memcpy(x->x_message, x->x_auxbuf + x->x_natoms,
		       x->x_natoms * sizeof(*x->x_message));
		freebytes(x->x_auxbuf, x->x_auxsize * sizeof(*x->x_auxbuf));
	    }
	    else
	    {
		if (x->x_messbuf != x->x_messini)
		    freebytes(x->x_messbuf, x->x_size * sizeof(*x->x_messbuf));
		x->x_size = x->x_auxsize;
		x->x_messbuf = x->x_auxbuf;
		append_setnatoms(x, x->x_auxsize / 2);
	    }
	    x->x_auxbuf = 0;
	}
    }
}

static void append_bang(t_append *x)
{
    /* CHECKED: a nop */
}

static void append_float(t_append *x, t_float f)
{
    t_atom at;
    SETFLOAT(&at, f);
    append_anything(x, &s_list, 1, &at);  /* CHECKED: converted to list */
}

/* CHECKED: incompatible -- LATER consider converting to anything */
static void append_symbol(t_append *x, t_symbol *s)
{
    t_atom at;
    SETSYMBOL(&at, s);
    append_anything(x, &s_symbol, 1, &at);
}

/* LATER gpointer */

static void append_set(t_append *x, t_symbol *s, int ac, t_atom *av)
{
    int newsize = ac * 2;
    if (newsize > 0)
    {
	if (x->x_entered)
	{
	    if (x->x_auxbuf)
	    {
		loud_warning((t_pd *)x, "\'set\' message overridden");
		freebytes(x->x_auxbuf, x->x_auxsize * sizeof(*x->x_auxbuf));
		x->x_auxsize = 0;
	    }
	    if (x->x_auxbuf = getbytes(newsize * sizeof(*x->x_auxbuf)))
	    {
		memcpy(x->x_auxbuf + ac, av, ac * sizeof(*x->x_auxbuf));
		x->x_auxsize = newsize;
	    }
	}
	else
	{
	    t_atom *ap;
	    if (newsize > x->x_size)
	    {
		int sz = newsize;
		x->x_messbuf = grow_nodata(&sz, &x->x_size, x->x_messbuf,
					   APPEND_INISIZE, x->x_messini,
					   sizeof(*x->x_messbuf));
		if (sz != newsize)
		    ac = sz / 2;  /* LATER rethink */
	    }
	    append_setnatoms(x, ac);
	    ap = x->x_message;
	    while (ac--) *ap++ = *av++;
	}
    }
}

static void append_free(t_append *x)
{
    if (x->x_messbuf != x->x_messini)
	freebytes(x->x_messbuf, x->x_size * sizeof(*x->x_messbuf));
}

static void *append_new(t_symbol *s, int ac, t_atom *av)
{
    t_append *x = (t_append *)pd_new(append_class);
    x->x_size = APPEND_INISIZE;
    x->x_natoms = 0;
    x->x_messbuf = x->x_messini;
    x->x_auxbuf = 0;
    x->x_entered = 0;
    outlet_new((t_object *)x, &s_anything);
    append_setnatoms(x, 0);
    append_set(x, 0, ac, av);
    return (x);
}

void Append_setup(void)
{
    append_class = class_new(gensym("Append"),
			     (t_newmethod)append_new,
			     (t_method)append_free,
			     sizeof(t_append), 0,
			     A_GIMME, 0);
    class_addbang(append_class, append_bang);
    class_addfloat(append_class, append_float);
    class_addsymbol(append_class, append_symbol);
    class_addlist(append_class, append_anything);  /* LATER rethink */
    class_addanything(append_class, append_anything);
    class_addmethod(append_class, (t_method)append_set,
		    gensym("set"), A_GIMME, 0);
}
