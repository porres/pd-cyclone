/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "m_pd.h"
#include "fitter.h"

#ifdef KRZYSZCZ
# include "loud.h"
# define FITTER_DEBUG
#else
# define loudbug_bug(msg)  fprintf(stderr, "BUG: %s\n", msg), bug(msg)
#endif

/* FIXME compatibility mode should be a standard Pd feature.  When it is,
   it will be possible to simplify the implementation.  Until then,
   we have to handle multiple copies of the 'fittermode_value' variable
   (coming from different externals), and the only way is multicasting
   through a symbol (#compatibility). */
static t_symbol *fittermode_value = 0;

typedef struct _fittermode_client
{
    t_class                    *fc_owner;
    t_symbol                  **fc_mirror;
    t_fittermode_callback       fc_callback;
    struct _fittermode_client  *fc_next;
} t_fittermode_client;

static t_fittermode_client *fittermode_clients = 0;
static t_class *fittermode_class = 0;
static t_pd *fittermode_target = 0;
static int fittermode_ready = 0;
static t_symbol *fitterps_hashcompatibility = 0;
static t_symbol *fitterps_max = 0;

/* read access (query), only from fittermode_dosetup() */
static void fittermode_bang(t_pd *x)
{
    if (fitterps_hashcompatibility)
    {
	if (fittermode_ready  /* do not reply to own request */
	    && fitterps_hashcompatibility->s_thing)
	    /* this proliferates for the third and subsequent
	       fittermode_dosetup() calls... */
	    pd_symbol(fitterps_hashcompatibility->s_thing,
		      fittermode_value);
    }
    else loudbug_bug("fittermode_bang");
}

/* read access (reply), only from fitter_dosetup() */
static void fittermode_symbol(t_pd *x, t_symbol *s)
{
    fittermode_value = s;
}

/* write access, only from fitter_setmode() */
static void fittermode_set(t_pd *x, t_symbol *s)
{
    t_fittermode_client *fc;
    fittermode_value = s;
    for (fc = fittermode_clients; fc; fc = fc->fc_next)
    {
	if (fc->fc_mirror)
	    *fc->fc_mirror = s;
	if (fc->fc_callback)
	    fc->fc_callback(s);
    }
}

static void fittermode_dosetup(int noquery)
{
    if (fittermode_class || fittermode_target)
	loudbug_bug("fittermode_dosetup");
    fitterps_hashcompatibility = gensym("#compatibility");
    fitterps_max = gensym("max");
    fittermode_class = class_new(fitterps_hashcompatibility,
					  0, 0, sizeof(t_pd),
					  CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(fittermode_class, fittermode_bang);
    class_addsymbol(fittermode_class, fittermode_symbol);
    class_addmethod(fittermode_class,
		    (t_method)fittermode_set,
		    gensym("set"), A_SYMBOL, 0);
    fittermode_target = pd_new(fittermode_class);
    pd_bind(fittermode_target, fitterps_hashcompatibility);
    if (!noquery)
	pd_bang(fitterps_hashcompatibility->s_thing);
    fittermode_ready = 1;
}

void fitter_setup(t_class *owner, t_symbol **mirror,
		  t_fittermode_callback callback)
{
    if (!fittermode_class)
	fittermode_dosetup(0);
    if (mirror || callback)
    {
	t_fittermode_client *fc = getbytes(sizeof(*fc));
	fc->fc_owner = owner;
	fc->fc_mirror = mirror;
	fc->fc_callback = callback;
	fc->fc_next = fittermode_clients;
	fittermode_clients = fc;
	if (mirror)
	    *mirror = fittermode_value;
    }
}

void fitter_drop(t_class *owner)
{
    if (fittermode_class && fitterps_hashcompatibility->s_thing)
    {
	t_fittermode_client *fcp = 0,
	    *fc = fittermode_clients;
	while (fc)
	{
	    if (fc->fc_owner == owner)
	    {
		if (fcp)
		    fcp->fc_next = fc->fc_next;
		else
		    fittermode_clients = fc->fc_next;
		break;
	    }
	    fcp = fc;
	    fc = fc->fc_next;
	}
	if (fc)
	    freebytes(fc, sizeof(*fc));
	else
	    loudbug_bug("fitter_drop 1");
    }
    else loudbug_bug("fitter_drop 2");
}

void fitter_setmode(t_symbol *s)
{
    post("setting compatibility mode to '%s'", (s ? s->s_name : "none"));
    if (!fittermode_class)
	fittermode_dosetup(1);
    if (fitterps_hashcompatibility->s_thing)
    {
	t_atom at;
	SETSYMBOL(&at, s);
	typedmess(fitterps_hashcompatibility->s_thing, gensym("set"), 1, &at);
    }
    else loudbug_bug("fitter_setmode");
}

t_symbol *fitter_getmode(void)
{
    if (!fittermode_class)
	fittermode_dosetup(0);
    return (fittermode_value);
}

void fittermax_set(void)
{
    if (!fittermode_class)
	fittermode_dosetup(0);
    fitter_setmode(fitterps_max);
}

int fittermax_get(void)
{
    if (!fittermode_class)
	fittermode_dosetup(0);
    return (fittermode_value == fitterps_max);
}

void fittermax_warning(t_class *c, char *fmt, ...)
{
    if (!fittermode_class)
	fittermode_dosetup(0);
    if (fittermode_value == fitterps_max)
    {
	char buf[MAXPDSTRING];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	post("'%s' class incompatibility warning:\n\t%s",
	     class_getname(c), buf);
	va_end(ap);
    }
}

void fittermax_rangewarning(t_class *c, int maxmax, char *what)
{
    fittermax_warning(c, "more than %d %s requested", maxmax, what);
}
