/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

// Matt Barber added wizadry that makes grab talk to built-in receive names (2021)

#include "m_pd.h"
#include <common/api.h>
#include "m_imp.h"
#include "common/magicbit.h"

// LATER handle canvas grabbing (bypass)
// LATER check self-grabbing

// It would be nice to have write access to o_connections field...

struct _outlet{
    t_object        *o_owner;
    struct _outlet  *o_next;
    t_outconnect    *o_connections;
    t_symbol        *o_sym;
};

// ...and to have bindlist traversal routines in Pd API.

static t_class *bindlist_class = 0; // global variable

typedef struct _bindelem{
    t_pd                *e_who;
    struct _bindelem    *e_next;
}t_bindelem;

typedef struct _bindlist{
    t_pd b_pd;
    t_bindelem          *b_list;
}t_bindlist;

typedef struct _grab{
    t_object        x_ob;
    t_symbol       *x_target;
    int             x_noutlets;   // not counting right one
    t_object	   *x_receiver;   // object containing the receiver
    int				x_ncons;	  // number of connections from receiver
    int				x_maxobs;	  // maximum number of connections from receiver
    t_object      **x_grabbed;    // array of grabbed objects
    t_outconnect  **x_grabcons;   // array of grabbed connections
    int            *x_ngrabout;   // array; number of grabbed object's outlets
    t_outlet       *x_rightout;   // right outlet
// traversal helpers:
    t_outconnect   *x_tograbbed;  // a connection to grabbed object
    t_bindelem     *x_bindelem;
    int             x_nonreceive; //* flag to signal whether processed non-[receive] object
}t_grab;

static t_class *grab_class;

static int grab_prep(t_grab *x, t_object *ob){
	t_outlet *op;
	t_outconnect *ocp;
	t_object *dummy;
	int ncons, inno;
	if(x->x_target){
		x->x_receiver = ob;
		op = ob->ob_outlet;
	}
	else
        op = x->x_rightout;
	if(x->x_receiver && (x->x_receiver->te_g.g_pd->c_name != gensym("receive")))
		ncons = 1;
	else {
        if(!(x->x_tograbbed = ocp = magic_outlet_connections(op)))
            return(0);
        for(ncons = 0; ocp ; ++ncons)
            ocp = magic_outlet_nextconnection(ocp, &dummy, &inno);
	}
	x->x_ncons = ncons;
	if (!x->x_grabbed){
		if (!((x->x_grabbed = getbytes(ncons * sizeof(*x->x_grabbed))) && 
			 (x->x_ngrabout = getbytes(ncons * sizeof(*x->x_ngrabout))) &&
			(x->x_grabcons = getbytes(ncons * x->x_noutlets * sizeof(*x->x_grabcons)))))
		{
			pd_error(x, "grab: error allocating memory");
			return(0);
		}
		x->x_maxobs = ncons;
	}
	else if(ncons > x->x_maxobs){
		if (!((x->x_grabbed = resizebytes(x->x_grabbed,
			x->x_maxobs * sizeof(*x->x_grabbed),
			ncons * sizeof(*x->x_grabbed))) &&
			
			(x->x_ngrabout = resizebytes(x->x_ngrabout,
			x->x_maxobs * sizeof(*x->x_ngrabout),
			ncons * sizeof(*x->x_ngrabout))) &&
			
			(x->x_grabcons = resizebytes(x->x_grabcons,
			x->x_maxobs * x->x_noutlets * sizeof(*x->x_grabcons),
			ncons * x->x_noutlets * sizeof(*x->x_grabcons)))))
		{
			pd_error(x, "grab: error allocating memory");
			return(0);
		}
		x->x_maxobs = ncons;
	}
	return(1);
}

static void grab_start(t_grab *x){
    x->x_tograbbed = 0;
    x->x_bindelem = 0;
    x->x_receiver = 0;
    x->x_nonreceive = 0;
    if(x->x_target){
        t_pd *proxy = x->x_target->s_thing;
        t_object *ob;
        if(proxy && bindlist_class){
            if (*proxy == bindlist_class){
                x->x_bindelem = ((t_bindlist *)proxy)->b_list;
                while (x->x_bindelem){
                    if((ob = pd_checkobject(x->x_bindelem->e_who)))
                        if(grab_prep(x, ob))
                        	return;
                    x->x_bindelem = x->x_bindelem->e_next;
                }
            }
            else if((ob = pd_checkobject(proxy)))
                grab_prep(x,ob);
        }
    }
    else
        grab_prep(x,&x->x_ob);
}

static int grab_next(t_grab *x){
	if (!(x->x_grabbed && x->x_grabcons && x->x_ngrabout))
		return(0);
	t_object **grabbedp = x->x_grabbed;
	t_outconnect **grabconsp = x->x_grabcons;
	int *ngraboutp = x->x_ngrabout;
	t_object *gr;
	int nobs;
	int inno;
	t_outlet *op;
	t_outlet *goutp;
nextremote:
	// post("%s", x->x_receiver->te_g.g_pd->c_name->s_name);
	if(x->x_receiver && !(x->x_nonreceive) && (x->x_receiver->te_g.g_pd->c_name != gensym("receive"))){
		// post("nonreceive");
		*grabbedp = x->x_receiver;
		*ngraboutp = 1;
		*grabconsp = obj_starttraverseoutlet(x->x_receiver, &goutp, 0);
		goutp->o_connections = obj_starttraverseoutlet((t_object *)x, &op, 0);
		x->x_nonreceive = 1;
		return(1);
	}
    else if (x->x_tograbbed) {
		while(x->x_tograbbed){
			//post("entering grab_next while loop");
			x->x_tograbbed = magic_outlet_nextconnection(x->x_tograbbed, &gr, &inno);
			if(gr){
				if(inno){
					if(x->x_rightout)
						pd_error(x, "grab: right outlet must feed leftmost inlet");
					else
						pd_error(x, "grab: remote proxy must feed leftmost inlet");
				}
				else{
					*grabbedp++ = gr;
					int goutno = obj_noutlets(gr);
					if (goutno > x->x_noutlets) goutno = x->x_noutlets;
					// post ("grab_next goutno: %d", goutno);
					*ngraboutp++ = goutno;
					for(int i = 0; i < x->x_noutlets; i++){
						if(i < goutno){
							*grabconsp++ = obj_starttraverseoutlet(gr, &goutp, i);
							goutp->o_connections = obj_starttraverseoutlet((t_object *)x, &op, i);
						}
					}
				}
			}
		}
		return(grabbedp-x->x_grabbed); // return number of objects stored
    }
    if(x->x_bindelem) while ((x->x_bindelem = x->x_bindelem->e_next)){
        t_object *ob;
        if((ob = pd_checkobject(x->x_bindelem->e_who))){
        	x->x_nonreceive = 0;
            grab_prep(x,ob);
            grabbedp = x->x_grabbed;
            grabconsp = x->x_grabcons;
            ngraboutp = x->x_ngrabout;
            goto
                nextremote;
        }
    }
    return(0);
}

static void grab_restore(t_grab *x, int nobs){
	t_object **grabbedp = x->x_grabbed;
	t_object **grabconsp = x->x_grabcons;
	int *ngraboutp = x->x_ngrabout;
	int goutno;
	t_object *gr;
	t_outlet *goutp;
	while(nobs--){
		gr = *grabbedp++;
		goutno = *ngraboutp++;
		for(int i = 0; i < goutno ; i++){
			obj_starttraverseoutlet(gr, &goutp, i);
			goutp->o_connections = *grabconsp++;
		}
	}
}

static void grab_bang(t_grab *x){
	int nobs;
    grab_start(x);
    while(nobs = grab_next(x)){
        if(x->x_receiver)
        	pd_bang(x->x_receiver);
        else
        	outlet_bang(x->x_rightout);
        grab_restore(x,nobs);
    }
}

static void grab_float(t_grab *x, t_float f){
	int nobs;
    grab_start(x);
    while(nobs = grab_next(x)){
    	if(x->x_receiver)
        	pd_float(x->x_receiver, f);
        else
        	outlet_float(x->x_rightout, f);
        grab_restore(x,nobs);
    }
}

static void grab_symbol(t_grab *x, t_symbol *s){
    int nobs;
    grab_start(x);
    while(nobs = grab_next(x)){
    	if(x->x_receiver)
        	pd_symbol(x->x_receiver, s);
        else
        	outlet_symbol(x->x_rightout, s);
        grab_restore(x,nobs);
    }
}

static void grab_pointer(t_grab *x, t_gpointer *gp){
    int nobs;
    grab_start(x);
    while(nobs = grab_next(x)){
    	if(x->x_receiver)
        	pd_pointer(x->x_receiver, gp);
        else
        	outlet_pointer(x->x_rightout, gp);
        grab_restore(x,nobs);
    }
}

static void grab_list(t_grab *x, t_symbol *s, int ac, t_atom *av){
    int nobs;
    grab_start(x);
    while(nobs = grab_next(x)){
    	if(x->x_receiver)
        	pd_list(x->x_receiver, s, ac, av);
        else
        	outlet_list(x->x_rightout, s, ac, av);
       grab_restore(x,nobs);
    }
}

static void grab_anything(t_grab *x, t_symbol *s, int ac, t_atom *av){
    int nobs;
    grab_start(x);
    while(nobs = grab_next(x)){
    	if(x->x_receiver)
        	pd_anything(x->x_receiver, s, ac, av);
        else
        	outlet_anything(x->x_rightout, s, ac, av);
       grab_restore(x,nobs);
    }
}

static void grab_set(t_grab *x, t_symbol *s){
    if(x->x_target && s && s != &s_)
        x->x_target = s;
}

static void *grab_new(t_floatarg f, t_symbol *s){
    t_grab *x = (t_grab *)pd_new(grab_class);
    t_outconnect **grabcons;
    int noutlets = f < 1 ? 1 : (int)f;
    x->x_noutlets = noutlets;
    x->x_maxobs = 0;
    while(noutlets--)
        outlet_new((t_object *)x, &s_anything);
    if(s && s != &s_){
        x->x_target = s;
        x->x_rightout = 0;
    }
    else{
        x->x_target = 0;
        x->x_rightout = outlet_new((t_object *)x, &s_anything);
    }
    return(x);
}

static void grab_free(t_grab *x){
    if(x->x_grabbed)
        freebytes(x->x_grabbed, x->x_maxobs * sizeof(*x->x_grabbed));
    if(x->x_ngrabout)
    	freebytes(x->x_ngrabout, x->x_maxobs * sizeof(*x->x_ngrabout));
    if(x->x_grabcons)
    	freebytes(x->x_grabcons, x->x_maxobs * x->x_noutlets * sizeof(*x->x_grabcons));
}

CYCLONE_OBJ_API void grab_setup(void){
    t_symbol *s = gensym("grab");
    grab_class = class_new(s, (t_newmethod)grab_new, (t_method)grab_free,
        sizeof(t_grab), 0, A_DEFFLOAT, A_DEFSYMBOL, 0);
    class_addfloat(grab_class, grab_float);
    class_addbang(grab_class, grab_bang);
    class_addsymbol(grab_class, grab_symbol);
    class_addpointer(grab_class, grab_pointer);
    class_addlist(grab_class, grab_list);
    class_addanything(grab_class, grab_anything);
    class_addmethod(grab_class, (t_method)grab_set, gensym("set"), A_SYMBOL, 0);
    if(!bindlist_class){
        t_class *c = grab_class;
        pd_bind(&grab_class, s);
        pd_bind(&c, s);
        if (!s->s_thing || !(bindlist_class = *s->s_thing)
	    || bindlist_class->c_name != gensym("bindlist"))
            error("grab: failure to initialize remote grabbing feature");
	pd_unbind(&c, s);
	pd_unbind(&grab_class, s);
    }
}
