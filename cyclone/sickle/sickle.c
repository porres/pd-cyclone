/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "unstable/fragile.h"
#include "common/loud.h"
#include "common/port.h"
#include "hammer/file.h"
#include "../build_counter"
void allsickles_setup(void);

typedef struct _sickle
{
    t_object       x_ob;
    t_symbol      *x_dir;
    t_symbol      *x_canvasdir;
    t_hammerfile  *x_filehandle;
} t_sickle;

static t_class *sickle_class;
static int sickle_firstndx;
static int sickle_lastndx;

static void sickle_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    import_max(fn->s_name, "");
}

static void sickle_doimport(t_sickle *x, t_symbol *fn, t_symbol *dir)
{
    if (!dir || dir == &s_)
	dir = x->x_dir;
    if (fn && fn != &s_)
	import_max(fn->s_name, (dir && dir != &s_) ? dir->s_name : "");
    else
	hammerpanel_open(x->x_filehandle, dir);
}

static void sickle_click(t_sickle *x, t_floatarg xpos, t_floatarg ypos,
			 t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    sickle_doimport(x, 0, 0);
}

static void sickle_import(t_sickle *x, t_symbol *fn)
{
    sickle_doimport(x, fn, 0);
}

static void sickle_cd(t_sickle *x, t_symbol *dir)
{
    /* LATER hammerfile interface for relative jumps, etc. */
    x->x_dir = (dir && dir != &s_ ? dir : x->x_canvasdir);
}

static void sickle_pwd(t_sickle *x)
{
    outlet_symbol(((t_object *)x)->ob_outlet, x->x_dir);
}

static void sickle_bang(t_sickle *x)
{
    fragile_class_printnames("sickle classes are: ",
			     sickle_firstndx, sickle_lastndx);
}

static void sickle_free(t_sickle *x)
{
    hammerfile_free(x->x_filehandle);
}

static void *sickle_new(void)
{
    t_sickle *x = (t_sickle *)pd_new(sickle_class);
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, sickle_readhook, 0, 0);
    x->x_canvasdir = canvas_getdir(x->x_filehandle->f_canvas);
    x->x_dir = x->x_canvasdir;
    outlet_new((t_object *)x, &s_symbol);
    return (x);
}

void sickle_setup(void)
{
    if (canvas_getcurrent())
    {
	/* Loading the library by object creation is banned, because of a danger
	   of having some of the classes already loaded. LATER rethink. */
	loud_error(0, "apparently an attempt to create a 'sickle' object");
	loud_errand(0, "without having sickle library preloaded");
	return;
    }
    if (zgetfn(&pd_objectmaker, gensym("sickle")))
    {
	loud_error(0, "sickle is already loaded");
	return;
    }
    if (!zgetfn(&pd_objectmaker, gensym("cyclone")))
	post("this is sickle %s, %s %s build",
	     CYCLONE_VERSION, loud_ordinal(CYCLONE_BUILD), CYCLONE_RELEASE);
    sickle_class = class_new(gensym("sickle"),
			     (t_newmethod)sickle_new,
			     (t_method)sickle_free,
			     sizeof(t_sickle), 0, 0);
    class_addbang(sickle_class, sickle_bang);
    class_addmethod(sickle_class, (t_method)sickle_cd,
		    gensym("cd"), A_DEFSYM, 0);
    class_addmethod(sickle_class, (t_method)sickle_pwd,
		    gensym("pwd"), 0);
    class_addmethod(sickle_class, (t_method)sickle_import,
		    gensym("import"), A_DEFSYM, 0);
    class_addmethod(sickle_class, (t_method)sickle_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    hammerfile_setup(sickle_class, 0);
    sickle_firstndx = fragile_class_count();
    allsickles_setup();
    sickle_lastndx = fragile_class_count() - 1;
}
