/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __LOUD_H__
#define __LOUD_H__

#define LOUD_CLIP  1
#define LOUD_WARN  2

enum { LOUD_ARGOK, LOUD_ARGUNDER, LOUD_ARGOVER, LOUD_ARGTYPE, LOUD_ARGMISSING };

t_symbol *loud_floatsym(void);
char *loud_symbolname(t_symbol *s, char *nullname);
char *loud_ordinal(int n);
void loud_error(t_pd *x, char *fmt, ...);
void loud_errand(t_pd *x, char *fmt, ...);
void loud_syserror(t_pd *x, char *msg);
void loud_nomethod(t_pd *x, t_symbol *s);
void loud_messarg(t_pd *x, t_symbol *s);
int loud_checkint(t_pd *x, t_float f, int *valuep, t_symbol *mess);
void loud_classarg(t_class *c);
void loud_warning(t_pd *x, char *fmt, ...);
void loud_notimplemented(t_pd *x, char *name);
void loud_incompatible(t_class *c, char *fmt, ...);
void loud_incompatible_max(t_class *c, int maxmax, char *what);
int loud_floatarg(t_class *c, int which, int ac, t_atom *av,
		  t_float *vp, t_float minval, t_float maxval,
		  int underaction, int overaction, char *what);

#endif
