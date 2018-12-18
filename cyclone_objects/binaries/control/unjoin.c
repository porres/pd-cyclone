//Derek Kwan 2017 - brand new object!

#include "m_pd.h"
#include <common/api.h>
#include <string.h>

#define UNJOIN_MINOUTLETS  2 //not including extra outlet
#define UNJOIN_MAXOUTLETS  255 //not including extra outlet
#define UNJOIN_MINOUTSIZE  1

typedef struct _unjoin
{
    t_object    x_obj;
    int         x_numouts; //number of outlets not including extra outlet
    int         x_outsize; 
    t_outlet  **x_outlets; //numouts + 1 for extra outlet
} t_unjoin;

static t_class *unjoin_class;

static void unjoin_list(t_unjoin *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    int outsize = x->x_outsize;
    int numouts = x->x_numouts;
    int numleft = argc;
    for(i = 0; i < numouts; i++)
    {
        if(numleft >= outsize)
        { // if only one... out float
            if(outsize == 1 && argv->a_type == A_FLOAT)
                outlet_float(x->x_outlets[i], argv->a_w.w_float);
            else if(argv->a_type == A_FLOAT) // if first is float... out list
                outlet_list(x->x_outlets[i],  &s_list, outsize, argv);
            else
                outlet_anything(x->x_outlets[i],  &s_ , outsize, argv);
            numleft -= outsize;
            argv += outsize;
        }
        else if ((numleft > 0) && (numleft < outsize))
        {
            if(numleft == 1 && argv->a_type == A_FLOAT)
                outlet_float(x->x_outlets[i], argv->a_w.w_float);
            else if(argv->a_type == A_FLOAT) // if first is float... out list
                outlet_list(x->x_outlets[i],  &s_list, numleft, argv);
            else
                outlet_anything(x->x_outlets[i], &s_, numleft, argv);
            numleft = 0;
            break;
        }
        else if(numleft <= 0) break;
    };
    if(numleft) // extra outlet
    {
        if(numleft == 1 && argv->a_type == A_FLOAT)
            outlet_float(x->x_outlets[i], argv->a_w.w_float);
        else if(argv->a_type == A_FLOAT) // if first is float... out list
            outlet_list(x->x_outlets[numouts],  &s_list, numleft, argv);
        else
            outlet_anything(x->x_outlets[numouts], &s_, numleft, argv);
    };
}

static void unjoin_anything(t_unjoin * x, t_symbol *s, int argc, t_atom * argv)
{   
    if(s)
    {
        int i;
        t_atom* newlist = t_getbytes((argc + 1) * sizeof(*newlist));
        SETSYMBOL(&newlist[0],s);
        for(i=0;i<argc;i++)
        {
            newlist[i+1] = argv[i];
        };
        unjoin_list(x, NULL, argc+1, newlist);
        t_freebytes(newlist, (argc + 1) * sizeof(*newlist));
    }
    else unjoin_list(x, NULL, argc, argv);
}

static void unjoin_float(t_unjoin *x, t_float f)
{
    outlet_float(x->x_outlets[0], f);
}

static void unjoin_symbol(t_unjoin *x, t_symbol *s)
{
    outlet_symbol(x->x_outlets[0], s);
}

static void unjoin_free(t_unjoin *x)
{
    if (x->x_outlets)
	freebytes(x->x_outlets, (x->x_numouts+1) * sizeof(*x->x_outlets));
}

static void unjoin_outsize(t_unjoin *x, t_float f){
    x->x_outsize = f < UNJOIN_MINOUTSIZE ? UNJOIN_MINOUTSIZE : (int)f;
}

static void *unjoin_new(t_symbol *s, int argc, t_atom * argv)
{
    t_unjoin *x = (t_unjoin *)pd_new(unjoin_class);
    
    int i; 
    int numouts = UNJOIN_MINOUTLETS;
    int outsize = UNJOIN_MINOUTSIZE;
    while(argc)
    {
        if(argv->a_type == A_FLOAT)
        {
            numouts = (int)argv->a_w.w_float;
            argc--;
            argv++;
        }
        else if(argv->a_type == A_SYMBOL)
        {
            t_symbol *cursym = argv->a_w.w_symbol;
            argc--;
            argv++;
            if(strcmp(cursym->s_name, "@outsize") == 0)
            {
                if(argc)
                {
                    if(argv->a_type == A_FLOAT)
                    {
                        outsize = (int)argv->a_w.w_float;
                        argc--;
                        argv++;
                    };
                };
            };

        }
        else
        {
         argc--;
         argv++;
        };

    };

    if(numouts < UNJOIN_MINOUTLETS) numouts = UNJOIN_MINOUTLETS;
    if(numouts > UNJOIN_MAXOUTLETS) numouts = UNJOIN_MAXOUTLETS;
    if(outsize < UNJOIN_MINOUTSIZE) outsize = UNJOIN_MINOUTSIZE;
    x->x_numouts = numouts;
    x->x_outsize = outsize;
    x->x_outlets = (t_outlet **)getbytes((numouts+1) * sizeof(t_outlet *));
    //<= for extra outlet
    for(i=0; i<= numouts; i++)
    {
        x->x_outlets[i] = outlet_new(&x->x_obj, &s_anything);

    };

    return (x);
}

CYCLONE_OBJ_API void unjoin_setup(void)
{
    unjoin_class = class_new(gensym("unjoin"),
			    (t_newmethod)unjoin_new,
			    (t_method)unjoin_free,
			    sizeof(t_unjoin), 0, A_GIMME, 0);
    class_addfloat(unjoin_class, unjoin_float);
    class_addlist(unjoin_class, unjoin_list);
    class_addanything(unjoin_class, unjoin_anything);
    class_addsymbol(unjoin_class, unjoin_symbol);
    class_addmethod(unjoin_class, (t_method)unjoin_outsize, gensym("outsize"), A_FLOAT, 0);
}
