/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Write access is totally encapsulated in tablecommon calls, in order
   to simplify proper handling of the distribution cache.  Direct read
   access from table calls is allowed (for speed). */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/rand.h"
#include "hammer/file.h"

#define TABLE_DEBUG

#define TABLE_INISIZE      256  /* LATER rethink */
#define TABLE_DEFLENGTH    128  /* CHECKED */
#define TABLE_MINLENGTH      2  /* CHECKED */
#define TABLE_MAXLENGTH  16383  /* CHECKED, LATER rethink */
#define TABLE_MINRANGE       2  /* CHECKED */
#define TABLE_MAXQ       32768  /* CHECKME */

typedef struct _tablecommon
{
    t_pd            c_pd;
    struct _table  *c_refs;
    int             c_increation;
    int             c_volatile;
    int             c_selfmodified;
    int             c_entered;  /* a counter, LATER rethink */
    /* CHECKED flags, etc. are common fields */
    int             c_visflag;
    int             c_embedflag;
    int             c_dontsaveflag;
    int             c_notenamesflag;
    int             c_signedflag;
    int             c_range;
    int             c_left;
    int             c_top;
    int             c_right;
    int             c_bottom;
    int             c_size;    /* as allocated */
    int             c_length;  /* as used */
    int            *c_table;
    int             c_tableini[TABLE_INISIZE];
    int             c_cacheisfresh;
    int            *c_cache;
    int             c_cacheini[TABLE_INISIZE];
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
    int             x_loadflag;
    int             x_loadndx;
    unsigned int    x_seed;
    t_hammerfile   *x_filehandle;
    t_outlet       *x_bangout;
    struct _table  *x_next;
} t_table;

static t_class *table_class;
static t_class *tablecommon_class;

static void tablecommon_modified(t_tablecommon *cc, int relocated)
{
    if (cc->c_increation)
	return;
    if (relocated)
    {
	cc->c_volatile = 1;
    }
    if (cc->c_embedflag)
    {
	t_table *x;
	for (x = cc->c_refs; x; x = x->x_next)
	    if (x->x_canvas && glist_isvisible(x->x_canvas))
		canvas_dirty(x->x_canvas, 1);
    }
}

static int tablecommon_getindex(t_tablecommon *cc, int ndx)
{
    int mx = cc->c_length - 1;
    /* CHECKED ndx silently clipped */
    return (ndx < 0 ? 0 : (ndx > mx ? mx : ndx));
}

static int tablecommon_getvalue(t_tablecommon *cc, int ndx)
{
    int mx = cc->c_length - 1;
    /* CHECKED ndx silently clipped */
    return (cc->c_table[ndx < 0 ? 0 : (ndx > mx ? mx : ndx)]);
}

static void tablecommon_setvalue(t_tablecommon *cc, int ndx, int v)
{
    int mx = cc->c_length - 1;
    /* CHECKED ndx silently clipped, value not clipped */
    cc->c_table[ndx < 0 ? 0 : (ndx > mx ? mx : ndx)] = v;
    tablecommon_modified(cc, 0);
}

static int tablecommon_loadvalue(t_tablecommon *cc, int ndx, int v)
{
    /* CHECKME */
    if (ndx < cc->c_length)
    {
	cc->c_table[ndx] = v;
	tablecommon_modified(cc, 0);
	return (1);
    }
    else return (0);
}

static void tablecommon_setall(t_tablecommon *cc, int v)
{
    int ndx = cc->c_length;
    int *ptr = cc->c_table;
    while (ndx--) *ptr++ = v;
    tablecommon_modified(cc, 0);
}

static void tablecommon_setatoms(t_tablecommon *cc, int ndx, int ac, t_atom *av)
{
    if (ac > 1 && av->a_type == A_FLOAT)
    {
	/* CHECKME resizing */
	int last = tablecommon_getindex(cc, ndx + ac - 1);
	int *ptr = cc->c_table + ndx;
	for (av++; ndx <= last; ndx++, av++)
	     *ptr++ = (av->a_type == A_FLOAT ? (int)av->a_w.w_float : 0);
	tablecommon_modified(cc, 0);
    }
}

static void tablecommon_setlength(t_tablecommon *cc, int length)
{
    int relocate;
    if (length < TABLE_MINLENGTH)
	length = TABLE_MINLENGTH;
    else if (length > TABLE_MAXLENGTH)
	length = TABLE_MAXLENGTH;
    if (relocate = (length > cc->c_size))
    {
	cc->c_table = grow_nodata(&length, &cc->c_size, cc->c_table,
				  TABLE_INISIZE, cc->c_tableini,
				  sizeof(*cc->c_table));
	/* FIXME cache */
    }
    cc->c_length = length;
    tablecommon_setall(cc, 0);  /* CHECKME */
    /* CHECKME head */
    tablecommon_modified(cc, relocate);
}

static int tablecommon_quantile(t_tablecommon *cc, float f)
{
    /* FIXME */
    return (0);
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
    t_table *x = (t_table *)z;
    t_tablecommon *cc = x->x_common;
    if (cc->c_embedflag)
    {
	int ndx = 0, left = cc->c_length;
	int *ptr = cc->c_table;
	binbuf_addv(bb, "ssi;", bindsym, gensym("size"), cc->c_length);
	binbuf_addv(bb, "ssiiii;", bindsym, gensym("flags"), 1,
		    cc->c_dontsaveflag, cc->c_notenamesflag, cc->c_signedflag);
	binbuf_addv(bb, "ssi;", bindsym, gensym("tabrange"), cc->c_range);
	binbuf_addv(bb, "ssiiiii;", bindsym, gensym("_coords"),
		    cc->c_left, cc->c_top, cc->c_right, cc->c_bottom,
		    cc->c_visflag);
	while (left > 0)
	{
	    int cnt = (left > 128 ? 128 : left);
	    left -= cnt;
	    ndx += cnt;
	    binbuf_addv(bb, "ssi", bindsym, gensym("set"), ndx);
	    while (cnt--)
	    {
		t_atom at;
		SETFLOAT(&at, (float)*ptr);
		binbuf_add(bb, 1, &at);
		ptr++;
	    }
	    binbuf_addsemi(bb);
	}
    }
}

static void tablecommon_editorhook(t_pd *z, t_symbol *s, int ac, t_atom *av)
{
    /* FIXME */
}

static void tablecommon_free(t_tablecommon *cc)
{
    if (cc->c_table != cc->c_tableini)
	freebytes(cc->c_table, cc->c_size * sizeof(*cc->c_table));
    if (cc->c_cache != cc->c_cacheini)
	freebytes(cc->c_cache, cc->c_size * sizeof(*cc->c_cache));
}

static void *tablecommon_new(void)
{
    t_tablecommon *cc = (t_tablecommon *)pd_new(tablecommon_class);
    cc->c_visflag = 0;
    cc->c_embedflag = 0;
    cc->c_dontsaveflag = 0;
    cc->c_notenamesflag = 0;
    cc->c_signedflag = 0;
    cc->c_size = TABLE_INISIZE;
    cc->c_length = TABLE_DEFLENGTH;
    cc->c_table = cc->c_tableini;
    cc->c_cache = cc->c_cacheini;
    return (cc);
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
	    /* disable canvas dirty-flag handling, LATER rethink */
	    cc->c_increation = 1;
	    tablecommon_free(cc);
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
	cc = (t_tablecommon *)tablecommon_new();
	cc->c_refs = 0;
	cc->c_increation = 0;
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
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_getvalue(x->x_common, ndx));
}

static void table_bang(t_table *x)
{
    /* CHECKME */
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_quantile(x->x_common,
					       rand_unipolar(&x->x_seed)));
}

static void table_float(t_table *x, t_float f)
{
    if (x->x_loadflag)
    {
	/* CHECKME */
	if (tablecommon_loadvalue(x->x_common, x->x_loadndx, (int)f))
	    x->x_loadndx++;
    }
    else
    {
	int ndx = (int)f;  /* CHECKED floats are truncated */
	if (x->x_valueset)
	{
	    tablecommon_setvalue(x->x_common, ndx, x->x_value);
	    x->x_valueset = 0;
	}
	else table_dooutput(x, ndx);
	/* CHECKME if head is updated */
	x->x_head = ndx;
    }
}

static void table_ft1(t_table *x, t_floatarg f)
{
    x->x_value = (int)f;  /* CHECKED floats are truncated */
    x->x_valueset = 1;
}

static void table_size(t_table *x, t_floatarg f)
{
    tablecommon_setlength(x->x_common, (int)f);
}

static void table_set(t_table *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac > 1 && av->a_type == A_FLOAT)
    {
	int ndx = tablecommon_getindex(x->x_common, (int)av->a_w.w_float);
	tablecommon_setatoms(x->x_common, ndx, ac - 1, av + 1);
    }
}

static void table_flags(t_table *x, t_symbol *s, int ac, t_atom *av)
{
    t_tablecommon *cc = x->x_common;
    int i = 0, v;
    while (ac && av->a_type == A_FLOAT
	&& loud_checkint((t_pd *)x, av->a_w.w_float, &v, gensym("flags")))
    {
	/* CHECKED order, modifying only explicitly specified flags */
	if (i == 0)
	    cc->c_embedflag = (v != 0);
	else if (i == 1)
	    cc->c_dontsaveflag = (v != 0);
	else if (i == 2)
	    cc->c_notenamesflag = (v != 0);
	else if (i == 3)
	    cc->c_signedflag = (v != 0);
	else
	    break;
	i++; ac--; av++;
    }
}

static void table_tabrange(t_table *x, t_floatarg f)
{
    int i = (int)f;
    x->x_common->c_range = (i > TABLE_MINRANGE ? i : TABLE_MINRANGE);
}

static void table__coords(t_table *x, t_floatarg fl, t_floatarg ft,
			  t_floatarg fr, t_floatarg fb, t_floatarg fv)
{
    t_tablecommon *cc = x->x_common;
    /* FIXME constraints */
    cc->c_left = (int)fl;
    cc->c_top = (int)ft;
    cc->c_right = (int)fr;
    cc->c_bottom = (int)fb;
    cc->c_visflag = ((int)fv != 0);
}

static void table_cancel(t_table *x)
{
    x->x_valueset = 0;
}

static void table_clear(t_table *x)
{
    tablecommon_setall(x->x_common, 0);
    /* CHECKME head */
}

static void table_const(t_table *x, t_floatarg f)
{
    tablecommon_setall(x->x_common, (int)f);
    /* CHECKME head */
}

static void table_load(t_table *x)
{
    x->x_loadflag = 1;
    x->x_loadndx = 0;  /* CHECKED rewind, head not affected */
}

static void table_normal(t_table *x)
{
    x->x_loadflag = 0;
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
    /* CHECKED floats are truncated */
    x->x_head = tablecommon_getindex(x->x_common, (int)f);
}

static void table_send(t_table *x, t_symbol *s, t_floatarg f)
{
    /* FIXME */
}

static void table_length(t_table *x)
{
    outlet_float(((t_object *)x)->ob_outlet, (t_float)x->x_common->c_length);
}

static void table_sum(t_table *x)
{
    /* FIXME */
}

static void table_min(t_table *x)
{
    /* FIXME */
}

static void table_max(t_table *x)
{
    /* FIXME */
}

static void table_getbits(t_table *x, t_floatarg f1,
			  t_floatarg f2, t_floatarg f3)
{
    /* FIXME */
}

static void table_setbits(t_table *x, t_floatarg f1,
			  t_floatarg f2, t_floatarg f3, t_floatarg f4)
{
    /* FIXME */
}

static void table_inv(t_table *x, t_floatarg f)
{
    /* FIXME */
}

static void table_quantile(t_table *x, t_floatarg f)
{
    /* CHECKME */
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_quantile(x->x_common,
					       f / ((float)TABLE_MAXQ)));
}

static void table_fquantile(t_table *x, t_floatarg f)
{
    /* CHECKME constraints */
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_quantile(x->x_common, f));
}

static void table_dump(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    t_outlet *out = ((t_object *)x)->ob_outlet;
    int ndx = cc->c_length;
    int *ptr = cc->c_table;
    /* CHECKME */
    while (ndx--)
    {
	outlet_float(out, (t_float)*ptr++);
	/* FIXME ptr may be invalid after outlet_float()... consider calling
	   tablecommon_getindex() rather than patching in selfmod tests */
    }
}

static void table_refer(t_table *x, t_symbol *s)
{
    if (!table_rebind(x, s))
    {
	/* LATER consider complaining */
    }
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
    x->x_loadflag = 0;
    rand_seed(&x->x_seed, 0);
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
    class_addmethod(table_class, (t_method)table_size,
		    gensym("size"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_set,
		    gensym("set"), A_GIMME, 0);
    class_addmethod(table_class, (t_method)table_flags,
		    gensym("flags"), A_GIMME, 0);
    class_addmethod(table_class, (t_method)table_tabrange,
		    gensym("tabrange"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table__coords,
		    gensym("_coords"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_cancel,
		    gensym("cancel"), 0);
    class_addmethod(table_class, (t_method)table_clear,
		    gensym("clear"), 0);
    class_addmethod(table_class, (t_method)table_const,
		    gensym("const"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_load,
		    gensym("load"), 0);
    class_addmethod(table_class, (t_method)table_normal,
		    gensym("normal"), 0);
    class_addmethod(table_class, (t_method)table_next,
		    gensym("next"), 0);
    class_addmethod(table_class, (t_method)table_prev,
		    gensym("prev"), 0);
    class_addmethod(table_class, (t_method)table_goto,
		    gensym("goto"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_send,
		    gensym("send"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_length,
		    gensym("length"), 0);
    class_addmethod(table_class, (t_method)table_sum,
		    gensym("sum"), 0);
    class_addmethod(table_class, (t_method)table_min,
		    gensym("min"), 0);
    class_addmethod(table_class, (t_method)table_max,
		    gensym("max"), 0);
    class_addmethod(table_class, (t_method)table_getbits,
		    gensym("getbits"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_setbits,
		    gensym("setbits"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_inv,
		    gensym("inv"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_quantile,
		    gensym("quantile"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_fquantile,
		    gensym("fquantile"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_dump,
		    gensym("dump"), 0);
    class_addmethod(table_class, (t_method)table_refer,
		    gensym("refer"), A_SYMBOL, 0);
    class_addmethod(table_class, (t_method)table_read,
		    gensym("read"), A_DEFSYM, 0);
    class_addmethod(table_class, (t_method)table_write,
		    gensym("write"), A_DEFSYM, 0);
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
