/* Based on my_numbox.c written by Thomas Musil (c) IEM KUG Graz Austria (2000-2001)
   number~.c written by Timothy Schoen for the cyclone library (2022)
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "g_all_guis.h"

#include <math.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define MINDIGITS 1
#define MINFONT   4

typedef struct _number_tilde
{
    t_iemgui x_gui;
    t_outlet *x_signal_outlet;
    t_outlet *x_float_outlet;
    t_clock  *x_clock_reset;
    t_clock  *x_clock_wait;
    t_clock  *x_clock_repaint;
    t_float x_val;
    t_float x_ramp_val;
    t_float x_ramp_time;
    t_float x_ramp;
    t_float x_last_out;
    t_float  x_interval_ms;
    int      x_numwidth;
    int      x_mode;
    int      x_needs_repaint;
    char     x_buf[IEMGUI_MAX_NUM_LEN];
} t_number_tilde;


/*------------------ global functions -------------------------*/

static void number_tilde_key(void *z, t_symbol *keysym, t_floatarg fkey);
static void number_tilde_draw_update(t_gobj *client, t_glist *glist);
static void init_number_tilde_dialog(void);
static void number_tilde_interval(t_number_tilde *x, t_floatarg f);
static void number_tilde_mode(t_number_tilde *x, t_floatarg f);
static void number_tilde_bang(t_number_tilde *x);
static void number_tilde_set(t_number_tilde *x, t_floatarg f);

/* ------------ nbx gui-my number box ----------------------- */

t_widgetbehavior number_tilde_widgetbehavior;
static t_class *number_tilde_class;

/* widget helper functions */

static void number_tilde_tick_reset(t_number_tilde *x)
{
    if(x->x_gui.x_fsf.x_change && x->x_gui.x_glist)
    {
        x->x_gui.x_fsf.x_change = 0;
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
    }
}

static void number_tilde_tick_wait(t_number_tilde *x)
{
    sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
}

static void number_tilde_periodic_repaint(t_number_tilde *x)
{
    // Make sure it's not constantly repainting when there is no audio coming in
    if(x->x_needs_repaint) {
        
        number_tilde_bang(x);
    
        // Clear repaint flag
        x->x_needs_repaint = 0;
    }
    
    // Cap the repaint speed at 60fps
    clock_delay(x->x_clock_repaint, fmax(x->x_interval_ms, 15));
}

static void number_tilde_calc_fontwidth(t_number_tilde *x)
{
    int w, f = 31;

    if(x->x_gui.x_fsf.x_font_style == 1)
        f = 27;
    else if(x->x_gui.x_fsf.x_font_style == 2)
        f = 25;

    w = 10 * f * x->x_numwidth;
    w /= 36;
    x->x_gui.x_w = (w + (x->x_gui.x_h/2)/IEMGUI_ZOOM(x) + 4) * IEMGUI_ZOOM(x);
}

static void number_tilde_ftoa(t_number_tilde *x)
{
    double f = x->x_val;
    int bufsize, is_exp = 0, i, idecimal;

    sprintf(x->x_buf, "%g", f);
    bufsize = (int)strlen(x->x_buf);
    
    int real_numwidth = x->x_numwidth + 1;
    if(bufsize > real_numwidth)/* if to reduce */
    {
        if(is_exp)
        {
            if(real_numwidth <= 5)
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            i = bufsize - 4;
            for(idecimal = 0; idecimal < i; idecimal++)
                if(x->x_buf[idecimal] == '.')
                    break;
            if(idecimal > (real_numwidth - 4))
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            else
            {
                int new_exp_index = real_numwidth - 4;
                int old_exp_index = bufsize - 4;

                for(i = 0; i < 4; i++, new_exp_index++, old_exp_index++)
                    x->x_buf[new_exp_index] = x->x_buf[old_exp_index];
                x->x_buf[x->x_numwidth] = 0;
            }
        }
        else
        {
            for(idecimal = 0; idecimal < bufsize; idecimal++)
                if(x->x_buf[idecimal] == '.')
                    break;
            if(idecimal > real_numwidth)
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            else
                x->x_buf[real_numwidth] = 0;
        }
    }
}

static void number_tilde_draw_update(t_gobj *client, t_glist *glist)
{
    t_number_tilde *x = (t_number_tilde *)client;
    if(glist_isvisible(glist))
    {
        if(x->x_gui.x_fsf.x_change && x->x_buf[0] && x->x_mode)
        {
            char *cp = x->x_buf;
            int sl = (int)strlen(x->x_buf);
            
            x->x_buf[sl] = '>';
            x->x_buf[sl+1] = 0;
            
            if(sl >= x->x_numwidth)
                cp += sl - x->x_numwidth + 1;
            sys_vgui(".x%lx.c itemconfigure %lxNUMBER -fill #%06x -text {%s} \n",
                     glist_getcanvas(glist), x,
                     x->x_gui.x_fcol, cp);
            
            sys_vgui(".x%lx.c itemconfigure %lxBASE1 -width %d\n",
                     glist_getcanvas(glist), x,
                     IEMGUI_ZOOM(x) * 2);
            
            x->x_buf[sl] = 0;
        }
        else {
            number_tilde_ftoa(x);
            
            sys_vgui(".x%lx.c itemconfigure %lxNUMBER -fill #%06x -text {%s} \n",
                     glist_getcanvas(glist), x,
                     x->x_gui.x_fsf.x_selected ? IEM_GUI_COLOR_SELECTED : x->x_gui.x_fcol, x->x_buf);
            
            sys_vgui(".x%lx.c itemconfigure %lxBASE1 -width %d\n",
                     glist_getcanvas(glist), x,
                     IEMGUI_ZOOM(x));
            
            x->x_buf[0] = 0;
        }
    }
}

static void number_tilde_draw_new(t_number_tilde *x, t_glist *glist)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int w = x->x_gui.x_w, half = x->x_gui.x_h/2;
    int d = IEMGUI_ZOOM(x) + x->x_gui.x_h/(34*IEMGUI_ZOOM(x));
    int corner = x->x_gui.x_h/4;
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = 3 * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d %d %d "
             "-width %d -outline #%06x -fill #%06x -tags %lxBASE1\n",
             canvas,
             xpos, ypos,
             xpos + w - corner, ypos,
             xpos + w, ypos + corner,
             xpos + w, ypos + x->x_gui.x_h,
             xpos, ypos + x->x_gui.x_h,
             xpos, ypos,
             IEMGUI_ZOOM(x), IEM_GUI_COLOR_NORMAL, x->x_gui.x_bcol, x);

    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w -font {{%s} -%d %s} -fill #%06x -tags %lxBASE2\n",
             canvas, xpos + 2*IEMGUI_ZOOM(x) + 1, ypos + half + d,
             x->x_mode ? ">" : "~", x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x),
             sys_fontweight, x->x_gui.x_fcol, x);

    if(!x->x_gui.x_fsf.x_snd_able) {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxOUT%d outlet]\n",
                 canvas,
                 xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
                 xpos + iow, ypos + x->x_gui.x_h,
                 x, 0);
        
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxOUT2%d outlet]\n",
             canvas,
             xpos + x->x_gui.x_w - iow, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + x->x_gui.x_w, ypos + x->x_gui.x_h,
             x, 0);
    }
    else {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxOUT%d outlet]\n",
                 canvas,
                 xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
                 xpos + iow, ypos + x->x_gui.x_h,
                 x, 0);
    }
    
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxIN%d inlet]\n",
             canvas,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh,
             x, 0);
    
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxIN2%d inlet]\n",
             canvas,
             xpos + x->x_gui.x_w - iow, ypos,
             xpos + x->x_gui.x_w, ypos - IEMGUI_ZOOM(x) + ioh,
             x, 0);

    
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w -font {{%s} -%d %s} -fill #%06x -tags [list %lxLABEL label text]\n",
             canvas, xpos + x->x_gui.x_ldx * IEMGUI_ZOOM(x),
             ypos + x->x_gui.x_ldy * IEMGUI_ZOOM(x),
             (strcmp(x->x_gui.x_lab->s_name, "empty") ? x->x_gui.x_lab->s_name : ""),
             x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x), sys_fontweight,
             x->x_gui.x_lcol, x);
    number_tilde_ftoa(x);
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w -font {{%s} -%d %s} -fill #%06x -tags %lxNUMBER\n",
             canvas, xpos + half + 2*IEMGUI_ZOOM(x) + 3, ypos + half + d,
             x->x_buf, x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x),
             sys_fontweight, x->x_gui.x_fcol, x);
}

static void number_tilde_draw_move(t_number_tilde *x, t_glist *glist)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int w = x->x_gui.x_w, half = x->x_gui.x_h/2;
    int d = IEMGUI_ZOOM(x) + x->x_gui.x_h / (34 * IEMGUI_ZOOM(x));
    int corner = x->x_gui.x_h/4;
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = 3 * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c coords %lxBASE1 %d %d %d %d %d %d %d %d %d %d %d %d\n",
             canvas, x,
             xpos, ypos,
             xpos + w - corner, ypos,
             xpos + w, ypos + corner,
             xpos + w, ypos + x->x_gui.x_h,
             xpos, ypos + x->x_gui.x_h,
             xpos, ypos);

    
    sys_vgui(".x%lx.c coords %lxBASE2 %d %d\n",
             canvas, x,
             xpos + 2*IEMGUI_ZOOM(x) + 1, ypos + half + d);

    if(!x->x_gui.x_fsf.x_snd_able) {
        sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
                 canvas, x, 0,
                 xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
                 xpos + iow, ypos + x->x_gui.x_h);
        
        sys_vgui(".x%lx.c coords %lxOUT2%d %d %d %d %d\n",
                 canvas, x, 0,
                 xpos + x->x_gui.x_w - iow, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
                 xpos + x->x_gui.x_w, ypos + x->x_gui.x_h);
    }
    else {
        sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
                 canvas, x, 0,
                 xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
                 xpos + iow, ypos + x->x_gui.x_h);
    }

    sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh);
    
    sys_vgui(".x%lx.c coords %lxIN2%d %d %d %d %d\n",
             canvas, x, 0,
             xpos + x->x_gui.x_w - iow, ypos,
             xpos + x->x_gui.x_w, ypos - IEMGUI_ZOOM(x) + ioh);
    
    sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x,
             xpos + x->x_gui.x_ldx * IEMGUI_ZOOM(x),
             ypos + x->x_gui.x_ldy * IEMGUI_ZOOM(x));
    sys_vgui(".x%lx.c coords %lxNUMBER %d %d\n",
             canvas, x, xpos + half + 2*IEMGUI_ZOOM(x) + 3, ypos + half + d);
}

static void number_tilde_draw_erase(t_number_tilde* x, t_glist* glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c delete %lxBASE1\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxBASE2\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxLABEL\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxNUMBER\n", canvas, x);
    if(!x->x_gui.x_fsf.x_snd_able) {
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
        sys_vgui(".x%lx.c delete %lxOUT2%d\n", canvas, x, 0);
    }
    else {
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    }
    
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
    sys_vgui(".x%lx.c delete %lxIN2%d\n", canvas, x, 0);
}

static void number_tilde_draw_config(t_number_tilde* x, t_glist* glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c itemconfigure %lxLABEL -font {{%s} -%d %s} -fill #%06x -text {%s} \n",
             canvas, x, x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x), sys_fontweight,
             (x->x_gui.x_fsf.x_selected ? IEM_GUI_COLOR_SELECTED : x->x_gui.x_lcol),
             strcmp(x->x_gui.x_lab->s_name, "empty") ? x->x_gui.x_lab->s_name:"");
    sys_vgui(".x%lx.c itemconfigure %lxNUMBER -font {{%s} -%d %s} -fill #%06x \n",
             canvas, x, x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x), sys_fontweight,
             (x->x_gui.x_fsf.x_selected ? IEM_GUI_COLOR_SELECTED : x->x_gui.x_fcol));
    sys_vgui(".x%lx.c itemconfigure %lxBASE1 -fill #%06x\n", canvas,
             x, x->x_gui.x_bcol);
    sys_vgui(".x%lx.c itemconfigure %lxBASE2 -fill #%06x\n", canvas, x,
             (x->x_gui.x_fsf.x_selected ? IEM_GUI_COLOR_SELECTED : x->x_gui.x_fcol));
}

static void number_tilde_draw_io(t_number_tilde* x,t_glist* glist, int old_snd_rcv_flags)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = 3 * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);

    if((old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && !x->x_gui.x_fsf.x_snd_able) {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags %lxOUT2%d\n",
             canvas,
             xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + iow, ypos + x->x_gui.x_h,
             x, 0);
        
        /* keep these above outlet */
        sys_vgui(".x%lx.c raise %lxLABEL %lxOUT%d\n", canvas, x, x, 0);
        sys_vgui(".x%lx.c raise %lxNUMBER %lxLABEL\n", canvas, x, x);
    }
    if(!(old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c delete %lxOUT2%d\n", canvas, x, 0);
}

static void number_tilde_draw_select(t_number_tilde *x, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    if(x->x_gui.x_fsf.x_selected)
    {
        if(x->x_gui.x_fsf.x_change)
        {
            x->x_gui.x_fsf.x_change = 0;
            clock_unset(x->x_clock_reset);
            x->x_buf[0] = 0;
            sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
        }
        sys_vgui(".x%lx.c itemconfigure %lxBASE1 -outline #%06x\n",
            canvas, x, IEM_GUI_COLOR_SELECTED);
        sys_vgui(".x%lx.c itemconfigure %lxBASE2 -fill #%06x\n",
            canvas, x, IEM_GUI_COLOR_SELECTED);
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%06x\n",
            canvas, x, IEM_GUI_COLOR_SELECTED);
        sys_vgui(".x%lx.c itemconfigure %lxNUMBER -fill #%06x\n",
            canvas, x, IEM_GUI_COLOR_SELECTED);
    }
    else
    {
        sys_vgui(".x%lx.c itemconfigure %lxBASE1 -outline #%06x\n",
            canvas, x, IEM_GUI_COLOR_NORMAL);
        sys_vgui(".x%lx.c itemconfigure %lxBASE2 -fill #%06x\n",
            canvas, x, x->x_gui.x_fcol);
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%06x\n",
            canvas, x, x->x_gui.x_lcol);
        sys_vgui(".x%lx.c itemconfigure %lxNUMBER -fill #%06x\n",
            canvas, x, x->x_gui.x_fcol);
    }
}

static void number_tilde_draw(t_number_tilde *x, t_glist *glist, int mode)
{
    if(mode == IEM_GUI_DRAW_MODE_UPDATE)
        sys_queuegui(x, glist, number_tilde_draw_update);
    else if(mode == IEM_GUI_DRAW_MODE_MOVE)
        number_tilde_draw_move(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_NEW)
        number_tilde_draw_new(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_SELECT)
        number_tilde_draw_select(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_ERASE)
        number_tilde_draw_erase(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_CONFIG)
        number_tilde_draw_config(x, glist);
    else if(mode >= IEM_GUI_DRAW_MODE_IO)
        number_tilde_draw_io(x, glist, mode - IEM_GUI_DRAW_MODE_IO);
}

/* ------------------------ nbx widgetbehaviour----------------------------- */

static void number_tilde_getrect(t_gobj *z, t_glist *glist,
                              int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_number_tilde* x = (t_number_tilde*)z;

    *xp1 = text_xpix(&x->x_gui.x_obj, glist);
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w;
    *yp2 = *yp1 + x->x_gui.x_h;
}

static void number_tilde_save(t_gobj *z, t_binbuf *b)
{
    t_number_tilde *x = (t_number_tilde *)z;
    t_symbol *bflcol[3];
    t_symbol *srl[3];

    iemgui_save(&x->x_gui, srl, bflcol);
    if(x->x_gui.x_fsf.x_change)
    {
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
    }
    binbuf_addv(b, "ssiisiififisssiiiisssf", gensym("#X"), gensym("obj"),
                (int)x->x_gui.x_obj.te_xpix, (int)x->x_gui.x_obj.te_ypix,
                gensym("number~"), x->x_numwidth, x->x_gui.x_h/IEMGUI_ZOOM(x),
                x->x_interval_ms, x->x_mode,
                x->x_ramp_time, iem_symargstoint(&x->x_gui.x_isa),
                srl[0], srl[1], srl[2],
                x->x_gui.x_ldx, x->x_gui.x_ldy,
                iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
                bflcol[0], bflcol[1], bflcol[2],
                x->x_gui.x_isa.x_loadinit?x->x_val:0.);
    
    binbuf_addv(b, ";");
}

static void number_tilde_properties(t_gobj *z, t_glist *owner)
{
    // For some reason the dialog doesn't work if we initialise it in the setup...
    init_number_tilde_dialog();
    
    t_number_tilde *x = (t_number_tilde *)z;
    char buf[800];
    t_symbol *srl[3];
    
    iemgui_properties(&x->x_gui, srl);
    if(x->x_gui.x_fsf.x_change)
    {
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
    }
    
    sprintf(buf, "::dialog_number::pdtk_number_tilde_dialog %%s \
            -------dimensions(digits)(pix):------- %d %d width: %d %d height: \
            -----------output-range:----------- %G min: %G max: %d \
            %d %d %.2f \
            %s %s \
            %s %d %d \
            %d %d \
            #%06x #%06x #%06x\n",
            x->x_numwidth, MINDIGITS, x->x_gui.x_h/IEMGUI_ZOOM(x), IEM_GUI_MINSIZE,
            0.0, 0.0 // min, max
            , 0,/*no_schedule*/
            x->x_gui.x_isa.x_loadinit, x->x_mode, x->x_interval_ms,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
            x->x_gui.x_fsf.x_font_style, x->x_gui.x_fontsize,
            0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol,
                0xffffff & x->x_gui.x_lcol);


    gfxstub_new(&x->x_gui.x_obj.ob_pd, x, buf);
}

static void number_tilde_bang(t_number_tilde *x)
{
    // Output float from second outlet
    outlet_float(x->x_float_outlet, x->x_val);
    
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, x->x_val);
    
    // Repaint the number
    // Don't queue this because that will lead to too much latency
    number_tilde_draw_update(&x->x_gui.x_obj.te_g, x->x_gui.x_glist);
}

static void number_tilde_dialog(t_number_tilde *x, t_symbol *s, int argc,
    t_atom *argv)
{
    t_symbol *srl[3];
    int w = (int)atom_getfloatarg(0, argc, argv);
    int h = (int)atom_getfloatarg(1, argc, argv);
    
    int interval = (int)atom_getfloatarg(6, argc, argv);
    int mode = (int)atom_getfloatarg(17, argc, argv);
    
    int sr_flags;
    t_atom undo[18];
    iemgui_setdialogatoms(&x->x_gui, 18, undo);
    SETFLOAT(undo+0, x->x_numwidth);

    pd_undo_set_objectstate(x->x_gui.x_glist, (t_pd*)x, gensym("dialog"),
                            18, undo,
                            argc, argv);

    sr_flags = iemgui_dialog(&x->x_gui, srl, argc, argv);
    if(w < MINDIGITS)
        w = MINDIGITS;
    x->x_numwidth = w;
    if(h < IEM_GUI_MINSIZE)
        h = IEM_GUI_MINSIZE;
    x->x_gui.x_h = h * IEMGUI_ZOOM(x);
    number_tilde_calc_fontwidth(x);
    
    number_tilde_mode(x, mode);
    number_tilde_interval(x, interval);
    
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_IO + sr_flags);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_CONFIG);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_MOVE);
    canvas_fixlinesfor(x->x_gui.x_glist, (t_text*)x);
}

static void number_tilde_motion(t_number_tilde *x, t_floatarg dx, t_floatarg dy,
    t_floatarg up)
{
    double k2 = 1.0;
    if (up != 0)
        return;

    if(x->x_gui.x_fsf.x_finemoved)
        k2 = 0.01;

    number_tilde_set(x, x->x_val - k2*dy);
    
    sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
    number_tilde_bang(x);
    clock_unset(x->x_clock_reset);
}

static void number_tilde_click(t_number_tilde *x, t_floatarg xpos, t_floatarg ypos,
                            t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{

    int relative_x = xpos - x->x_gui.x_obj.te_xpix;
    if(relative_x < 10) {
        number_tilde_mode(x, !x->x_mode);
    }
    else if (x->x_mode) {
        glist_grab(x->x_gui.x_glist, &x->x_gui.x_obj.te_g,
            (t_glistmotionfn)number_tilde_motion, number_tilde_key, xpos, ypos);
    }
}

static int number_tilde_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_number_tilde* x = (t_number_tilde *)z;

    if(doit)
    {
        number_tilde_click( x, (t_floatarg)xpix, (t_floatarg)ypix,
            (t_floatarg)shift, 0, (t_floatarg)alt);
        if(shift)
            x->x_gui.x_fsf.x_finemoved = 1;
        else
            x->x_gui.x_fsf.x_finemoved = 0;
        if(!x->x_gui.x_fsf.x_change && xpix - x->x_gui.x_obj.te_xpix > 10)
        {
            clock_delay(x->x_clock_wait, 50);
            x->x_gui.x_fsf.x_change = 1;
            clock_delay(x->x_clock_reset, 3000);

            x->x_buf[0] = 0;
        }
        else
        {
            x->x_gui.x_fsf.x_change = 0;
            clock_unset(x->x_clock_reset);
            x->x_buf[0] = 0;
            sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
        }
    }
    return (1);
}

static void number_tilde_set(t_number_tilde *x, t_floatarg f)
{
    t_float ftocompare = f;
        /* bitwise comparison, suggested by Dan Borstein - to make this work
        ftocompare must be t_float type like x_val. */
    if (memcmp(&ftocompare, &x->x_val, sizeof(ftocompare)))
    {
        
        x->x_val = ftocompare;
        x->x_ramp = (x->x_val - x->x_ramp_val) / (x->x_ramp_time * (sys_getsr() / 1000.0));
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
    }
    
    
}

static void number_tilde_float(t_number_tilde *x, t_floatarg f)
{
    number_tilde_set(x, f);
    number_tilde_bang(x);
}

static void number_tilde_size(t_number_tilde *x, t_symbol *s, int ac, t_atom *av)
{
    int h, w;

    w = (int)atom_getfloatarg(0, ac, av);
    if(w < MINDIGITS)
        w = MINDIGITS;
    x->x_numwidth = w;
    if(ac > 1)
    {
        h = (int)atom_getfloatarg(1, ac, av);
        if(h < IEM_GUI_MINSIZE)
            h = IEM_GUI_MINSIZE;
        x->x_gui.x_h = h * IEMGUI_ZOOM(x);
    }
    number_tilde_calc_fontwidth(x);
    iemgui_size((void *)x, &x->x_gui);
}

static void number_tilde_delta(t_number_tilde *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void number_tilde_pos(t_number_tilde *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}


static void number_tilde_color(t_number_tilde *x, t_symbol *s, int ac, t_atom *av)
{iemgui_color((void *)x, &x->x_gui, s, ac, av);}

static void number_tilde_send(t_number_tilde *x, t_symbol *s)
{iemgui_send(x, &x->x_gui, s);}

static void number_tilde_receive(t_number_tilde *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}

static void number_tilde_label(t_number_tilde *x, t_symbol *s)
{iemgui_label((void *)x, &x->x_gui, s);}

static void number_tilde_label_pos(t_number_tilde *x, t_symbol *s, int ac, t_atom *av)
{iemgui_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void number_tilde_label_font(t_number_tilde *x,
    t_symbol *s, int ac, t_atom *av)
{
    int f = (int)atom_getfloatarg(1, ac, av);

    if(f < 4)
        f = 4;
    x->x_gui.x_fontsize = f;
    f = (int)atom_getfloatarg(0, ac, av);
    if((f < 0) || (f > 2))
        f = 0;
    x->x_gui.x_fsf.x_font_style = f;
    number_tilde_calc_fontwidth(x);
    iemgui_label_font((void *)x, &x->x_gui, s, ac, av);
}

static void number_tilde_interval(t_number_tilde *x, t_floatarg f)
{
    x->x_interval_ms = f;
}

static void number_tilde_mode(t_number_tilde *x, t_floatarg f)
{
    x->x_mode = f;
    
    sys_vgui(".x%lx.c itemconfigure %lxBASE2 -text {%s} \n",
             glist_getcanvas(x->x_gui.x_glist), x,
             x->x_mode ? ">" : "~");
    
    x->x_gui.x_fsf.x_change = 0;
    
    sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
}


static void number_tilde_init(t_number_tilde *x, t_floatarg f)
{
    x->x_gui.x_isa.x_loadinit = (f == 0.0) ? 0 : 1;
}

static void number_tilde_loadbang(t_number_tilde *x, t_floatarg action)
{
    if(action == LB_LOAD && x->x_gui.x_isa.x_loadinit)
    {
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
        number_tilde_bang(x);
    }
}

static void number_tilde_key(void *z, t_symbol *keysym, t_floatarg fkey)
{
    t_number_tilde *x = z;
    char c = fkey;
    char buf[3];
    buf[1] = 0;

    if(c == 0)
    {
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
        return;
    }
    if(((c >= '0') && (c <= '9')) || (c == '.') || (c == '-') ||
        (c == 'e') || (c == '+') || (c == 'E'))
    {
        if(strlen(x->x_buf) < (IEMGUI_MAX_NUM_LEN-2))
        {
            buf[0] = c;
            strcat(x->x_buf, buf);
            sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
        }
    }
    else if((c == '\b') || (c == 127))
    {
        int sl = (int)strlen(x->x_buf) - 1;

        if(sl < 0)
            sl = 0;
        x->x_buf[sl] = 0;
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
    }
    else if((c == '\n') || (c == 13))
    {
        number_tilde_set(x, atof(x->x_buf));
        x->x_buf[0] = 0;
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);

        number_tilde_bang(x);
        sys_queuegui(x, x->x_gui.x_glist, number_tilde_draw_update);
    }
    clock_delay(x->x_clock_reset, 3000);
}

static void number_tilde_list(t_number_tilde *x, t_symbol *s, int ac, t_atom *av)
{
    if(!ac) {
        number_tilde_bang(x);
    }
    else if(IS_A_FLOAT(av, 0))
    {
        number_tilde_set(x, atom_getfloatarg(0, ac, av));
        number_tilde_bang(x);
    }
}

static void *number_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_number_tilde *x = (t_number_tilde *)pd_new(number_tilde_class);
    int w = 4, h = 15;
    int ldx = 0, ldy = -8;
    int fs = 10;
    t_float v = 0;
    int interval = 100;
    int mode = 0;
    t_float ramp_time = 0;
    
    floatinlet_new(&x->x_gui.x_obj, &x->x_ramp_time);
    
    x->x_signal_outlet = outlet_new(&x->x_gui.x_obj,  &s_signal);
    x->x_float_outlet = outlet_new(&x->x_gui.x_obj,  &s_float);
    
    x->x_gui.x_bcol = 0xFCFCFC;
    x->x_gui.x_fcol = 0x00;
    x->x_gui.x_lcol = 0x00;

    if((argc >= 17)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
       &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
       &&IS_A_FLOAT(argv,4)&&IS_A_FLOAT(argv,5)
       &&(IS_A_SYMBOL(argv,6)||IS_A_FLOAT(argv,6))
       &&(IS_A_SYMBOL(argv,7)||IS_A_FLOAT(argv,7))
       &&(IS_A_SYMBOL(argv,8)||IS_A_FLOAT(argv,8))
       &&IS_A_FLOAT(argv,9)&&IS_A_FLOAT(argv,10)
       &&IS_A_FLOAT(argv,11)&&IS_A_FLOAT(argv,12)&&IS_A_FLOAT(argv,16))
    {
        w = (int)atom_getfloatarg(0, argc, argv);
        h = (int)atom_getfloatarg(1, argc, argv);
        interval = atom_getfloatarg(2, argc, argv);
        mode = atom_getfloatarg(3, argc, argv);
        ramp_time = atom_getfloatarg(4, argc, argv);
        iem_inttosymargs(&x->x_gui.x_isa, atom_getfloatarg(5, argc, argv));
        iemgui_new_getnames(&x->x_gui, 6, argv);
        ldx = (int)atom_getfloatarg(9, argc, argv);
        ldy = (int)atom_getfloatarg(10, argc, argv);
        iem_inttofstyle(&x->x_gui.x_fsf, atom_getfloatarg(11, argc, argv));
        fs = (int)atom_getfloatarg(12, argc, argv);
        iemgui_all_loadcolors(&x->x_gui, argv+13, argv+14, argv+15);
        v = atom_getfloatarg(16, argc, argv);
    }
    else iemgui_new_getnames(&x->x_gui, 6, 0);
    x->x_gui.x_draw = (t_iemfunptr)number_tilde_draw;
    x->x_gui.x_fsf.x_snd_able = 1;
    x->x_gui.x_fsf.x_rcv_able = 1;
    x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
    if(x->x_gui.x_isa.x_loadinit)
        x->x_val = v;
    else
        x->x_val = 0.0;

    if(!strcmp(x->x_gui.x_snd->s_name, "empty"))
        x->x_gui.x_fsf.x_snd_able = 0;
    if(!strcmp(x->x_gui.x_rcv->s_name, "empty"))
        x->x_gui.x_fsf.x_rcv_able = 0;
    if(x->x_gui.x_fsf.x_font_style == 1) strcpy(x->x_gui.x_font, "helvetica");
    else if(x->x_gui.x_fsf.x_font_style == 2) strcpy(x->x_gui.x_font, "times");
    else { x->x_gui.x_fsf.x_font_style = 0;
        strcpy(x->x_gui.x_font, sys_font); }
    if(x->x_gui.x_fsf.x_rcv_able)
        pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
    x->x_gui.x_ldx = ldx;
    x->x_gui.x_ldy = ldy;
    if(fs < MINFONT)
        fs = MINFONT;
    x->x_gui.x_fontsize = fs;
    if(w < MINDIGITS)
        w = MINDIGITS;
    x->x_numwidth = w;
    if(h < IEM_GUI_MINSIZE)
        h = IEM_GUI_MINSIZE;
    x->x_gui.x_h = h;
    x->x_buf[0] = 0;
    iemgui_verify_snd_ne_rcv(&x->x_gui);
    x->x_clock_reset = clock_new(x, (t_method)number_tilde_tick_reset);
    x->x_clock_wait = clock_new(x, (t_method)number_tilde_tick_wait);
    x->x_clock_repaint = clock_new(x, (t_method)number_tilde_periodic_repaint);
    x->x_gui.x_fsf.x_change = 0;
    iemgui_newzoom(&x->x_gui);
    number_tilde_calc_fontwidth(x);
    
    x->x_interval_ms = interval;
    x->x_mode = mode;
    x->x_ramp_time = ramp_time;
    
    // Start repaint clock
    clock_delay(x->x_clock_repaint, x->x_interval_ms);
    
    return (x);
}

static void number_tilde_free(t_number_tilde *x)
{
    if(x->x_gui.x_fsf.x_rcv_able)
        pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
    clock_free(x->x_clock_reset);
    clock_free(x->x_clock_wait);
    clock_free(x->x_clock_repaint);
    outlet_free(x->x_signal_outlet);
    outlet_free(x->x_float_outlet);
    gfxstub_deleteforkey(x);
}

static t_int *number_tilde_perform(t_int *w)
{
    t_number_tilde *x = (t_number_tilde *)(w[1]);
    t_sample *in      = (t_sample *)(w[2]);
    t_sample *out     = (t_sample *)(w[3]);
    t_int n           = (t_int)(w[4]);

    t_int idx = rand() % n;
    
    // Output mode
    if(x->x_mode) {
        // Check if we need to apply a ramp
        if(x->x_ramp_val != x->x_val && x->x_ramp != 0) {
            for(int i = 0; i < n; i++)  {
                
                // Check if we reached our destination
                if((x->x_ramp < 0 && x->x_ramp_val <= x->x_val) || (x->x_ramp > 0 && x->x_ramp_val >= x->x_val)) {
                    x->x_ramp_val = x->x_val;
                    x->x_ramp = 0;
                }
                
                x->x_ramp_val += x->x_ramp;

                out[i] = x->x_ramp_val;
                x->x_last_out = x->x_ramp_val;
            }
        }
        // No ramp needed
        else {
            for(int i = 0; i < n; i++)  {
                out[i] = x->x_val;
            }
        }
    }
    // Input mode
    else {
        x->x_val = in[idx];
        x->x_needs_repaint = 1;
        memcpy(out, in, n);
    }
    
    return (w + 5);
}

static void number_tilde_dsp(t_number_tilde *x, t_signal **sp)
{
    dsp_add(number_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, (t_int)sp[0]->s_n);
}

void number_tilde_setup(void)
{
    number_tilde_class = class_new(gensym("number~"), (t_newmethod)number_tilde_new,
        (t_method)number_tilde_free, sizeof(t_number_tilde), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)number_tilde_new, gensym("number~"), A_GIMME, 0);

    CLASS_MAINSIGNALIN(number_tilde_class, t_number_tilde, x_val);

    class_addmethod(number_tilde_class, (t_method)number_tilde_dsp, gensym("dsp"), A_CANT, 0);
    
    class_addbang(number_tilde_class, number_tilde_bang);
    class_addfloat(number_tilde_class, number_tilde_float);
    class_addlist(number_tilde_class, number_tilde_list);
    
    class_addmethod(number_tilde_class, (t_method)number_tilde_click,
        gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_motion,
        gensym("motion"), A_FLOAT, A_FLOAT, A_DEFFLOAT, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_dialog,
        gensym("dialog"), A_GIMME, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_loadbang,
        gensym("loadbang"), A_DEFFLOAT, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_set,
        gensym("set"), A_FLOAT, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_size,
        gensym("size"), A_GIMME, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_delta,
        gensym("delta"), A_GIMME, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_pos,
        gensym("pos"), A_GIMME, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_color,
        gensym("color"), A_GIMME, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_send,
        gensym("send"), A_DEFSYM, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_receive,
        gensym("receive"), A_DEFSYM, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_label,
        gensym("label"), A_DEFSYM, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_label_pos,
        gensym("label_pos"), A_GIMME, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_label_font,
        gensym("label_font"), A_GIMME, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_interval,
        gensym("interval"), A_FLOAT, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_mode,
        gensym("mode"), A_FLOAT, 0);
    class_addmethod(number_tilde_class, (t_method)number_tilde_init,
        gensym("init"), A_FLOAT, 0);
    class_addmethod(number_tilde_class, (t_method)iemgui_zoom,
        gensym("zoom"), A_CANT, 0);

    number_tilde_widgetbehavior.w_getrectfn =    number_tilde_getrect;
    number_tilde_widgetbehavior.w_displacefn =   iemgui_displace;
    number_tilde_widgetbehavior.w_selectfn =     iemgui_select;
    number_tilde_widgetbehavior.w_activatefn =   NULL;
    number_tilde_widgetbehavior.w_deletefn =     iemgui_delete;
    number_tilde_widgetbehavior.w_visfn =        iemgui_vis;
    number_tilde_widgetbehavior.w_clickfn =      number_tilde_newclick;
    class_setwidget(number_tilde_class, &number_tilde_widgetbehavior);
    class_sethelpsymbol(number_tilde_class, gensym("nbx"));
    class_setsavefn(number_tilde_class, number_tilde_save);
    class_setpropertiesfn(number_tilde_class, number_tilde_properties);
}

// Tcl/Tk code for the preferences dialog, based on the default IEMGUI dialog, but with a few custom fields
static void init_number_tilde_dialog(void)
{
    sys_gui("\n"
            "package provide dialog_number 0.1\n"
            "namespace eval ::dialog_number:: {\n"
            "    variable define_min_fontsize 4\n"
            "\n"
            "    namespace export pdtk_number_tilde_dialog\n"
            "}\n"
            "\n"
            "\n"
            "proc ::dialog_number::clip_dim {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_wdt [concat iemgui_wdt_$vid]\n"
            "    global $var_iemgui_wdt\n"
            "    set var_iemgui_min_wdt [concat iemgui_min_wdt_$vid]\n"
            "    global $var_iemgui_min_wdt\n"
            "    set var_iemgui_hgt [concat iemgui_hgt_$vid]\n"
            "    global $var_iemgui_hgt\n"
            "    set var_iemgui_min_hgt [concat iemgui_min_hgt_$vid]\n"
            "    global $var_iemgui_min_hgt\n"
            "\n"
            "    if {[eval concat $$var_iemgui_wdt] < [eval concat $$var_iemgui_min_wdt]} {\n"
            "        set $var_iemgui_wdt [eval concat $$var_iemgui_min_wdt]\n"
            "        $mytoplevel.dim.w_ent configure -textvariable $var_iemgui_wdt\n"
            "    }\n"
            "    if {[eval concat $$var_iemgui_hgt] < [eval concat $$var_iemgui_min_hgt]} {\n"
            "        set $var_iemgui_hgt [eval concat $$var_iemgui_min_hgt]\n"
            "        $mytoplevel.dim.h_ent configure -textvariable $var_iemgui_hgt\n"
            "    }\n"
            "}\n"
            "\n"
            "\n"
            "proc ::dialog_number::clip_fontsize {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_gn_fs [concat iemgui_gn_fs_$vid]\n"
            "    global $var_iemgui_gn_fs\n"
            "\n"
            "    variable define_min_fontsize\n"
            "\n"
            "    if {[eval concat $$var_iemgui_gn_fs] < $define_min_fontsize} {\n"
            "        set $var_iemgui_gn_fs $define_min_fontsize\n"
            "        $mytoplevel.label.fs_ent configure -textvariable $var_iemgui_gn_fs\n"
            "    }\n"
            "}\n"
            "\n"
            "proc ::dialog_number::set_col_example {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_bcol [concat iemgui_bcol_$vid]\n"
            "    global $var_iemgui_bcol\n"
            "    set var_iemgui_fcol [concat iemgui_fcol_$vid]\n"
            "    global $var_iemgui_fcol\n"
            "    set var_iemgui_lcol [concat iemgui_lcol_$vid]\n"
            "    global $var_iemgui_lcol\n"
            "\n"
            "    $mytoplevel.colors.sections.exp.lb_bk configure \\\n"
            "        -background [eval concat $$var_iemgui_bcol] \\\n"
            "        -activebackground [eval concat $$var_iemgui_bcol] \\\n"
            "        -foreground [eval concat $$var_iemgui_lcol] \\\n"
            "        -activeforeground [eval concat $$var_iemgui_lcol]\n"
            "\n"
            "    if { [eval concat $$var_iemgui_fcol] ne \"none\" } {\n"
            "        $mytoplevel.colors.sections.exp.fr_bk configure \\\n"
            "            -background [eval concat $$var_iemgui_bcol] \\\n"
            "            -activebackground [eval concat $$var_iemgui_bcol] \\\n"
            "            -foreground [eval concat $$var_iemgui_fcol] \\\n"
            "            -activeforeground [eval concat $$var_iemgui_fcol]\n"
            "    } else {\n"
            "        $mytoplevel.colors.sections.exp.fr_bk configure \\\n"
            "            -background [eval concat $$var_iemgui_bcol] \\\n"
            "            -activebackground [eval concat $$var_iemgui_bcol] \\\n"
            "            -foreground [eval concat $$var_iemgui_bcol] \\\n"
            "            -activeforeground [eval concat $$var_iemgui_bcol]}\n"
            "\n"
            "    # for OSX live updates\n"
            "    if {$::windowingsystem eq \"aqua\"} {\n"
            "        ::dialog_number::apply_and_rebind_return $mytoplevel\n"
            "    }\n"
            "}\n"
            "\n"
            "proc ::dialog_number::preset_col {mytoplevel presetcol} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_l2_f1_b0 [concat iemgui_l2_f1_b0_$vid]\n"
            "    global $var_iemgui_l2_f1_b0\n"
            "    set var_iemgui_bcol [concat iemgui_bcol_$vid]\n"
            "    global $var_iemgui_bcol\n"
            "    set var_iemgui_fcol [concat iemgui_fcol_$vid]\n"
            "    global $var_iemgui_fcol\n"
            "    set var_iemgui_lcol [concat iemgui_lcol_$vid]\n"
            "    global $var_iemgui_lcol\n"
            "\n"
            "    if { [eval concat $$var_iemgui_l2_f1_b0] == 0 } { set $var_iemgui_bcol $presetcol }\n"
            "    if { [eval concat $$var_iemgui_l2_f1_b0] == 1 } { set $var_iemgui_fcol $presetcol }\n"
            "    if { [eval concat $$var_iemgui_l2_f1_b0] == 2 } { set $var_iemgui_lcol $presetcol }\n"
            "    ::dialog_number::set_col_example $mytoplevel\n"
            "}\n"
            "\n"
            "proc ::dialog_number::choose_col_bkfrlb {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_l2_f1_b0 [concat iemgui_l2_f1_b0_$vid]\n"
            "    global $var_iemgui_l2_f1_b0\n"
            "    set var_iemgui_bcol [concat iemgui_bcol_$vid]\n"
            "    global $var_iemgui_bcol\n"
            "    set var_iemgui_fcol [concat iemgui_fcol_$vid]\n"
            "    global $var_iemgui_fcol\n"
            "    set var_iemgui_lcol [concat iemgui_lcol_$vid]\n"
            "    global $var_iemgui_lcol\n"
            "\n"
            "    if {[eval concat $$var_iemgui_l2_f1_b0] == 0} {\n"
            "        set $var_iemgui_bcol [eval concat $$var_iemgui_bcol]\n"
            "        set helpstring [tk_chooseColor -title [_ \"Background color\"] -initialcolor [eval concat $$var_iemgui_bcol]]\n"
            "        if { $helpstring ne \"\" } {\n"
            "            set $var_iemgui_bcol $helpstring }\n"
            "    }\n"
            "    if {[eval concat $$var_iemgui_l2_f1_b0] == 1} {\n"
            "        set $var_iemgui_fcol [eval concat $$var_iemgui_fcol]\n"
            "        set helpstring [tk_chooseColor -title [_ \"Foreground color\"] -initialcolor [eval concat $$var_iemgui_fcol]]\n"
            "        if { $helpstring ne \"\" } {\n"
            "            set $var_iemgui_fcol $helpstring }\n"
            "    }\n"
            "    if {[eval concat $$var_iemgui_l2_f1_b0] == 2} {\n"
            "        set helpstring [tk_chooseColor -title [_ \"Label color\"] -initialcolor [eval concat $$var_iemgui_lcol]]\n"
            "        if { $helpstring ne \"\" } {\n"
            "            set $var_iemgui_lcol $helpstring }\n"
            "    }\n"
            "    ::dialog_number::set_col_example $mytoplevel\n"
            "}\n"
            "\n"
            "# open popup over source button\n"
            "proc ::dialog_number::font_popup {mytoplevel} {\n"
            "    $mytoplevel.popup unpost\n"
            "    set button $mytoplevel.label.fontpopup_label\n"
            "    set x [expr [winfo rootx $button] + ( [winfo width $button] / 2 )]\n"
            "    set y [expr [winfo rooty $button] + ( [winfo height $button] / 2 )]\n"
            "    tk_popup $mytoplevel.popup $x $y 0\n"
            "}\n"
            "\n"
            "proc ::dialog_number::toggle_font {mytoplevel gn_f} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_gn_f [concat iemgui_gn_f_$vid]\n"
            "    global $var_iemgui_gn_f\n"
            "\n"
            "    set $var_iemgui_gn_f $gn_f\n"
            "\n"
            "    switch -- $gn_f {\n"
            "        0 { set current_font $::font_family}\n"
            "        1 { set current_font \"Helvetica\" }\n"
            "        2 { set current_font \"Times\" }\n"
            "    }\n"
            "    set current_font_spec \"{$current_font} 14 $::font_weight\"\n"
            "\n"
            "    $mytoplevel.label.fontpopup_label configure -text $current_font \\\n"
            "        -font [list $current_font 16 $::font_weight]\n"
            "    $mytoplevel.label.name_entry configure -font $current_font_spec\n"
            "    $mytoplevel.colors.sections.exp.fr_bk configure -font $current_font_spec\n"
            "    $mytoplevel.colors.sections.exp.lb_bk configure -font $current_font_spec\n"
            "}\n"
            "\n"
            "proc ::dialog_number::lb {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]\n"
            "    global $var_iemgui_loadbang\n"
            "\n"
            "    if {[eval concat $$var_iemgui_loadbang] == 0} {\n"
            "        set $var_iemgui_loadbang 1\n"
            "        $mytoplevel.para.lb configure -text [_ \"Init\"]\n"
            "    } else {\n"
            "        set $var_iemgui_loadbang 0\n"
            "        $mytoplevel.para.lb configure -text [_ \"No init\"]\n"
            "    }\n"
            "}\n"
            "\n"
            "proc ::dialog_number::mode {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_mode [concat iemgui_mode_$vid]\n"
            "    global $var_iemgui_mode\n"
            "\n"
            "    if {[eval concat $$var_iemgui_mode]} {\n"
            "        set $var_iemgui_mode 0\n"
            "        $mytoplevel.para.mode configure -text [_ \"Input\"]\n"
            "    } else {\n"
            "        set $var_iemgui_mode 1\n"
            "        $mytoplevel.para.mode configure -text [_ \"Output\"]\n"
            "    }\n"
            "}\n"
            "\n"
            "proc ::dialog_number::apply {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_wdt [concat iemgui_wdt_$vid]\n"
            "    global $var_iemgui_wdt\n"
            "    set var_iemgui_min_wdt [concat iemgui_min_wdt_$vid]\n"
            "    global $var_iemgui_min_wdt\n"
            "    set var_iemgui_hgt [concat iemgui_hgt_$vid]\n"
            "    global $var_iemgui_hgt\n"
            "    set var_iemgui_min_hgt [concat iemgui_min_hgt_$vid]\n"
            "    global $var_iemgui_min_hgt\n"
            "    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]\n"
            "    global $var_iemgui_loadbang\n"
            "    set var_iemgui_interval [concat iemgui_interval_$vid]\n"
            "    global $var_iemgui_interval\n"
            "    set var_iemgui_mode [concat iemgui_mode_$vid]\n"
            "    global $var_iemgui_mode\n"
            "    set var_iemgui_snd [concat iemgui_snd_$vid]\n"
            "    global $var_iemgui_snd\n"
            "    set var_iemgui_rcv [concat iemgui_rcv_$vid]\n"
            "    global $var_iemgui_rcv\n"
            "    set var_iemgui_gui_nam [concat iemgui_gui_nam_$vid]\n"
            "    global $var_iemgui_gui_nam\n"
            "    set var_iemgui_gn_dx [concat iemgui_gn_dx_$vid]\n"
            "    global $var_iemgui_gn_dx\n"
            "    set var_iemgui_gn_dy [concat iemgui_gn_dy_$vid]\n"
            "    global $var_iemgui_gn_dy\n"
            "    set var_iemgui_gn_f [concat iemgui_gn_f_$vid]\n"
            "    global $var_iemgui_gn_f\n"
            "    set var_iemgui_gn_fs [concat iemgui_gn_fs_$vid]\n"
            "    global $var_iemgui_gn_fs\n"
            "    set var_iemgui_bcol [concat iemgui_bcol_$vid]\n"
            "    global $var_iemgui_bcol\n"
            "    set var_iemgui_fcol [concat iemgui_fcol_$vid]\n"
            "    global $var_iemgui_fcol\n"
            "    set var_iemgui_lcol [concat iemgui_lcol_$vid]\n"
            "    global $var_iemgui_lcol\n"
            "\n"
            "    ::dialog_number::clip_dim $mytoplevel\n"
            "    ::dialog_number::clip_fontsize $mytoplevel\n"
            "\n"
            "    if {[eval concat $$var_iemgui_snd] == \"\"} {set hhhsnd \"empty\"} else {set hhhsnd [eval concat $$var_iemgui_snd]}\n"
            "    if {[eval concat $$var_iemgui_rcv] == \"\"} {set hhhrcv \"empty\"} else {set hhhrcv [eval concat $$var_iemgui_rcv]}\n"
            "    if {[eval concat $$var_iemgui_gui_nam] == \"\"} {set hhhgui_nam \"empty\"\n"
            "    } else {\n"
            "        set hhhgui_nam [eval concat $$var_iemgui_gui_nam]}\n"
            "\n"
            "\n"
            "    set hhhsnd [string map {\"$\" {\\$}} [unspace_text $hhhsnd]]\n"
            "    set hhhrcv [string map {\"$\" {\\$}} [unspace_text $hhhrcv]]\n"
            "    set hhhgui_nam [string map {\"$\" {\\$}} [unspace_text $hhhgui_nam]]\n"
            "\n"
            "    # make sure the offset boxes have a value\n"
            "    if {[eval concat $$var_iemgui_gn_dx] eq \"\"} {set $var_iemgui_gn_dx 0}\n"
            "    if {[eval concat $$var_iemgui_gn_dy] eq \"\"} {set $var_iemgui_gn_dy 0}\n"
            "\n"
            "    pdsend [concat $mytoplevel dialog \\\n"
            "            [eval concat $$var_iemgui_wdt] \\\n"
            "            [eval concat $$var_iemgui_hgt] \\\n"
            "            0 \\\n"
            "            0 \\\n"
            "            0  \\\n"
            "            [eval concat $$var_iemgui_loadbang] \\\n"
            "            [eval concat $$var_iemgui_interval] \\\n"
            "            $hhhsnd \\\n"
            "            $hhhrcv \\\n"
            "            $hhhgui_nam \\\n"
            "            [eval concat $$var_iemgui_gn_dx] \\\n"
            "            [eval concat $$var_iemgui_gn_dy] \\\n"
            "            [eval concat $$var_iemgui_gn_f] \\\n"
            "            [eval concat $$var_iemgui_gn_fs] \\\n"
            "            [string tolower [eval concat $$var_iemgui_bcol]] \\\n"
            "            [string tolower [eval concat $$var_iemgui_fcol]] \\\n"
            "            [string tolower [eval concat $$var_iemgui_lcol]] \\\n"
            "            [eval concat $$var_iemgui_mode]]\n"
            "}\n"
            "\n"
            "\n"
            "proc ::dialog_number::cancel {mytoplevel} {\n"
            "    pdsend \"$mytoplevel cancel\"\n"
            "}\n"
            "\n"
            "proc ::dialog_number::ok {mytoplevel} {\n"
            "    ::dialog_number::apply $mytoplevel\n"
            "    ::dialog_number::cancel $mytoplevel\n"
            "}\n"
            "\n"
            "proc ::dialog_number::pdtk_number_tilde_dialog {mytoplevel dim_header \\\n"
            "                                       wdt min_wdt wdt_label \\\n"
            "                                       hgt min_hgt hgt_label \\\n"
            "                                       rng_header min_rng min_rng_label max_rng \\\n"
            "                                       max_rng_label rng_sched \\\n"
            "                                       loadbang mode interval \\\n"
            "                                       snd rcv \\\n"
            "                                       gui_name \\\n"
            "                                       gn_dx gn_dy gn_f gn_fs \\\n"
            "                                       bcol fcol lcol} {\n"
            "\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_wdt [concat iemgui_wdt_$vid]\n"
            "    global $var_iemgui_wdt\n"
            "    set var_iemgui_min_wdt [concat iemgui_min_wdt_$vid]\n"
            "    global $var_iemgui_min_wdt\n"
            "    set var_iemgui_hgt [concat iemgui_hgt_$vid]\n"
            "    global $var_iemgui_hgt\n"
            "    set var_iemgui_min_hgt [concat iemgui_min_hgt_$vid]\n"
            "    global $var_iemgui_min_hgt\n"
            "    set var_iemgui_rng_sch [concat iemgui_rng_sch_$vid]\n"
            "    global $var_iemgui_rng_sch\n"
            "    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]\n"
            "    global $var_iemgui_loadbang\n"
            "    set var_iemgui_interval [concat iemgui_interval_$vid]\n"
            "    global $var_iemgui_interval\n"
            "    set var_iemgui_mode [concat iemgui_mode_$vid]\n"
            "    global $var_iemgui_mode\n"
            "    set var_iemgui_snd [concat iemgui_snd_$vid]\n"
            "    global $var_iemgui_snd\n"
            "    set var_iemgui_rcv [concat iemgui_rcv_$vid]\n"
            "    global $var_iemgui_rcv\n"
            "    set var_iemgui_gui_nam [concat iemgui_gui_nam_$vid]\n"
            "    global $var_iemgui_gui_nam\n"
            "    set var_iemgui_gn_dx [concat iemgui_gn_dx_$vid]\n"
            "    global $var_iemgui_gn_dx\n"
            "    set var_iemgui_gn_dy [concat iemgui_gn_dy_$vid]\n"
            "    global $var_iemgui_gn_dy\n"
            "    set var_iemgui_gn_f [concat iemgui_gn_f_$vid]\n"
            "    global $var_iemgui_gn_f\n"
            "    set var_iemgui_gn_fs [concat iemgui_gn_fs_$vid]\n"
            "    global $var_iemgui_gn_fs\n"
            "    set var_iemgui_l2_f1_b0 [concat iemgui_l2_f1_b0_$vid]\n"
            "    global $var_iemgui_l2_f1_b0\n"
            "    set var_iemgui_bcol [concat iemgui_bcol_$vid]\n"
            "    global $var_iemgui_bcol\n"
            "    set var_iemgui_fcol [concat iemgui_fcol_$vid]\n"
            "    global $var_iemgui_fcol\n"
            "    set var_iemgui_lcol [concat iemgui_lcol_$vid]\n"
            "    global $var_iemgui_lcol\n"
            "\n"
            "    set $var_iemgui_wdt $wdt\n"
            "    set $var_iemgui_min_wdt $min_wdt\n"
            "    set $var_iemgui_hgt $hgt\n"
            "    set $var_iemgui_min_hgt $min_hgt\n"
            "    set $var_iemgui_rng_sch $rng_sched\n"
            "    set $var_iemgui_loadbang $loadbang\n"
            "    set $var_iemgui_interval $interval\n"
            "    set $var_iemgui_mode $mode\n"
            //"    set $var_iemgui_interval $interval\n"
            "    if {$snd == \"empty\"} {set $var_iemgui_snd [format \"\"]\n"
            "    } else {set $var_iemgui_snd [respace_text [format \"%s\" $snd]]}\n"
            "    if {$rcv == \"empty\"} {set $var_iemgui_rcv [format \"\"]\n"
            "    } else {set $var_iemgui_rcv [respace_text [format \"%s\" $rcv]]}\n"
            "    if {$gui_name == \"empty\"} {set $var_iemgui_gui_nam [format \"\"]\n"
            "    } else {set $var_iemgui_gui_nam [respace_text [format \"%s\" $gui_name]]}\n"
            "\n"
            "    set $var_iemgui_gn_dx $gn_dx\n"
            "    set $var_iemgui_gn_dy $gn_dy\n"
            "    set $var_iemgui_gn_f $gn_f\n"
            "    set $var_iemgui_gn_fs $gn_fs\n"
            "\n"
            "    set $var_iemgui_bcol $bcol\n"
            "    set $var_iemgui_fcol $fcol\n"
            "    set $var_iemgui_lcol $lcol\n"
            "\n"
            "    set $var_iemgui_l2_f1_b0 0\n"
            "\n"
            "    # Override incoming values for known iem guis.\n"
            "    set iemgui_type [_ \"Number~\"]\n"
            "    set wdt_label [_ \"Width (digits):\"]\n"
            "    set hgt_label [_ \"Height:\"]\n"
            "    toplevel $mytoplevel -class DialogWindow\n"
            "    wm title $mytoplevel [format [_ \"%s Properties\"] $iemgui_type]\n"
            "    wm group $mytoplevel .\n"
            "    wm resizable $mytoplevel 0 0\n"
            "    wm transient $mytoplevel $::focused_window\n"
            "    $mytoplevel configure -menu $::dialog_menubar\n"
            "    $mytoplevel configure -padx 0 -pady 0\n"
            "    ::pd_bindings::dialog_bindings $mytoplevel \"iemgui\"\n"
            "\n"
            "    # dimensions\n"
            "    frame $mytoplevel.dim -height 7\n"
            "    pack $mytoplevel.dim -side top\n"
            "    label $mytoplevel.dim.w_lab -text [_ $wdt_label]\n"
            "    entry $mytoplevel.dim.w_ent -textvariable $var_iemgui_wdt -width 4\n"
            "    label $mytoplevel.dim.dummy1 -text \"\" -width 1\n"
            "    label $mytoplevel.dim.h_lab -text [_ $hgt_label]\n"
            "    entry $mytoplevel.dim.h_ent -textvariable $var_iemgui_hgt -width 4\n"
            "    pack $mytoplevel.dim.w_lab $mytoplevel.dim.w_ent -side left\n"
            "    if { $hgt_label ne \"empty\" } {\n"
            "        pack $mytoplevel.dim.dummy1 $mytoplevel.dim.h_lab $mytoplevel.dim.h_ent -side left }\n"
            "\n"
            "\n"
            "    # parameters\n"
            "    labelframe $mytoplevel.para -borderwidth 1 -padx 5 -pady 5 -text [_ \"Parameters\"]\n"
            "    pack $mytoplevel.para -side top -fill x -pady 5\n"
            "    if {[eval concat $$var_iemgui_loadbang] == 0} {\n"
            "        button $mytoplevel.para.lb -text [_ \"No init\"] \\\n"
            "            -command \"::dialog_number::lb $mytoplevel\"  }\n"
            "    if {[eval concat $$var_iemgui_loadbang] == 1} {\n"
            "        button $mytoplevel.para.lb -text [_ \"Init\"] \\\n"
            "            -command \"::dialog_number::lb $mytoplevel\"  }\n"
            "\n"
            "   frame $mytoplevel.para.interval\n"
            "       label $mytoplevel.para.interval.lab -text [_ \"Interval (ms)\"]\n"
            "       entry $mytoplevel.para.interval.ent -textvariable $var_iemgui_interval -width 6\n"
            "       pack $mytoplevel.para.interval.ent $mytoplevel.para.interval.lab -side right -anchor e\n"
            "    if {[eval concat $$var_iemgui_mode] == 0} {\n"
            "        button $mytoplevel.para.mode -command \"::dialog_number::mode $mytoplevel\" \\\n"
            "            -text [_ \"Input\"] }\n"
            "    if {[eval concat $$var_iemgui_mode] == 1} {\n"
            "        button $mytoplevel.para.mode -command \"::dialog_number::mode $mytoplevel\" \\\n"
            "            -text [_ \"Output\"] }\n"
            "    if {[eval concat $$var_iemgui_interval] > 0} {\n"
            "        pack $mytoplevel.para.interval -side left -expand 1 -ipadx 10}\n"
            "    if {[eval concat $$var_iemgui_loadbang] >= 0} {\n"
            "        pack $mytoplevel.para.lb -side left -expand 1 -ipadx 10}\n"
            "    if {[eval concat $$var_iemgui_mode] >= 0} {\n"
            "        pack $mytoplevel.para.mode -side left -expand 1 -ipadx 10}\n"
            "\n"
            "    # messages\n"
            "    labelframe $mytoplevel.s_r -borderwidth 1 -padx 5 -pady 5 -text [_ \"Messages\"]\n"
            "    pack $mytoplevel.s_r -side top -fill x\n"
            "    frame $mytoplevel.s_r.send\n"
            "    pack $mytoplevel.s_r.send -side top -anchor e -padx 5\n"
            "    label $mytoplevel.s_r.send.lab -text [_ \"Send symbol:\"]\n"
            "    entry $mytoplevel.s_r.send.ent -textvariable $var_iemgui_snd -width 21\n"
            "    if { $snd ne \"nosndno\" } {\n"
            "        pack $mytoplevel.s_r.send.lab $mytoplevel.s_r.send.ent -side left \\\n"
            "            -fill x -expand 1\n"
            "    }\n"
            "\n"
            "    frame $mytoplevel.s_r.receive\n"
            "    pack $mytoplevel.s_r.receive -side top -anchor e -padx 5\n"
            "    label $mytoplevel.s_r.receive.lab -text [_ \"Receive symbol:\"]\n"
            "    entry $mytoplevel.s_r.receive.ent -textvariable $var_iemgui_rcv -width 21\n"
            "    if { $rcv ne \"norcvno\" } {\n"
            "        pack $mytoplevel.s_r.receive.lab $mytoplevel.s_r.receive.ent -side left \\\n"
            "            -fill x -expand 1\n"
            "    }\n"
            "\n"
            "    # get the current font name from the int given from C-space (gn_f)\n"
            "    set current_font $::font_family\n"
            "    if {[eval concat $$var_iemgui_gn_f] == 1} \\\n"
            "        { set current_font \"Helvetica\" }\n"
            "    if {[eval concat $$var_iemgui_gn_f] == 2} \\\n"
            "        { set current_font \"Times\" }\n"
            "\n"
            "    # label\n"
            "    labelframe $mytoplevel.label -borderwidth 1 -text [_ \"Label\"] -padx 5 -pady 5\n"
            "    pack $mytoplevel.label -side top -fill x -pady 5\n"
            "    entry $mytoplevel.label.name_entry -textvariable $var_iemgui_gui_nam \\\n"
            "        -width 30 -font [list $current_font 14 $::font_weight]\n"
            "    pack $mytoplevel.label.name_entry -side top -fill both -padx 5\n"
            "\n"
            "    frame $mytoplevel.label.xy -padx 20 -pady 1\n"
            "    pack $mytoplevel.label.xy -side top\n"
            "    label $mytoplevel.label.xy.x_lab -text [_ \"X offset:\"]\n"
            "    entry $mytoplevel.label.xy.x_entry -textvariable $var_iemgui_gn_dx -width 5\n"
            "    label $mytoplevel.label.xy.dummy1 -text \" \" -width 1\n"
            "    label $mytoplevel.label.xy.y_lab -text [_ \"Y offset:\"]\n"
            "    entry $mytoplevel.label.xy.y_entry -textvariable $var_iemgui_gn_dy -width 5\n"
            "    pack $mytoplevel.label.xy.x_lab $mytoplevel.label.xy.x_entry $mytoplevel.label.xy.dummy1 \\\n"
            "        $mytoplevel.label.xy.y_lab $mytoplevel.label.xy.y_entry -side left\n"
            "\n"
            "    button $mytoplevel.label.fontpopup_label -text $current_font \\\n"
            "        -font [list $current_font 16 $::font_weight] -pady 4 \\\n"
            "        -command \"::dialog_number::font_popup $mytoplevel\"\n"
            "    pack $mytoplevel.label.fontpopup_label -side left -anchor w \\\n"
            "        -expand 1 -fill x -padx 5\n"
            "    frame $mytoplevel.label.fontsize\n"
            "    pack $mytoplevel.label.fontsize -side right -padx 5 -pady 5\n"
            "    label $mytoplevel.label.fontsize.label -text [_ \"Size:\"]\n"
            "    entry $mytoplevel.label.fontsize.entry -textvariable $var_iemgui_gn_fs -width 4\n"
            "    pack $mytoplevel.label.fontsize.entry $mytoplevel.label.fontsize.label \\\n"
            "        -side right -anchor e\n"
            "    menu $mytoplevel.popup\n"
            "    $mytoplevel.popup add command \\\n"
            "        -label $::font_family \\\n"
            "        -font [format {{%s} 16 %s} $::font_family $::font_weight] \\\n"
            "        -command \"::dialog_number::toggle_font $mytoplevel 0\"\n"
            "    $mytoplevel.popup add command \\\n"
            "        -label \"Helvetica\" \\\n"
            "        -font [format {Helvetica 16 %s} $::font_weight] \\\n"
            "        -command \"::dialog_number::toggle_font $mytoplevel 1\"\n"
            "    $mytoplevel.popup add command \\\n"
            "        -label \"Times\" \\\n"
            "        -font [format {Times 16 %s} $::font_weight] \\\n"
            "        -command \"::dialog_number::toggle_font $mytoplevel 2\"\n"
            "\n"
            "    # colors\n"
            "    labelframe $mytoplevel.colors -borderwidth 1 -text [_ \"Colors\"] -padx 5 -pady 5\n"
            "    pack $mytoplevel.colors -fill x\n"
            "\n"
            "    frame $mytoplevel.colors.select\n"
            "    pack $mytoplevel.colors.select -side top\n"
            "    radiobutton $mytoplevel.colors.select.radio0 -value 0 -variable \\\n"
            "        $var_iemgui_l2_f1_b0 -text [_ \"Background\"] -justify left\n"
            "    radiobutton $mytoplevel.colors.select.radio1 -value 1 -variable \\\n"
            "        $var_iemgui_l2_f1_b0 -text [_ \"Front\"] -justify left\n"
            "    radiobutton $mytoplevel.colors.select.radio2 -value 2 -variable \\\n"
            "        $var_iemgui_l2_f1_b0 -text [_ \"Label\"] -justify left\n"
            "    if { [eval concat $$var_iemgui_fcol] ne \"none\" } {\n"
            "        pack $mytoplevel.colors.select.radio0 $mytoplevel.colors.select.radio1 \\\n"
            "            $mytoplevel.colors.select.radio2 -side left\n"
            "    } else {\n"
            "        pack $mytoplevel.colors.select.radio0 $mytoplevel.colors.select.radio2 -side left\n"
            "    }\n"
            "\n"
            "    frame $mytoplevel.colors.sections\n"
            "    pack $mytoplevel.colors.sections -side top\n"
            "    button $mytoplevel.colors.sections.but -text [_ \"Compose color\"] \\\n"
            "        -command \"::dialog_number::choose_col_bkfrlb $mytoplevel\"\n"
            "    pack $mytoplevel.colors.sections.but -side left -anchor w -pady 5 \\\n"
            "        -expand yes -fill x\n"
            "    frame $mytoplevel.colors.sections.exp\n"
            "    pack $mytoplevel.colors.sections.exp -side right -padx 5\n"
            "    if { [eval concat $$var_iemgui_fcol] ne \"none\" } {\n"
            "        label $mytoplevel.colors.sections.exp.fr_bk -text \"o=||=o\" -width 6 \\\n"
            "            -background [eval concat $$var_iemgui_bcol] \\\n"
            "            -activebackground [eval concat $$var_iemgui_bcol] \\\n"
            "            -foreground [eval concat $$var_iemgui_fcol] \\\n"
            "            -activeforeground [eval concat $$var_iemgui_fcol] \\\n"
            "            -font [list $current_font 14 $::font_weight] -padx 2 -pady 2 -relief ridge\n"
            "    } else {\n"
            "        label $mytoplevel.colors.sections.exp.fr_bk -text \"o=||=o\" -width 6 \\\n"
            "            -background [eval concat $$var_iemgui_bcol] \\\n"
            "            -activebackground [eval concat $$var_iemgui_bcol] \\\n"
            "            -foreground [eval concat $$var_iemgui_bcol] \\\n"
            "            -activeforeground [eval concat $$var_iemgui_bcol] \\\n"
            "            -font [list $current_font 14 $::font_weight] -padx 2 -pady 2 -relief ridge\n"
            "    }\n"
            "    label $mytoplevel.colors.sections.exp.lb_bk -text [_ \"Test label\"] \\\n"
            "        -background [eval concat $$var_iemgui_bcol] \\\n"
            "        -activebackground [eval concat $$var_iemgui_bcol] \\\n"
            "        -foreground [eval concat $$var_iemgui_lcol] \\\n"
            "        -activeforeground [eval concat $$var_iemgui_lcol] \\\n"
            "        -font [list $current_font 14 $::font_weight] -padx 2 -pady 2 -relief ridge\n"
            "    pack $mytoplevel.colors.sections.exp.lb_bk $mytoplevel.colors.sections.exp.fr_bk \\\n"
            "        -side right -anchor e -expand yes -fill both -pady 7\n"
            "\n"
            "    # color scheme by Mary Ann Benedetto http://piR2.org\n"
            "    foreach r {r1 r2 r3} hexcols {\n"
            "       { \"#FFFFFF\" \"#DFDFDF\" \"#BBBBBB\" \"#FFC7C6\" \"#FFE3C6\" \"#FEFFC6\" \"#C6FFC7\" \"#C6FEFF\" \"#C7C6FF\" \"#E3C6FF\" }\n"
            "       { \"#9F9F9F\" \"#7C7C7C\" \"#606060\" \"#FF0400\" \"#FF8300\" \"#FAFF00\" \"#00FF04\" \"#00FAFF\" \"#0400FF\" \"#9C00FF\" }\n"
            "       { \"#404040\" \"#202020\" \"#000000\" \"#551312\" \"#553512\" \"#535512\" \"#0F4710\" \"#0E4345\" \"#131255\" \"#2F004D\" } } \\\n"
            "    {\n"
            "       frame $mytoplevel.colors.$r\n"
            "       pack $mytoplevel.colors.$r -side top\n"
            "       foreach i { 0 1 2 3 4 5 6 7 8 9} hexcol $hexcols \\\n"
            "           {\n"
            "               label $mytoplevel.colors.$r.c$i -background $hexcol -activebackground $hexcol -relief ridge -padx 7 -pady 0 -width 1\n"
            "               bind $mytoplevel.colors.$r.c$i <Button> \"::dialog_number::preset_col $mytoplevel $hexcol\"\n"
            "           }\n"
            "       pack $mytoplevel.colors.$r.c0 $mytoplevel.colors.$r.c1 $mytoplevel.colors.$r.c2 $mytoplevel.colors.$r.c3 \\\n"
            "           $mytoplevel.colors.$r.c4 $mytoplevel.colors.$r.c5 $mytoplevel.colors.$r.c6 $mytoplevel.colors.$r.c7 \\\n"
            "           $mytoplevel.colors.$r.c8 $mytoplevel.colors.$r.c9 -side left\n"
            "    }\n"
            "\n"
            "    # buttons\n"
            "    frame $mytoplevel.cao -pady 10\n"
            "    pack $mytoplevel.cao -side top\n"
            "    button $mytoplevel.cao.cancel -text [_ \"Cancel\"] \\\n"
            "        -command \"::dialog_number::cancel $mytoplevel\"\n"
            "    pack $mytoplevel.cao.cancel -side left -expand 1 -fill x -padx 15 -ipadx 10\n"
            "    if {$::windowingsystem ne \"aqua\"} {\n"
            "        button $mytoplevel.cao.apply -text [_ \"Apply\"] \\\n"
            "            -command \"::dialog_number::apply $mytoplevel\"\n"
            "        pack $mytoplevel.cao.apply -side left -expand 1 -fill x -padx 15 -ipadx 10\n"
            "    }\n"
            "    button $mytoplevel.cao.ok -text [_ \"OK\"] \\\n"
            "        -command \"::dialog_number::ok $mytoplevel\" -default active\n"
            "    pack $mytoplevel.cao.ok -side left -expand 1 -fill x -padx 15 -ipadx 10\n"
            "\n"
            "    $mytoplevel.dim.w_ent select from 0\n"
            "    $mytoplevel.dim.w_ent select adjust end\n"
            "    focus $mytoplevel.dim.w_ent\n"
            "\n"
            "    # live widget updates on OSX in lieu of Apply button\n"
            "    if {$::windowingsystem eq \"aqua\"} {\n"
            "\n"
            "        # call apply on Return in entry boxes that are in focus & rebind Return to ok button\n"
            "        bind $mytoplevel.dim.w_ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.dim.h_ent <KeyPress-Return>  \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.label.name_entry <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.s_r.send.ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.s_r.receive.ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.label.xy.x_entry <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.label.xy.y_entry <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.label.fontsize.entry <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "\n"
            "        # unbind Return from ok button when an entry takes focus\n"
            "        $mytoplevel.dim.w_ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.dim.h_ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.label.name_entry config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.s_r.send.ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.s_r.receive.ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.label.xy.x_entry config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.label.xy.y_entry config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.label.fontsize.entry config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "\n"
            "        # remove cancel button from focus list since it's not activated on Return\n"
            "        $mytoplevel.cao.cancel config -takefocus 0\n"
            "\n"
            "        # show active focus on the ok button as it *is* activated on Return\n"
            "        $mytoplevel.cao.ok config -default normal\n"
            "        bind $mytoplevel.cao.ok <FocusIn> \"$mytoplevel.cao.ok config -default active\"\n"
            "        bind $mytoplevel.cao.ok <FocusOut> \"$mytoplevel.cao.ok config -default normal\"\n"
            "\n"
            "        # since we show the active focus, disable the highlight outline\n"
            "        $mytoplevel.cao.ok config -highlightthickness 0\n"
            "        $mytoplevel.cao.cancel config -highlightthickness 0\n"
            "    }\n"
            "\n"
            "    position_over_window $mytoplevel $::focused_window\n"
            "}\n"
            "\n"
            "# for live widget updates on OSX\n"
            "proc ::dialog_number::apply_and_rebind_return {mytoplevel} {\n"
            "    ::dialog_number::apply $mytoplevel\n"
            "    bind $mytoplevel <KeyPress-Return> \"::dialog_number::ok $mytoplevel\"\n"
            "    focus $mytoplevel.cao.ok\n"
            "    return 0\n"
            "}\n"
            "\n"
            "# for live widget updates on OSX\n"
            "proc ::dialog_number::unbind_return {mytoplevel} {\n"
            "    bind $mytoplevel <KeyPress-Return> break\n"
            "    return 1\n"
            "}\n");
}
