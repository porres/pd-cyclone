/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Never use forked calls in shadow code... */

/* LATER support multi-atom dir (creation args), and fn ('import' message)
   (same in hammer and sickle) */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/port.h"
#include "hammer/file.h"
#include "unstable/fragile.h"
#include "unstable/loader.h"
#include "shadow.h"
#include "../build_counter"

typedef struct _cyclone
{
    t_object       x_ob;
    t_symbol      *x_dir;
    t_hammerfile  *x_filehandle;
} t_cyclone;

static t_class *cyclone_class;
static int cyclone_hammerndx;
static int cyclone_sicklendx;
static int cyclone_nettlesndx;
static int cyclone_dummiesndx;
static int cyclone_lastndx;

static t_pd *cyclone_dproxy = 0;

static void cyclone_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    import_max(fn->s_name, "");
}

static void cyclone_doimport(t_cyclone *x, t_symbol *fn, t_symbol *dir)
{
    if (!dir || dir == &s_) dir = x->x_dir;
    if (fn && fn != &s_)
	import_max(fn->s_name, (dir && dir != &s_) ? dir->s_name : "");
    else
	hammerpanel_open(x->x_filehandle, dir);
}

static void cyclone_click(t_cyclone *x, t_floatarg xpos, t_floatarg ypos,
			  t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    cyclone_doimport(x, 0, 0);
}

static void cyclone_import(t_cyclone *x, t_symbol *fn)
{
    cyclone_doimport(x, fn, 0);
}

static void cyclone_bang(t_cyclone *x)
{
    int i;
    fragile_class_printnames("hammer classes are: ",
			     cyclone_hammerndx, cyclone_sicklendx - 1);
    fragile_class_printnames("sickle classes are: ",
			     cyclone_sicklendx, cyclone_nettlesndx - 1);
    fragile_class_printnames("nettles are: ",
			     cyclone_nettlesndx, cyclone_dummiesndx - 1);
    if (cyclone_dproxy)
	pd_bang(cyclone_dproxy);
    else
	post("no replacement abstractions");
    if (cyclone_lastndx > cyclone_dummiesndx)
	post("send 'dummies' message to see the list of %d dummy classes",
	     /* cyclone_lastndx points to the "_dummy" sentinel class */
	     cyclone_lastndx - cyclone_dummiesndx);
    else
	post("no dummies");
}

static void cyclone_reps(t_cyclone *x)
{
    if (cyclone_dproxy)
	typedmess(cyclone_dproxy, gensym("reps"), 0, 0);
    else
	post("no replacement abstractions");
}

static void cyclone_dummies(t_cyclone *x)
{
    if (cyclone_lastndx > cyclone_dummiesndx)
	fragile_class_printnames("dummies are: ",
				 cyclone_dummiesndx, cyclone_lastndx);
    else
	post("no dummies");
}

static void cyclone_free(t_cyclone *x)
{
    hammerfile_free(x->x_filehandle);
}

static void *cyclone_new(t_symbol *s)
{
    t_cyclone *x = (t_cyclone *)pd_new(cyclone_class);
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, cyclone_readhook, 0, 0);
    x->x_dir = (s && s != &s_ ? s : canvas_getdir(x->x_filehandle->f_canvas));
    return (x);
}

void cyclone_setup(void)
{
    int hresult, sresult, dresult;
    hresult = sresult = dresult = LOADER_OK;
    if (canvas_getcurrent())
    {
	/* Loading the library by object creation is banned, because of a danger
	   of having some of the classes already loaded. LATER rethink. */
	loud_error(0, "apparently an attempt to create a 'cyclone' object");
	loud_errand(0, "without having cyclone library preloaded");
	return;
    }
    post("this is cyclone %s, %s %s build",
	 CYCLONE_VERSION, loud_ordinal(CYCLONE_BUILD), CYCLONE_RELEASE);
    cyclone_class = class_new(gensym("cyclone"),
			      (t_newmethod)cyclone_new,
			      (t_method)cyclone_free,
			      sizeof(t_cyclone), 0, A_DEFSYM, 0);
    class_addbang(cyclone_class, cyclone_bang);
    class_addmethod(cyclone_class, (t_method)cyclone_reps,
		    gensym("reps"), 0);
    class_addmethod(cyclone_class, (t_method)cyclone_dummies,
		    gensym("dummies"), 0);
    class_addmethod(cyclone_class, (t_method)cyclone_import,
		    gensym("import"), A_DEFSYM, 0);
    class_addmethod(cyclone_class, (t_method)cyclone_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    hammerfile_setup(cyclone_class, 0);

    cyclone_hammerndx = fragile_class_count();
    if (zgetfn(&pd_objectmaker, gensym("hammer")))
	loud_warning(0, "hammer is already loaded");
    else
	hresult = unstable_load_lib("", "hammer");

    cyclone_sicklendx = fragile_class_count();
    if (zgetfn(&pd_objectmaker, gensym("sickle")))
	loud_warning(0, "sickle is already loaded");
    else
	sresult = unstable_load_lib("", "sickle");

    cyclone_nettlesndx = fragile_class_count();
    allnettles_setup();

    cyclone_dummiesndx = fragile_class_count();
    if (zgetfn(&pd_objectmaker, gensym("dummies")))
	loud_warning(0, "dummies are already loaded");
    else
	dresult = unstable_load_lib("", "dummies");

    cyclone_lastndx = fragile_class_count() - 1;

    if (hresult == LOADER_NOFILE)
	loud_error(0, "hammer library is missing");
    else if (sresult == LOADER_NOFILE)
	loud_error(0, "sickle library is missing");
    else if (!zgetfn(&pd_objectmaker, gensym("hammer")) ||
	     !zgetfn(&pd_objectmaker, gensym("sickle")))
    {
	loud_error(0, "version mismatch");
	loud_errand(0,
		    "use a more recent Pd release (or recompile the cyclone).");
    }
    else if (dresult == LOADER_NOFILE)
	loud_warning(0, "dummies not found");
    else
    {
	t_symbol *s = gensym("_cc.dummies");
	if (s->s_thing && !s->s_next
	    && !strcmp(class_getname(*s->s_thing), "_cc.dummies"))
	    cyclone_dproxy = s->s_thing;
	else
	    bug("cyclone_setup");  /* FIXME */
    }
}
