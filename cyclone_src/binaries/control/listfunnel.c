//taken from maxlib

/* ------------------------- listfunnel   ------------------------------------- */
/*                                                                              */
/* Convert list into two-element lists with source index.                       */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

//listfunnel v0.1, written by Olaf Matthes <olaf.matthes@gmx.de> (maxlib)
// Derek Kwan 2017 - ported from maxlib and added offset  

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>

 
static t_class *listfunnel_class;

typedef struct listfunnel
{
  t_object  x_ob;
  int       x_offset;
  t_outlet  *x_outlet;              
} t_listfunnel;

static void listfunnel_list(t_listfunnel *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
        int offset = x->x_offset;
	t_atom list[2];

	for(i = 0; i < argc; i++)
	{
		SETFLOAT(list, (i+offset));
		list[1] = argv[i]; // SETFLOAT(list+1, atom_getfloatarg(i, argc, argv));
		outlet_list(x->x_outlet, NULL, 2, list);
	}
}

static void listfunnel_float(t_listfunnel *x, t_floatarg f)
{
	t_atom list[2];

	SETFLOAT(list, x->x_offset);
	SETFLOAT(list+1, f);
	outlet_list(x->x_outlet, NULL, 2, list);
}

static void listfunnel_offset(t_listfunnel *x, t_float f)
{
    x->x_offset = (int)f;
}

static void *listfunnel_new(t_floatarg f)
{
    
    t_listfunnel *x = (t_listfunnel *)pd_new(listfunnel_class);
    listfunnel_offset(x, f);
    x->x_outlet = outlet_new(&x->x_ob, &s_list);

    return (void *)x;
}

void listfunnel_setup(void)
{
    listfunnel_class = class_new(gensym("listfunnel"), (t_newmethod)listfunnel_new,
    	0, sizeof(t_listfunnel), 0, A_DEFFLOAT, 0);
    class_addmethod(listfunnel_class, (t_method)listfunnel_offset, gensym("offset"), A_FLOAT, 0);
    class_addfloat(listfunnel_class, listfunnel_float);
    class_addlist(listfunnel_class, listfunnel_list);
    
}

