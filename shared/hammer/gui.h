/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __HAMMERGUI_H__
#define __HAMMERGUI_H__

typedef struct _hammergui
{
    t_pd       g_pd;
    t_symbol  *g_gui;
    t_symbol  *g_mouse;
    t_symbol  *g_poll;
    t_symbol  *g_focus;
    t_symbol  *g_vised;
    int        g_up;
} t_hammergui;

void hammergui_bindmouse(t_pd *master);
void hammergui_unbindmouse(t_pd *master);
void hammergui_mousexy(t_symbol *s);
void hammergui_willpoll(void);
void hammergui_startpolling(t_pd *master);
void hammergui_stoppolling(t_pd *master);
void hammergui_bindfocus(t_pd *master);
void hammergui_unbindfocus(t_pd *master);
void hammergui_bindvised(t_pd *master);
void hammergui_unbindvised(t_pd *master);

#endif
