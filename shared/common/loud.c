/* Copyright (c) 2002-2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "m_pd.h"
#include "common/loud.h"

#define LOUD_ERROR_DEFAULT  "error (miXed): "

/* LATER move it somewhere else */
t_symbol *loud_floatsym(void)
{
    static t_symbol *s = 0;
    return (s ? s : (s = gensym("noninteger float")));
}

/* LATER move it somewhere else */
char *loud_symbolname(t_symbol *s, char *nullname)
{
    return (s && s != &s_ ? s->s_name : nullname);
}

/* LATER move it somewhere else */
int loud_matchignorecase(char *test, char *pattern)
{
    char ct, cp;
    for (ct = *test, cp = *pattern; ct && cp; ct = *++test, cp = *++pattern)
	if (ct != cp
	    && ((ct < 'A' || ct > 'z')
		|| ((ct > 'Z' || ct + 32 != cp)
		    && (ct < 'a' || ct - 32 != cp))))
	    return (0);
    return (ct == cp);
}

/* LATER move it somewhere else */
char *loud_ordinal(int n)
{
    static char buf[16];  /* assuming 10-digit INT_MAX */
    sprintf(buf, "%dth", n);
    if (n < 0) n = -n;
    n %= 100;
    if (n > 20) n %= 10;
    if (n && n <= 3)
    {
	char *ptr = buf + strlen(buf) - 2;
	switch (n)
	{
	case 1: strcpy(ptr, "st"); break;
	case 2: strcpy(ptr, "nd"); break;
	case 3: strcpy(ptr, "rd"); break;
	}
    }
    return (buf);
}

void loud_error(t_pd *x, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (x)
    {
	char buf[MAXPDSTRING];
	fprintf(stderr, "%s's ", class_getname(*x));
	vsprintf(buf, fmt, ap);
	pd_error(x, buf);
    }
    else
    {
	fputs(LOUD_ERROR_DEFAULT, stderr);
	vfprintf(stderr, fmt, ap);
	putc('\n', stderr);
    }
    va_end(ap);
}

void loud_errand(t_pd *x, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "%*s", (int)(x ? strlen(class_getname(*x)) + 10
				 : strlen(LOUD_ERROR_DEFAULT)), "");
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
}

void loud_syserror(t_pd *x, char *msg)
{
    if (msg)
	loud_error(x, "%s (%s)", msg, strerror(errno));
    else
	loud_error(x, strerror(errno));
}

void loud_nomethod(t_pd *x, t_symbol *s)
{
    loud_error(x, "doesn't understand \"%s\"", s->s_name);
}

void loud_messarg(t_pd *x, t_symbol *s)
{
    loud_error(x, "bad arguments for message \"%s\"", s->s_name);
}

int loud_checkint(t_pd *x, t_float f, int *valuep, t_symbol *mess)
{
    if ((*valuep = (int)f) == f)
	return (1);
    else
    {
	if (mess == &s_float)
	    loud_nomethod(x, loud_floatsym());
	else if (mess)
	    loud_error(x, "\"%s\" argument invalid for message \"%s\"",
		       loud_floatsym()->s_name, mess->s_name);
	return (0);
    }
}

void loud_classarg(t_class *c)
{
    loud_error(0, "missing or bad arguments in \"%s\"", class_getname(c));
}

void loud_warning(t_pd *x, char *who, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "warning (%s): ",
	    (x ? class_getname(*x) : (who ? who : "miXed")));
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);
}

void loud_notimplemented(t_pd *x, char *name)
{
    if (name)
	loud_warning(x, 0, "\"%s\" method not implemented (yet)", name);
    else
	loud_warning(x, 0, "not implemented (yet)");
}

void loud_incompatible(t_class *c, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "'%s' class incompatibility warning:\n\t",
	    class_getname(c));
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);
}

void loud_incompatible_max(t_class *c, int maxmax, char *what)
{
    loud_incompatible(c, "more than %d %s requested", maxmax, what);
}

int loud_floatarg(t_class *c, int which, int ac, t_atom *av,
		  t_float *vp, t_float minval, t_float maxval,
		  int underaction, int overaction, char *what)
{
    int result = LOUD_ARGOK;
    if (which < ac)
    {
	av += which;
	if (av->a_type == A_FLOAT)
	{
	    t_float f = av->a_w.w_float;
	    if (f < minval)
	    {
		*vp = (underaction & LOUD_CLIP ? minval : f);
		if (underaction)
		    result = LOUD_ARGUNDER;
	    }
	    else if (f > maxval)
	    {
		*vp = (overaction & LOUD_CLIP ? maxval : f);
		if (overaction)
		    result = LOUD_ARGOVER;
	    }
	    else *vp = f;
	}
	else result = LOUD_ARGTYPE;
    }
    else result = LOUD_ARGMISSING;
    if (what)
    {
	switch (result)
	{
	case LOUD_ARGUNDER:
	    if (underaction & LOUD_WARN)
	    {
		if (underaction & LOUD_CLIP)
		    loud_warning(&c, 0, "%s rounded up to %g", what, minval);
		else
		    loud_incompatible(c, "less than %g %s requested",
				      minval, what);
	    }
	    break;
	case LOUD_ARGOVER:
	    if (overaction & LOUD_WARN)
	    {
		if (overaction & LOUD_CLIP)
		    loud_warning(&c, 0, "%s truncated to %g", what, maxval);
		else
		    loud_incompatible(c, "more than %g %s requested",
				      maxval, what);
	    }
	    break;
	case LOUD_ARGTYPE:
	    loud_error(0, "bad argument %d (%s)", which, class_getname(c));
	    break;
	default:;
	}
    }
    return (result);
}
