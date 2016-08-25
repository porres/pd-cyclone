/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

//Derek Kwan 2016 - redoing fromsymbol_symbol, fromsymbol_separator, adding mtok()

#include <stdlib.h>
#include <string.h>
#include "m_pd.h"

char *mtok(char *input, char *delimiter) {
    // adapted from stack overflow - Derek Kwan
    // designed to work like strtok
    static char *string;
    
    //if not passed null, use static var
    if (input != NULL){
        string = input;
    };

    //if reached the end, just return the static var, i think
    if (string == NULL){
        return string;
    };

    //return pointer of first occurence of delim
    //added, keep going until first non delim
    char *end = strstr(string, delimiter);
    while(end == string){
        *end = '\0';
        string = end + strlen(delimiter);
        end = strstr(string, delimiter);
    };
    //if not found, just return the string
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    //set thing pointed to at end as null char, advance pointer to after delim
    *end = '\0';
    string = end + strlen(delimiter);
    return temp;
}


typedef struct _fromsymbol
{
    t_object   x_ob;
    t_symbol  *x_separator;
} t_fromsymbol;

static t_class *fromsymbol_class;

static void fromsymbol_bang(t_fromsymbol *x)
{
    outlet_bang(((t_object *)x)->ob_outlet);  /* CHECKED */
}

static void fromsymbol_float(t_fromsymbol *x, t_float f)
{
outlet_float(((t_object *)x)->ob_outlet, f);
}

static void fromsymbol_separator(t_fromsymbol *x, t_symbol *s)
{
    //changing it up so we default to " " 
    if(s){
        x->x_separator = s;
    }
    else{
        x->x_separator = gensym("\s");
    };
}

static void fromsymbol_symbol(t_fromsymbol *x, t_symbol *s){
    //new and redone - Derek Kwan
    long unsigned int seplen = strlen(x->x_separator->s_name);
    seplen++;
    char sep[seplen];
    memset(sep, '\0', sizeof(seplen));
    strcpy(sep, x->x_separator->s_name);
    //char * sep = x->x_separator->s_name;
    if(s){
        //get length of input string
        long unsigned int iptlen = strlen(s->s_name);
        //allocate t_atom [] on stack length of string
        //(hacky way of making sure there's enough space)
        t_atom out[iptlen];
        iptlen++;
        char newstr[iptlen];
        memset(newstr, '\0', sizeof(newstr));
        strcpy(newstr,s->s_name);
        int atompos = 0; //position in atom
        //parsing by token
        char * ret = mtok(newstr, sep);
        while(ret != NULL){
            if(strlen(ret) > 0){
                char * err; //error pointer
                double f = strtod(ret, &err);
                if(*err == 0){
                    //if errpointer is at beginning, that means we've got a float
                    SETFLOAT(&out[atompos], (t_float)f);
                }
                else{
                    //else we're dealing with a symbol
                    t_symbol * cursym = gensym(ret);
                    SETSYMBOL(&out[atompos], cursym);
                };
                ret = mtok(NULL,sep);
                atompos++; //increment position in atom
            };
        };

	outlet_list(((t_object *)x)->ob_outlet, &s_list, atompos, out);
    };
}

/* old fromsymbol_symbol
static void fromsymbol_symbol(t_fromsymbol *x, t_symbol *s)
{
    static char zero = 0;
    char *sname = &zero;
    if (s)
    {
	sname = s->s_name;
	while (*sname == ' ' || *sname == '\t'
	       || *sname == '\n' || *sname == '\r') sname++;
    }
    if (*sname)
    {
	t_binbuf *bb = binbuf_new();
	int ac;
	t_atom *av;
	binbuf_text(bb, sname, strlen(sname));
	ac = binbuf_getnatom(bb);
	av = binbuf_getvec(bb);
	if (ac)
	{
	    if (av->a_type == A_SYMBOL)
		outlet_anything(((t_object *)x)->ob_outlet,
				av->a_w.w_symbol, ac - 1, av + 1);
	    else if (av->a_type == A_FLOAT)
	    {
		if (ac > 1)
		    outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
		else
		    outlet_float(((t_object *)x)->ob_outlet, av->a_w.w_float);
	    }
	}
	binbuf_free(bb);
    }
}
*/

static void fromsymbol_list(t_fromsymbol *x, t_symbol *s, int argc, t_atom *argv)
{
    if(argv->a_type == A_FLOAT){
        t_float f = atom_getfloatarg(0, argc, argv);
        outlet_float(((t_object *)x)->ob_outlet, f);
    }
}


static void fromsymbol_anything(t_fromsymbol *x, t_symbol *s, int ac, t_atom *av)
{
    fromsymbol_symbol(x, s);  /* CHECKED */
}

static void *fromsymbol_new(t_symbol * s, int argc, t_atom * argv){
    t_fromsymbol *x = (t_fromsymbol *)pd_new(fromsymbol_class);

    t_symbol * sep; //separator 
    int sepset = 0; //if the separator is passed via attribute
    while(argc){
        if(argv->a_type == A_SYMBOL){
            if(argc >= 1){
                t_symbol * cursym = atom_getsymbolarg(0, argc, argv);
                if(strcmp(cursym->s_name, "@separator") == 0){
                    if(argc >= 2){
                        //if something is passed
                        sep = atom_getsymbolarg(1, argc, argv);
                        sepset = 1;
                        //increment/decrement
                        argc-=2;
                        argv+=2;
                    }
                    else{
                        //do nothing
                        argc--;
                        argv++;
                    };
                }
                else{
                    goto errstate;
                };
            }
            else{
                goto errstate;
            };
        }
        else{
            goto errstate;
        };

    };
    if(sepset){
        fromsymbol_separator(x, sep);
    }
    else{
        x->x_separator = gensym(" ");  /* default: a space */
    };

    outlet_new((t_object *)x, &s_anything);
    return (x);
	errstate:
		pd_error(x, "fromsymbol: improper args");
		return NULL;
}

void fromsymbol_setup(void)
{
    fromsymbol_class = class_new(gensym("fromsymbol"),
				 (t_newmethod)fromsymbol_new, 0,
				 sizeof(t_fromsymbol), 0, A_GIMME, 0);
    class_addbang(fromsymbol_class, fromsymbol_bang);
    class_addfloat(fromsymbol_class, fromsymbol_float);
    class_addsymbol(fromsymbol_class, fromsymbol_symbol);
    class_addlist(fromsymbol_class, fromsymbol_list);
    class_addanything(fromsymbol_class, fromsymbol_anything);
    class_addmethod(fromsymbol_class, (t_method)fromsymbol_separator, gensym("separator"), A_DEFSYM, 0);
}
