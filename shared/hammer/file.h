/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __HAMMERFILE_H__
#define __HAMMERFILE_H__

typedef void (*t_hammerfilefn)(t_pd *, t_symbol *, int, t_atom *);
typedef void (*t_hammerembedfn)(t_pd *, t_binbuf *, t_symbol *);

typedef struct _hammerfile
{
    t_pd                 f_pd;
    t_pd                *f_master;
    t_canvas            *f_canvas;
    t_symbol            *f_bindname;
    t_symbol            *f_inidir;
    t_symbol            *f_inifile;
    t_hammerfilefn       f_panelfn;
    t_hammerfilefn       f_editorfn;
    t_hammerembedfn      f_embedfn;
    t_binbuf            *f_binbuf;
    t_clock             *f_panelclock;
    t_clock             *f_editorclock;
    struct _hammerfile  *f_savepanel;
    struct _hammerfile  *f_next;
} t_hammerfile;

void hammereditor_open(t_hammerfile *f, char *title);
void hammereditor_close(t_hammerfile *f, int ask);
void hammereditor_append(t_hammerfile *f, char *contents);
void hammerpanel_open(t_hammerfile *f, t_symbol *inidir);
void hammerpanel_save(t_hammerfile *f, t_symbol *inidir, t_symbol *inifile);
int hammerfile_ismapped(t_hammerfile *f);
int hammerfile_isloading(t_hammerfile *f);
int hammerfile_ispasting(t_hammerfile *f);
void hammerfile_free(t_hammerfile *f);
t_hammerfile *hammerfile_new(t_pd *master, t_hammerembedfn embedfn,
			     t_hammerfilefn readfn, t_hammerfilefn writefn,
			     t_hammerfilefn updatefn);
void hammerfile_setup(t_class *c, int embeddable);

#endif
