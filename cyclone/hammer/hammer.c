/* Copyright (c) 2002-2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include "m_pd.h"
#include "unstable/fragile.h"
#include "common/loud.h"
#include "common/port.h"
#include "hammer/file.h"
#include "../build_counter"
void allhammers_setup(void);

typedef struct _hammer
{
    t_object       x_ob;
    t_symbol      *x_dir;
    t_hammerfile  *x_filehandle;
} t_hammer;

static t_class *hammer_class;
static int hammer_firstndx;
static int hammer_lastndx;

static void hammer_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    import_max(fn->s_name, "");
}

static void hammer_doimport(t_hammer *x, t_symbol *fn, t_symbol *dir)
{
    if (!dir || dir == &s_) dir = x->x_dir;
    if (fn && fn != &s_)
	import_max(fn->s_name, (dir && dir != &s_) ? dir->s_name : "");
    else
	hammerpanel_open(x->x_filehandle, dir);
}

static void hammer_click(t_hammer *x, t_floatarg xpos, t_floatarg ypos,
			 t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    hammer_doimport(x, 0, 0);
}

static void hammer_import(t_hammer *x, t_symbol *fn)
{
    hammer_doimport(x, fn, 0);
}

static void hammer_bang(t_hammer *x)
{
    fragile_class_printnames("hammer classes are: ",
			     hammer_firstndx, hammer_lastndx);
}

static void hammer_free(t_hammer *x)
{
    hammerfile_free(x->x_filehandle);
}

static void *hammer_new(t_symbol *s)
{
    t_hammer *x = (t_hammer *)pd_new(hammer_class);
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, hammer_readhook, 0, 0);
    x->x_dir = (s && s != &s_ ? s : canvas_getdir(x->x_filehandle->f_canvas));
    return (x);
}

void hammer_setup(void)
{
    if (canvas_getcurrent())
    {
	/* Loading the library by object creation is banned, because of a danger
	   of having some of the classes already loaded. LATER rethink. */
	loud_error(0, "apparently an attempt to create a 'hammer' object");
	loud_errand(0, "without having hammer library preloaded");
	return;
    }
    if (zgetfn(&pd_objectmaker, gensym("hammer")))
    {
	loud_error(0, "hammer is already loaded");
	return;
    }
    if (!zgetfn(&pd_objectmaker, gensym("cyclone")))
	post("this is hammer %s, %s %s build",
	     CYCLONE_VERSION, loud_ordinal(CYCLONE_BUILD), CYCLONE_RELEASE);
    hammer_class = class_new(gensym("hammer"),
			     (t_newmethod)hammer_new,
			     (t_method)hammer_free,
			     sizeof(t_hammer), 0, A_DEFSYM, 0);
    class_addbang(hammer_class, hammer_bang);
    class_addmethod(hammer_class, (t_method)hammer_import,
		    gensym("import"), A_DEFSYM, 0);
    class_addmethod(hammer_class, (t_method)hammer_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    hammerfile_setup(hammer_class, 0);
    hammer_firstndx = fragile_class_count();
    allhammers_setup();
    hammer_lastndx = fragile_class_count() - 1;
}
