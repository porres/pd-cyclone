/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "hammer/file.h"

#define TABLE_DEBUG

#define TABLE_INISIZE      256  /* LATER rethink */
#define TABLE_DEFLENGTH    128
#define TABLE_MINLENGTH      2
#define TABLE_MAXLENGTH  16383

typedef struct _tablecommon
{
    t_pd            c_pd;
    struct _table  *c_refs;  /* used in read-banging and dirty flag handling */
    int             c_embedflag;  /* common field (CHECKED in 'TEXT') */
    int             c_loading;
    int             c_relinked;
    int             c_size;    /* as allocated */
    int             c_length;  /* as used */
    int            *c_table;
    int             c_tableini[TABLE_INISIZE];
    t_symbol       *c_filename;
    t_canvas       *c_lastcanvas;
    t_hammerfile   *c_filehandle;
} t_tablecommon;

typedef struct _table
{
    t_object        x_ob;
    t_canvas       *x_canvas;
    t_symbol       *x_name;
    t_tablecommon  *x_common;
    t_float         x_value;
    int             x_valueset;
    int             x_head;
    t_hammerfile   *x_filehandle;
    t_outlet       *x_bangout;
    struct _table  *x_next;
} t_table;

static t_class *table_class;
static t_class *tablecommon_class;

static void tablecommon_modified(t_tablecommon *cc, int relinked)
{
    if (cc->c_loading)
	return;
    if (relinked)
    {
	cc->c_relinked = 1;
    }
    if (cc->c_embedflag)
    {
	t_table *x;
	for (x = cc->c_refs; x; x = x->x_next)
	    if (x->x_canvas && glist_isvisible(x->x_canvas))
		canvas_dirty(x->x_canvas, 1);
    }
}

static void tablecommon_doread(t_tablecommon *cc, t_symbol *fn, t_canvas *cv)
{
    /* FIXME */
}

static void tablecommon_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    tablecommon_doread((t_tablecommon *)z, fn, 0);
}

static void tablecommon_dowrite(t_tablecommon *cc, t_symbol *fn, t_canvas *cv)
{
    /* FIXME */
}

static void tablecommon_writehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    tablecommon_dowrite((t_tablecommon *)z, fn, 0);
}

static void table_embedhook(t_pd *z, t_binbuf *bb, t_symbol *bindsym)
{
    /* FIXME */
}

static void tablecommon_editorhook(t_pd *z, t_symbol *s, int ac, t_atom *av)
{
    /* FIXME */
}

static t_tablecommon *table_checkcommon(t_table *x)
{
    if (x->x_name &&
	x->x_common != (t_tablecommon *)pd_findbyclass(x->x_name,
						       tablecommon_class))
    {
	bug("table_checkcommon");
	return (0);
    }
    return (x->x_common);
}

static void table_unbind(t_table *x)
{
    /* LATER consider calling table_checkcommon(x) */
    t_tablecommon *cc = x->x_common;
    t_table *prev, *next;
    if ((prev = cc->c_refs) == x)
    {
	if (!(cc->c_refs = x->x_next))
	{
	    hammerfile_free(cc->c_filehandle);
	    cc->c_loading = 1;  /* disable dirty-flag handling, LATER rethink */
	    if (cc->c_table != cc->c_tableini)
		freebytes(cc->c_table, cc->c_size * sizeof(*cc->c_table));
	    if (x->x_name) pd_unbind(&cc->c_pd, x->x_name);
	    pd_free(&cc->c_pd);
	}
    }
    else if (prev)
    {
	while (next = prev->x_next)
	{
	    if (next == x)
	    {
		prev->x_next = next->x_next;
		break;
	    }
	    prev = next;
	}
    }
    x->x_common = 0;
    x->x_name = 0;
    x->x_next = 0;
}

static void table_bind(t_table *x, t_symbol *name)
{
    t_tablecommon *cc = 0;
    if (name == &s_)
	name = 0;
    else if (name)
	cc = (t_tablecommon *)pd_findbyclass(name, tablecommon_class);
    if (!cc)
    {
	cc = (t_tablecommon *)pd_new(tablecommon_class);
	cc->c_refs = 0;
	cc->c_embedflag = 0;
	cc->c_loading = 0;
	cc->c_size = TABLE_INISIZE;
	cc->c_length = TABLE_DEFLENGTH;
	cc->c_table = cc->c_tableini;
	if (name)
	{
	    pd_bind(&cc->c_pd, name);
	    /* LATER rethink canvas unpredictability */
	    tablecommon_doread(cc, name, x->x_canvas);
	}
	else
	{
	    cc->c_filename = 0;
	    cc->c_lastcanvas = 0;
	}
	cc->c_filehandle = hammerfile_new((t_pd *)cc, 0, tablecommon_readhook,
					  tablecommon_writehook,
					  tablecommon_editorhook);
    }
    x->x_common = cc;
    x->x_name = name;
    x->x_next = cc->c_refs;
    cc->c_refs = x;
}

static int table_rebind(t_table *x, t_symbol *name)
{
    t_tablecommon *cc;
    if (name && name != &s_ &&
	(cc = (t_tablecommon *)pd_findbyclass(name, tablecommon_class)))
    {
	table_unbind(x);
	x->x_common = cc;
	x->x_name = name;
	x->x_next = cc->c_refs;
	cc->c_refs = x;
	return (1);
    }
    else return (0);
}

static void table_dooutput(t_table *x, int ndx)
{
    t_tablecommon *cc = x->x_common;
    /* CHECKED ndx silently truncated */
    if (ndx < 0)
	ndx = 0;
    else if (ndx > cc->c_length)
	ndx = cc->c_length - 1;
    outlet_float(((t_object *)x)->ob_outlet, (t_float)cc->c_table[ndx]);
}

static void table_setvalue(t_table *x, int ndx, int v)
{
    t_tablecommon *cc = x->x_common;
    /* CHECKED ndx silently truncated */
    if (ndx < 0)
	ndx = 0;
    else if (ndx > cc->c_length)
	ndx = cc->c_length - 1;
    cc->c_table[ndx] = v;  /* CHECKED no truncation */
}

static void table_bang(t_table *x)
{
    /* FIXME */
}

static void table_float(t_table *x, t_float f)
{
    int ndx = (int)f;  /* CHECKED floats are truncated */
    if (x->x_valueset)
    {
	table_setvalue(x, ndx, x->x_value);
	x->x_valueset = 0;
    }
    else table_dooutput(x, ndx);
    /* CHECKME if head is updated */
    x->x_head = ndx;
}

static void table_ft1(t_table *x, t_floatarg f)
{
    x->x_value = (int)f;  /* CHECKED floats are truncated */
    x->x_valueset = 1;
}

static void table_cancel(t_table *x)
{
    x->x_valueset = 0;
}

static void table_clear(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    int ndx = cc->c_length;
    int *ptr = cc->c_table;
    while (ndx--) *ptr++ = 0;
    /* CHECKME head */
}

static void table_next(t_table *x)
{
    table_dooutput(x, x->x_head++);
    if (x->x_head >= x->x_common->c_length)
	x->x_head = 0;
}

static void table_prev(t_table *x)
{
    table_dooutput(x, x->x_head--);
    if (x->x_head < 0)
	x->x_head = x->x_common->c_length - 1;
}

static void table_goto(t_table *x, t_floatarg f)
{
    t_tablecommon *cc = x->x_common;
    int ndx = (int)f;  /* CHECKED floats are truncated */
    /* CHECKED ndx silently truncated */
    if (ndx < 0)
	ndx = 0;
    else if (ndx > cc->c_length)
	ndx = cc->c_length - 1;
    x->x_head = ndx;
}

static void table_length(t_table *x)
{
    outlet_float(((t_object *)x)->ob_outlet, (t_float)x->x_common->c_length);
}

static void table_min(t_table *x)
{
    /* FIXME */
}

static void table_max(t_table *x)
{
    /* FIXME */
}

static void table_refer(t_table *x, t_symbol *s)
{
    if (!table_rebind(x, s))
    {
	/* LATER consider complaining */
    }
}

static void table_flags(t_table *x, t_float f1, t_float f2)
{
    int i1;
    if (loud_checkint((t_pd *)x, f1, &i1, gensym("flags")))
    {
	t_tablecommon *cc = x->x_common;
	cc->c_embedflag = (i1 != 0);
    }
    /* FIXME don't save flag */
}

static void table_read(t_table *x, t_symbol *s)
{
    t_tablecommon *cc = x->x_common;
    if (s && s != &s_)
	tablecommon_doread(cc, s, x->x_canvas);
    else
	hammerpanel_open(cc->c_filehandle, 0);
}

static void table_write(t_table *x, t_symbol *s)
{
    t_tablecommon *cc = x->x_common;
    if (s && s != &s_)
	tablecommon_dowrite(cc, s, x->x_canvas);
    else
	hammerpanel_save(cc->c_filehandle, 0, 0);  /* CHECKME default name */
}

static void table_dump(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    /* FIXME */
}

static void table_open(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    /* FIXME */
}

static void table_wclose(t_table *x)
{
    /* FIXME */
}

static void table_click(t_table *x, t_floatarg xpos, t_floatarg ypos,
			t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    table_open(x);
}

#ifdef TABLE_DEBUG
static void table_debug(t_table *x, t_floatarg f)
{
    t_tablecommon *cc = table_checkcommon(x);
    if (cc)
    {
	t_table *x1 = cc->c_refs;
	int i = 0;
	while (x1) i++, x1 = x1->x_next;
	post("refcount %d", i);
    }
}
#endif

static void table_free(t_table *x)
{
    hammerfile_free(x->x_filehandle);
    table_unbind(x);
}

static void *table_new(t_symbol *s)
{
    t_table *x = (t_table *)pd_new(table_class);
    static int warned = 0;
    if (!warned)
    {
	loud_warning((t_pd *)x, 0, "Table is not ready yet");
	warned = 1;
    }
    x->x_canvas = canvas_getcurrent();
    x->x_valueset = 0;
    x->x_head = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    x->x_filehandle = hammerfile_new((t_pd *)x, table_embedhook, 0, 0, 0);
    table_bind(x, s);
    return (x);
}

void Table_setup(void)
{
    table_class = class_new(gensym("Table"),
			    (t_newmethod)table_new,
			    (t_method)table_free,
			    sizeof(t_table), 0, A_DEFSYM, 0);
    class_addbang(table_class, table_bang);
    class_addfloat(table_class, table_float);
    class_addmethod(table_class, (t_method)table_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_cancel,
		    gensym("cancel"), 0);
    class_addmethod(table_class, (t_method)table_clear,
		    gensym("clear"), 0);
    class_addmethod(table_class, (t_method)table_next,
		    gensym("next"), 0);
    class_addmethod(table_class, (t_method)table_prev,
		    gensym("prev"), 0);
    class_addmethod(table_class, (t_method)table_goto,
		    gensym("goto"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_length,
		    gensym("length"), 0);
    class_addmethod(table_class, (t_method)table_min,
		    gensym("min"), 0);
    class_addmethod(table_class, (t_method)table_max,
		    gensym("max"), 0);
    class_addmethod(table_class, (t_method)table_refer,
		    gensym("refer"), A_SYMBOL, 0);
    class_addmethod(table_class, (t_method)table_flags,
		    gensym("flags"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_read,
		    gensym("read"), A_DEFSYM, 0);
    class_addmethod(table_class, (t_method)table_write,
		    gensym("write"), A_DEFSYM, 0);
    class_addmethod(table_class, (t_method)table_dump,
		    gensym("dump"), 0);
    class_addmethod(table_class, (t_method)table_open,
		    gensym("open"), 0);
    class_addmethod(table_class, (t_method)table_wclose,
		    gensym("wclose"), 0);
    class_addmethod(table_class, (t_method)table_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
#ifdef TABLE_DEBUG
    class_addmethod(table_class, (t_method)table_debug,
		    gensym("debug"), A_DEFFLOAT, 0);
#endif
    hammerfile_setup(table_class, 1);
    tablecommon_class = class_new(gensym("Table"), 0, 0,
				 sizeof(t_tablecommon), CLASS_PD, 0);
    /* this call is a nop (tablecommon does not embed, and the hammerfile
       class itself has been already set up above), but it is better to
       have it around, just in case... */
    hammerfile_setup(tablecommon_class, 0);
}
