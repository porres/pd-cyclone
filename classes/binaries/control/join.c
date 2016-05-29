#include <m_pd.h>

static t_class *join_class;
static t_class* join_inlet_class;

struct _join_inlet;

typedef struct _join
{
    t_object            x_obj;
    t_int               x_n;
    t_atom*             x_vec;
    t_atom*             x_out;
    struct _join_inlet*  x_ins;
} t_join;

typedef struct _join_inlet
{
    t_class*    x_pd;
    t_atom*     x_atoms;
    t_int       x_max;
    t_int       x_int;
    t_join*     x_owner;
} t_join_inlet;


static void *join_new(t_symbol *s, int argc, t_atom *argv)
{
    int i;
    t_join *x = (t_join *)pd_new(join_class);
    t_atom defarg[2];
    if(!argc)
    {
        argv = defarg;
        argc = 2;
        SETFLOAT(&defarg[0], 0);
        SETFLOAT(&defarg[1], 0);
    }
    
    x->x_n   = argc;
    x->x_vec = (t_atom *)getbytes(argc * sizeof(*x->x_vec));
    x->x_out = (t_atom *)getbytes(argc * sizeof(*x->x_out));
    x->x_ins = (t_join_inlet *)getbytes(argc * sizeof(*x->x_ins));
    
    for(i = 0; i < x->x_n; ++i)
    {
        if(argv[i].a_type == A_FLOAT)
        {
            x->x_vec[i].a_type      = A_FLOAT;
            x->x_vec[i].a_w.w_float = argv[i].a_w.w_float;
            x->x_ins[i].x_pd    = join_inlet_class;
            x->x_ins[i].x_atoms = x->x_vec+i;
            x->x_ins[i].x_max   = x->x_n-i;
            x->x_ins[i].x_owner = x;
            x->x_ins[i].x_int = 0;
            inlet_new((t_object *)x, &(x->x_ins[i].x_pd), 0, 0);
        }
        else if(argv[i].a_type == A_SYMBOL)
        {
        if(argv[i].a_w.w_symbol == gensym("f")) // only "f" arg converts to float
            {
                x->x_vec[i].a_type      = A_FLOAT;
                x->x_vec[i].a_w.w_float = 0.f;
                x->x_ins[i].x_pd    = join_inlet_class;
                x->x_ins[i].x_atoms = x->x_vec+i;
                x->x_ins[i].x_max   = x->x_n-i;
                x->x_ins[i].x_owner = x;
                inlet_new((t_object *)x, &(x->x_ins[i].x_pd), 0, 0);
            }
            else
            {
                x->x_vec[i].a_type       = A_SYMBOL;
                x->x_vec[i].a_w.w_symbol = argv[i].a_w.w_symbol; // loads symbol from arg
                x->x_ins[i].x_pd    = join_inlet_class;
                x->x_ins[i].x_atoms = x->x_vec+i;
                x->x_ins[i].x_max   = x->x_n-i;
                x->x_ins[i].x_owner = x;
                x->x_ins[i].x_int = 0;
                inlet_new((t_object *)x, &(x->x_ins[i].x_pd), 0, 0);
            }
        }
    }
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void join_bang(t_join *x)
{
    int i;
    for(i = 0; i < x->x_n; ++i)
    {
        x->x_out[i] = x->x_vec[i];
    }
    outlet_list(x->x_obj.ob_outlet, &s_list, x->x_n, x->x_out);
}

static void join_copy(t_join *x, int ndest, t_atom* dest, int nsrc, t_atom* src)
{
    int i;
    for(i = 0; i < ndest && i < nsrc; ++i)
    {
        if(src[i].a_type == A_FLOAT)
        {
            if(dest[i].a_type == A_FLOAT)
            {
                dest[i].a_w.w_float = src[i].a_w.w_float;
            }
            else if(dest[i].a_type == A_SYMBOL)
            {
                dest[i].a_w.w_symbol = &s_; // float becomes blank symbol
            }
        }
        else if(src[i].a_type == A_SYMBOL)
        {
            if(dest[i].a_type == A_SYMBOL)
            {
                dest[i].a_w.w_symbol = src[i].a_w.w_symbol;
            }
            else if(dest[i].a_type == A_FLOAT)
            {
            dest[i].a_w.w_float = 0; // symbol becomes 0
            }
        }
    }
}

static void join_free(t_join *x)
{
    freebytes(x->x_vec, x->x_n * sizeof(*x->x_vec));
    freebytes(x->x_out, x->x_n * sizeof(*x->x_out));
    freebytes(x->x_ins, x->x_n * sizeof(*x->x_ins));
}


static void join_inlet_bang(t_join_inlet *x)
{
    join_bang(x->x_owner);
}

static void join_inlet_float(t_join_inlet *x, float f)
{
    if(x->x_int)
    {
        x->x_atoms->a_w.w_float = (int)f;
        join_bang(x->x_owner);
    }
    else if(x->x_atoms->a_type == A_FLOAT)
    {
        x->x_atoms->a_w.w_float = f;
        join_bang(x->x_owner);
    }
    else if(x->x_atoms->a_type == A_SYMBOL)
    {
        x->x_atoms->a_w.w_symbol =  &s_; // float becomes blank symbol
        join_bang(x->x_owner);
    }
}

static void join_inlet_symbol(t_join_inlet *x, t_symbol* s)
{
    if(x->x_atoms->a_type == A_SYMBOL)
    {
        x->x_atoms->a_w.w_symbol = s;
        join_bang(x->x_owner);
    }
    else if(x->x_atoms->a_type == A_FLOAT)
    {
        x->x_atoms->a_w.w_float = 0; // symbol becomes 0
        join_bang(x->x_owner);
    }
}

static void join_inlet_list(t_join_inlet *x, t_symbol* s, int argc, t_atom* argv)
{
    join_copy(x->x_owner, x->x_max, x->x_atoms, argc, argv);
    join_bang(x->x_owner);
}

static void join_inlet_anything(t_join_inlet *x, t_symbol* s, int argc, t_atom* argv)
{
    if(x->x_atoms[0].a_type == A_SYMBOL)
    {
        x->x_atoms[0].a_w.w_symbol = s;
    }
    else if (x->x_atoms[0].a_type == A_FLOAT)
    {
        x->x_atoms[0].a_w.w_float = 0; // symbol becomes 0
    }
    join_copy(x->x_owner, x->x_max-1, x->x_atoms+1, argc, argv);
    join_bang(x->x_owner);
}

static void join_inlet_set(t_join_inlet *x, t_symbol* s, int argc, t_atom* argv)
{
    join_copy(x->x_owner, x->x_max, x->x_atoms, argc, argv);
}

extern void join_setup(void)
{
    t_class* c = NULL;
    
    c = class_new(gensym("join-inlet"), 0, 0, sizeof(t_join_inlet), CLASS_PD, 0);
    if(c)
    {
        class_addbang(c,    (t_method)join_inlet_bang);
        class_addfloat(c,   (t_method)join_inlet_float);
        class_addsymbol(c,  (t_method)join_inlet_symbol);
        class_addlist(c,    (t_method)join_inlet_list);
        class_addanything(c,(t_method)join_inlet_anything);
        class_addmethod(c,  (t_method)join_inlet_set, gensym("set"), A_GIMME, 0);
    }
    join_inlet_class = c;
    
    c = class_new(gensym("join"), (t_newmethod)join_new, (t_method)join_free, sizeof(t_join), CLASS_NOINLET, A_GIMME, 0);
    join_class = c;
}