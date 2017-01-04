/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "hammer/gui.h"

//2016 note: now works with anything, pre v3: only floats - Derek Kwan
//props to Thomas Musil's iemlib_anything to figure out how to do handle anythings

//mousefilter_doup called after float method called -> mousedowns on slider leak through 


//stack size
#define MOUSEFILTER_STACK 256
//max alloc size
#define MOUSEFILTER_MAX 1024

typedef struct _mousefilter
{
    t_object   x_obj;
    int        x_isup;
    int        x_ispending;
    t_atom *   x_value;
    t_atom     x_valstack[MOUSEFILTER_STACK];
    int        x_allocsz; //size of allocated (or not) value
    int        x_valuesz; //size of stored value
    int        x_heaped; //if x_value is allocated
    int        x_bangpend; //if bang is pending
    t_symbol*  x_selector;

} t_mousefilter;

static t_class *mousefilter_class;

//value allocation helper function
static void mousefilter_alloc(t_mousefilter *x, int argc){
   int cursize = x->x_allocsz;
    int heaped = x->x_heaped;


    if(heaped && argc <= MOUSEFILTER_STACK){
        //if x_value is pointing to a heap and incoming list can fit into MOUSEFILTER_STACK
        //deallocate x_value and point it to stack, update status
        freebytes((x->x_value), (cursize) * sizeof(t_atom));
        x->x_value = x->x_valstack;
        x->x_heaped = 0;
        x->x_allocsz = MOUSEFILTER_STACK;
        }
    else if(heaped && argc > MOUSEFILTER_STACK && argc > cursize){
        //if already heaped, incoming list can't fit into MOUSEFILTER_STACK and can't fit into allocated t_atom
        //reallocate to accomodate larger list, update status
        
        int toalloc = argc; //size to allocate
        //bounds checking for maxsize
        if(toalloc > MOUSEFILTER_MAX){
            toalloc = MOUSEFILTER_MAX;
        };
        x->x_value = (t_atom *)resizebytes(x->x_value, cursize * sizeof(t_atom), toalloc * sizeof(t_atom));
        x->x_allocsz = toalloc;
    }
    else if(!heaped && argc > MOUSEFILTER_STACK){
        //if not heaped and incoming list can't fit into MOUSEFILTER_STACK
        //allocate and update status

        int toalloc = argc; //size to allocate
        //bounds checking for maxsize
        if(toalloc > MOUSEFILTER_MAX){
            toalloc = MOUSEFILTER_MAX;
        };
        x->x_value = (t_atom *)getbytes(toalloc * sizeof(t_atom));
        x->x_heaped = 1;
        x->x_allocsz = toalloc;
    };

}

//argc == 0, use for incoming bangs

static void mousefilter_output(t_mousefilter *x, t_symbol *s, int argc, t_atom *argv){
    if(s){ 
        if(argc == 0 && s == &s_bang){
            outlet_bang(x->x_obj.ob_outlet);
            x->x_bangpend = 0;
        }
        else if(argc == 1){
            if(argv -> a_type == A_FLOAT){
                t_float f = atom_getfloatarg(0, argc, argv);
	        outlet_float(x->x_obj.ob_outlet, f);
            }
            else if(argv->a_type == A_SYMBOL){
                t_symbol *symb = atom_getsymbolarg(0, argc, argv);
	        outlet_symbol(x->x_obj.ob_outlet, symb);
            };
        }
        else{
	        outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
        };
    };

}

//argc ==0 use for incoming bangs
static void mousefilter_store(t_mousefilter *x, t_symbol *s, int argc, t_atom *argv){
        
    if(s){
        x->x_selector = s;
    };
    if(argc || s != &s_bang){
        if(argc != x->x_valuesz){
        //if not same size as stored value, makes sure it can fit
            mousefilter_alloc(x, argc);
        };
        x->x_valuesz = argc;
        int i;
        for(i=0; i<argc; i++){
            if(argv -> a_type == A_FLOAT){
                t_float f = atom_getfloatarg(i, argc, argv);
                SETFLOAT(&x->x_value[i], f);
            }
            else if(argv -> a_type == A_SYMBOL){
                t_symbol *symb = atom_getsymbolarg(i, argc,argv);
                SETSYMBOL(&x->x_value[i], symb);
            };
        
        };

    }
    else{
        //else make a note that a bang is pending
        x->x_bangpend = 1;
    };
}

//the main doer of stuff
static void mousefilter_anything(t_mousefilter *x, t_symbol *s, int argc, t_atom *argv)
{

    
    if (x->x_isup && x->x_ispending){
        mousefilter_output(x, s, argc, argv);
           }
    else{
        x->x_ispending = 1;
        mousefilter_store(x, s, argc, argv);
    };
}
static void mousefilter_bang(t_mousefilter *x){
    mousefilter_anything(x, &s_bang, 0, 0);
}

static void mousefilter_doup(t_mousefilter *x, t_floatarg f)
{
    if ((x->x_isup = (int)f) && x->x_ispending)
    {
        x->x_ispending = 0;
        if(x->x_bangpend){
            mousefilter_output(x, &s_bang, 0, 0);
        }
        else{
            mousefilter_output(x, x->x_selector, x->x_valuesz, x->x_value);
        };
    }
}

static void mousefilter_free(t_mousefilter *x)
{
    if(x->x_heaped){

        freebytes((x->x_value), (x->x_allocsz) * sizeof(t_atom));
    };
    hammergui_unbindmouse((t_pd *)x);
}

static void *mousefilter_new(void)
{
    t_mousefilter *x = (t_mousefilter *)pd_new(mousefilter_class);
    x->x_isup = 0;  /* LATER rethink */
    x->x_ispending = 0;
    x->x_heaped = 0;
    x->x_allocsz = MOUSEFILTER_STACK;
    x->x_valuesz = 0;
    x->x_bangpend = 0;
    x->x_value = x->x_valstack;
    x->x_selector = &s_bang;
    outlet_new(&x->x_obj, &s_anything);
    hammergui_bindmouse((t_pd *)x);
    return (x);
}

void mousefilter_setup(void)
{
    mousefilter_class = class_new(gensym("mousefilter"),
				  (t_newmethod)mousefilter_new,
				  (t_method)mousefilter_free,
				  sizeof(t_mousefilter), 0, 0);
    class_addbang(mousefilter_class, mousefilter_bang);
    class_addanything(mousefilter_class, mousefilter_anything);
    class_addmethod(mousefilter_class, (t_method)mousefilter_doup,
                    gensym("_up"), A_FLOAT, 0);
}
