/* my_numbox.c written by Thomas Musil (c) IEM KUG Graz Austria (2000-2001)
   number.c written by Timothy Schoen (c) for the cyclone library (2022)
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "g_all_guis.h"

#include <math.h>

#include <common/api.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define MINDIGITS 1
#define MINFONT   4

typedef struct _number
{
    t_iemgui x_gui;
    t_clock  *x_clock_reset;
    t_clock  *x_clock_wait;
    int64_t x_val;
    int64_t x_min;
    int64_t x_max;
    char     x_buf[IEMGUI_MAX_NUM_LEN];
    int      x_numwidth;
} t_number;


/*------------------ global functions -------------------------*/

static void number_key(void *z, t_symbol *keysym, t_floatarg fkey);
static void number_draw_update(t_gobj *client, t_glist *glist);
static void init_dialog(void);

/* ------------ nbx gui-my number box ----------------------- */

t_widgetbehavior number_widgetbehavior;
static t_class *number_class;

/* widget helper functions */

static void number_tick_reset(t_number *x)
{
    if(x->x_gui.x_fsf.x_change && x->x_gui.x_glist)
    {
        x->x_gui.x_fsf.x_change = 0;
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
}

static void number_tick_wait(t_number *x)
{
    sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
}

void number_clip(t_number *x)
{
    if(x->x_min == 0 && x->x_max == 0) return;
    
    if(x->x_val < x->x_min)
        x->x_val = x->x_min;
    if(x->x_val > x->x_max)
        x->x_val = x->x_max;
}

static void number_calc_fontwidth(t_number *x)
{
    int w, f = 31;

    if(x->x_gui.x_fsf.x_font_style == 1)
        f = 27;
    else if(x->x_gui.x_fsf.x_font_style == 2)
        f = 25;

    w = x->x_gui.x_fontsize * f * x->x_numwidth;
    w /= 36;
    x->x_gui.x_w = (w + (x->x_gui.x_h/2)/IEMGUI_ZOOM(x) + 4) * IEMGUI_ZOOM(x);
}

static void number_ftoa(t_number *x)
{
    double f = x->x_val;
    int bufsize, is_exp = 0, i, idecimal;

    sprintf(x->x_buf, "%g", f);
    bufsize = (int)strlen(x->x_buf);
    if(bufsize >= 5)/* if it is in exponential mode */
    {
        i = bufsize - 4;
        if((x->x_buf[i] == 'e') || (x->x_buf[i] == 'E'))
            is_exp = 1;
    }
    if(bufsize > x->x_numwidth)/* if to reduce */
    {
        if(is_exp)
        {
            if(x->x_numwidth <= 5)
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            i = bufsize - 4;
            for(idecimal = 0; idecimal < i; idecimal++)
                if(x->x_buf[idecimal] == '.')
                    break;
            if(idecimal > (x->x_numwidth - 4))
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            else
            {
                int new_exp_index = x->x_numwidth - 4;
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
            if(idecimal > x->x_numwidth)
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            else
                x->x_buf[x->x_numwidth] = 0;
        }
    }
}

static void number_draw_update(t_gobj *client, t_glist *glist)
{
    t_number *x = (t_number *)client;
    if(glist_isvisible(glist))
    {
        if(x->x_gui.x_fsf.x_change)
        {
            if(x->x_buf[0])
            {
                char *cp = x->x_buf;
                int sl = (int)strlen(x->x_buf);

                x->x_buf[sl] = '>';
                x->x_buf[sl+1] = 0;
                if(sl >= x->x_numwidth)
                    cp += sl - x->x_numwidth + 1;
                sys_vgui(".x%lx.c itemconfigure %lxNUMBER -fill #%06x -text {%s} \n",
                         glist_getcanvas(glist), x,
                         x->x_gui.x_lcol, cp);
                x->x_buf[sl] = 0;
            }
            else
            {
                number_ftoa(x);
                sys_vgui(".x%lx.c itemconfigure %lxNUMBER -fill #%06x -text {%s} \n",
                         glist_getcanvas(glist), x,
                         x->x_gui.x_lcol, x->x_buf);
                x->x_buf[0] = 0;
            }
            
            sys_vgui(".x%lx.c itemconfigure %lxBASE2 -fill #%06x\n",
                     glist_getcanvas(glist), x,
                     IEM_GUI_COLOR_EDITED);
        }
        else
        {
            number_ftoa(x);
            sys_vgui(".x%lx.c itemconfigure %lxNUMBER -fill #%06x -text {%s} \n",
                     glist_getcanvas(glist), x,
                    x->x_gui.x_lcol,
                     x->x_buf);
            
            sys_vgui(".x%lx.c itemconfigure %lxBASE2 -fill #%06x\n",
                     glist_getcanvas(glist), x,
                     x->x_gui.x_fcol);
            
            x->x_buf[0] = 0;
        }
    }
}

static void number_draw_new(t_number *x, t_glist *glist)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int w = x->x_gui.x_w, half = x->x_gui.x_h/2;
    int d = IEMGUI_ZOOM(x) + x->x_gui.x_h/(34*IEMGUI_ZOOM(x));
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = IEM_GUI_IOHEIGHT * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d "
             "-width %d -outline #%06x -fill #%06x -tags %lxBASE1\n",
             canvas,
             xpos, ypos,
             xpos + w, ypos,
             xpos + w, ypos + x->x_gui.x_h,
             xpos, ypos + x->x_gui.x_h,
             xpos, ypos,
             IEMGUI_ZOOM(x), IEM_GUI_COLOR_NORMAL, x->x_gui.x_bcol, x);

    sys_vgui(".x%lx.c create polygon %d %d %d %d %d %d %d %d "
        "-width %d -outline #%06x -fill #%06x -tags %lxBASE2\n",
        canvas,
        xpos + IEMGUI_ZOOM(x), ypos + IEMGUI_ZOOM(x),
        xpos + half, ypos + half,
        xpos + IEMGUI_ZOOM(x), ypos + x->x_gui.x_h - IEMGUI_ZOOM(x),
        xpos + IEMGUI_ZOOM(x), ypos + IEMGUI_ZOOM(x),
        IEMGUI_ZOOM(x), x->x_gui.x_bcol, x->x_gui.x_fcol, x);

    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxOUT%d outlet]\n",
             canvas,
             xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + iow, ypos + x->x_gui.x_h,
             x, 0);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxIN%d inlet]\n",
             canvas,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh,
             x, 0);
    number_ftoa(x);
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w -font {{%s} -%d %s} -fill #%06x -tags %lxNUMBER\n",
             canvas, xpos + half + 2*IEMGUI_ZOOM(x), ypos + half + d,
             x->x_buf, x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x),
             sys_fontweight, x->x_gui.x_lcol, x);
}

static void number_draw_move(t_number *x, t_glist *glist)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int w = x->x_gui.x_w, half = x->x_gui.x_h/2;
    int d = IEMGUI_ZOOM(x) + x->x_gui.x_h / (34 * IEMGUI_ZOOM(x));
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = IEM_GUI_IOHEIGHT * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c coords %lxBASE1 %d %d %d %d %d %d %d %d %d %d \n",
             canvas, x,
             xpos, ypos,
             xpos + w, ypos,
             xpos + w, ypos + x->x_gui.x_h,
             xpos, ypos + x->x_gui.x_h,
             xpos, ypos);

    sys_vgui(".x%lx.c coords %lxBASE2 %d %d %d %d %d %d %d %d\n",
             canvas, x,
            xpos + IEMGUI_ZOOM(x), ypos + IEMGUI_ZOOM(x),
            xpos + half, ypos + half,
            xpos + IEMGUI_ZOOM(x), ypos + x->x_gui.x_h - IEMGUI_ZOOM(x),
            xpos + IEMGUI_ZOOM(x), ypos + IEMGUI_ZOOM(x));

    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + iow, ypos + x->x_gui.x_h);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh);
    sys_vgui(".x%lx.c coords %lxNUMBER %d %d\n",
             canvas, x, xpos + half + 2*IEMGUI_ZOOM(x), ypos + half + d);
}

static void number_draw_erase(t_number* x, t_glist* glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c delete %lxBASE1\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxBASE2\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxNUMBER\n", canvas, x);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void number_draw_config(t_number* x, t_glist* glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c itemconfigure %lxNUMBER -font {{%s} -%d %s} -fill #%06x \n",
             canvas, x, x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x), sys_fontweight,
             x->x_gui.x_lcol);
    sys_vgui(".x%lx.c itemconfigure %lxBASE1 -fill #%06x\n", canvas,
             x, x->x_gui.x_bcol);
    
    sys_vgui(".x%lx.c itemconfigure %lxBASE2 -outline #%06x\n", canvas, x,
             (x->x_gui.x_bcol));
    sys_vgui(".x%lx.c itemconfigure %lxBASE2 -fill #%06x\n", canvas, x,
             (x->x_gui.x_fsf.x_change ? IEM_GUI_COLOR_SELECTED : x->x_gui.x_fcol));
}

static void number_draw_io(t_number* x,t_glist* glist, int old_snd_rcv_flags)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = IEM_GUI_IOHEIGHT * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);

    if((old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && !x->x_gui.x_fsf.x_snd_able) {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags %lxOUT%d\n",
             canvas,
             xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + iow, ypos + x->x_gui.x_h,
             x, 0);
        /* keep these above outlet */
        sys_vgui(".x%lx.c raise %lxNUMBER %lxOUT%d\n", canvas, x, x);
    }
    if(!(old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    if((old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && !x->x_gui.x_fsf.x_rcv_able) {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags %lxIN%d\n",
             canvas,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh,
             x, 0);
        /* keep these above inlet */
        sys_vgui(".x%lx.c raise %lxNUMBER %lxIN%d\n", canvas, x, x);
    }
    if(!(old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void number_draw_select(t_number *x, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    if(x->x_gui.x_fsf.x_selected)
    {
        if(x->x_gui.x_fsf.x_change)
        {
            x->x_gui.x_fsf.x_change = 0;
            clock_unset(x->x_clock_reset);
            x->x_buf[0] = 0;
            sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
        }
        sys_vgui(".x%lx.c itemconfigure %lxBASE1 -outline #%06x\n",
            canvas, x, IEM_GUI_COLOR_SELECTED);
    }
    else
    {
        sys_vgui(".x%lx.c itemconfigure %lxBASE1 -outline #%06x\n",
            canvas, x, IEM_GUI_COLOR_NORMAL);
    }
}

static void number_draw(t_number *x, t_glist *glist, int mode)
{
    if(mode == IEM_GUI_DRAW_MODE_UPDATE)
        sys_queuegui(x, glist, number_draw_update);
    else if(mode == IEM_GUI_DRAW_MODE_MOVE)
        number_draw_move(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_NEW)
        number_draw_new(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_SELECT)
        number_draw_select(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_ERASE)
        number_draw_erase(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_CONFIG)
        number_draw_config(x, glist);
    else if(mode >= IEM_GUI_DRAW_MODE_IO)
        number_draw_io(x, glist, mode - IEM_GUI_DRAW_MODE_IO);
}

/* ------------------------ nbx widgetbehaviour----------------------------- */

static void number_getrect(t_gobj *z, t_glist *glist,
                              int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_number* x = (t_number*)z;

    *xp1 = text_xpix(&x->x_gui.x_obj, glist);
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w;
    *yp2 = *yp1 + x->x_gui.x_h;
}

static void number_save(t_gobj *z, t_binbuf *b)
{
    t_number *x = (t_number *)z;
    t_symbol *bflcol[3];
    t_symbol *srl[3];

    iemgui_save(&x->x_gui, srl, bflcol);
    if(x->x_gui.x_fsf.x_change)
    {
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
    
    binbuf_addv(b, "ssiisiiffisssiiiisssf", gensym("#X"), gensym("obj"),
                (int)x->x_gui.x_obj.te_xpix, (int)x->x_gui.x_obj.te_ypix,
                gensym("number"), x->x_numwidth, (x->x_gui.x_h /IEMGUI_ZOOM(x)) - 5,
                (t_float)x->x_min, (t_float)x->x_max,
                iem_symargstoint(&x->x_gui.x_isa),
                srl[0], srl[1], srl[2],
                x->x_gui.x_ldx, x->x_gui.x_ldy,
                iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
                bflcol[0], bflcol[1], bflcol[2],
                x->x_gui.x_isa.x_loadinit?x->x_val:0.);
    binbuf_addv(b, ";");
}

int number_check_minmax(t_number *x, int64_t min, int64_t max)
{
    int ret = 0;

    x->x_min = min;
    x->x_max = max;
    if(x->x_val < x->x_min)
    {
        x->x_val = x->x_min;
        ret = 1;
    }
    if(x->x_val > x->x_max)
    {
        x->x_val = x->x_max;
        ret = 1;
    }
    return(ret);
}

static void number_properties(t_gobj *z, t_glist *owner)
{
    init_dialog();
    
    t_number *x = (t_number *)z;
    char buf[800];
    t_symbol *srl[3];
    
    iemgui_properties(&x->x_gui, srl);
    if(x->x_gui.x_fsf.x_change)
    {
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
    
    sprintf(buf, "::dialog_number::pdtk_number_dialog %%s \
            -------dimensions(digits)(pix):------- %d %d width: %d %d height: \
            -----------output-range:----------- %lld min: %lld max: %d \
            %d %d \
            %s %s \
            %s %d %d \
            %d %d \
            #%06x #%06x #%06x\n",
            x->x_numwidth, MINDIGITS, (x->x_gui.x_h /IEMGUI_ZOOM(x)) - 5, IEM_GUI_MINSIZE,
            x->x_min, x->x_max, 0,/*no_schedule*/
            x->x_gui.x_isa.x_loadinit, -1,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
            x->x_gui.x_fsf.x_font_style, x->x_gui.x_fontsize,
            0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol,
                0xffffff & x->x_gui.x_lcol);


    gfxstub_new(&x->x_gui.x_obj.ob_pd, x, buf);
}

static void number_bang(t_number *x)
{
    outlet_float(x->x_gui.x_obj.ob_outlet, x->x_val);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, x->x_val);
}

static void number_dialog(t_number *x, t_symbol *s, int argc,
    t_atom *argv)
{
    #define SETCOLOR(a, col) do {char color[MAXPDSTRING]; snprintf(color, MAXPDSTRING-1, "#%06x", 0xffffff & col); color[MAXPDSTRING-1] = 0; SETSYMBOL(a, gensym(color));} while(0)
    
    t_symbol *srl[3];
    int w = (int)atom_getfloatarg(0, argc, argv);
    int h = (int)atom_getfloatarg(1, argc, argv);
    double min = (double)atom_getfloatarg(2, argc, argv);
    double max = (double)atom_getfloatarg(3, argc, argv);
    int sr_flags;
    
    /*
    t_atom undo[13];
    
    SETFLOAT(undo+0, x->x_numwidth);
    SETFLOAT(undo+2, x->x_min);
    SETFLOAT(undo+3, x->x_max);
    SETFLOAT (argv+ 4, x->x_gui.x_w/IEMGUI_ZOOM(x));
    SETFLOAT (argv+ 5, x->x_gui.x_h/IEMGUI_ZOOM(x));
    SETFLOAT (argv+ 6, x->x_gui.x_isa.x_loadinit);
    SETSYMBOL(argv+ 7, srl[0]);
    SETSYMBOL(argv+ 8, srl[1]);
    SETSYMBOL(argv+ 9, srl[2]);
    SETCOLOR (argv+10, x->x_gui.x_bcol);
    SETCOLOR (argv+11, x->x_gui.x_fcol);
    SETCOLOR (argv+12, x->x_gui.x_lcol);
    
    pd_undo_set_objectstate(x->x_gui.x_glist, (t_pd*)x, gensym("dialog"),
                            13, undo,
                            argc, argv); */

    sr_flags = iemgui_dialog(&x->x_gui, srl, argc, argv);
    
    if(w < MINDIGITS)
        w = MINDIGITS;
    x->x_numwidth = w;
    if(h < IEM_GUI_MINSIZE)
        h = IEM_GUI_MINSIZE;
    x->x_gui.x_h = (h + 5) * IEMGUI_ZOOM(x);
    x->x_gui.x_fontsize = h;
    
    number_calc_fontwidth(x);
    /*if(number_check_minmax(x, min, max))
     number_bang(x);*/
    number_check_minmax(x, min, max);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_IO + sr_flags);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_CONFIG);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_MOVE);
    canvas_fixlinesfor(x->x_gui.x_glist, (t_text*)x);
}

static void number_motion(t_number *x, t_floatarg dx, t_floatarg dy,
    t_floatarg up)
{
    double k2 = 1.0;
    if (up != 0)
        return;

    if(x->x_gui.x_fsf.x_finemoved)
        k2 = 0.01;

    x->x_val -= k2*dy;
    
    number_clip(x);
    sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    number_bang(x);
    clock_unset(x->x_clock_reset);
}

static void number_click(t_number *x, t_floatarg xpos, t_floatarg ypos,
                            t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    glist_grab(x->x_gui.x_glist, &x->x_gui.x_obj.te_g,
        (t_glistmotionfn)number_motion, number_key, xpos, ypos);
}

static int number_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_number* x = (t_number *)z;

    if(doit)
    {
        number_click( x, (t_floatarg)xpix, (t_floatarg)ypix,
            (t_floatarg)shift, 0, (t_floatarg)alt);
        if(shift)
            x->x_gui.x_fsf.x_finemoved = 1;
        else
            x->x_gui.x_fsf.x_finemoved = 0;
        if(!x->x_gui.x_fsf.x_change)
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
            sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
        }
    }
    return (1);
}

static void number_set(t_number *x, t_floatarg f)
{
    t_float ftocompare = f;
    t_float currentfloat = x->x_val;
        /* bitwise comparison, suggested by Dan Borstein - to make this work
        ftocompare must be t_float type like x_val. */
    if (memcmp(&ftocompare, &currentfloat, sizeof(ftocompare)))
    {
        x->x_val = (int)ftocompare;
        number_clip(x);
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
}

static void number_float(t_number *x, t_floatarg f)
{
    number_set(x, f);
    if(x->x_gui.x_fsf.x_put_in2out)
        number_bang(x);
}

static void number_size(t_number *x, t_symbol *s, int ac, t_atom *av)
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
        x->x_gui.x_h = (h + 5) * IEMGUI_ZOOM(x);
        x->x_gui.x_fontsize = h * IEMGUI_ZOOM(x);
    }
    number_calc_fontwidth(x);
    iemgui_size((void *)x, &x->x_gui);
}

static void number_delta(t_number *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void number_pos(t_number *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}

static void number_minimum(t_number *x, t_floatarg f)
{
    if(number_check_minmax(x, f,
                                 x->x_max))
    {
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
}

static void number_maximum(t_number *x, t_floatarg f)
{
    if(number_check_minmax(x, x->x_min,
                                 f))
    {
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
}

static void number_color(t_number *x, t_symbol *s, int ac, t_atom *av)
{iemgui_color((void *)x, &x->x_gui, s, ac, av);}

static void number_send(t_number *x, t_symbol *s)
{iemgui_send(x, &x->x_gui, s);}

static void number_receive(t_number *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}

static void number_init(t_number *x, t_floatarg f)
{
    x->x_gui.x_isa.x_loadinit = (f == 0.0) ? 0 : 1;
}

static void number_loadbang(t_number *x, t_floatarg action)
{
    if(action == LB_LOAD && x->x_gui.x_isa.x_loadinit)
    {
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
        number_bang(x);
    }
}

static void number_key(void *z, t_symbol *keysym, t_floatarg fkey)
{
    t_number *x = z;
    char c = fkey;
    char buf[3];
    buf[1] = 0;

    if(c == 0)
    {
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
        return;
    }
    if(((c >= '0') && (c <= '9')) || (c == '-') ||
        (c == 'e') || (c == '+') || (c == 'E'))
    {
        if(strlen(x->x_buf) < (IEMGUI_MAX_NUM_LEN-2))
        {
            buf[0] = c;
            strcat(x->x_buf, buf);
            sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
        }
    }
    else if((c == '\b') || (c == 127))
    {
        int sl = (int)strlen(x->x_buf) - 1;

        if(sl < 0)
            sl = 0;
        x->x_buf[sl] = 0;
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
    else if(((c == '\n') || (c == 13)) && x->x_buf[0] != 0)
    {
        x->x_val = atof(x->x_buf);
        x->x_buf[0] = 0;
        x->x_gui.x_fsf.x_change = 0;
        clock_unset(x->x_clock_reset);
        number_clip(x);
        number_bang(x);
        sys_queuegui(x, x->x_gui.x_glist, number_draw_update);
    }
    clock_delay(x->x_clock_reset, 3000);
}

// Number list is also called when there is a key event
static void number_list(t_number *x, t_symbol *s, int ac, t_atom *av)
{
    if(!ac) {
        number_bang(x);
        return;
    }

    int editmode = x->x_gui.x_glist->gl_edit;
    int selected = x->x_gui.x_fsf.x_change;
    int is_keydown_event = ac == 2 && IS_A_FLOAT(av, 0) && IS_A_SYMBOL(av, 1) && atom_getfloat(av) == 1;
    // Check for key events
    if(!editmode && selected && is_keydown_event) {
        const char* symbol = atom_getsymbol(av + 1)->s_name;
        
        if(!strcmp(symbol, "Up")) {
            number_set(x, x->x_val + 1);
            number_bang(x);
        }
        if(!strcmp(symbol, "Down")) {
            number_set(x, x->x_val - 1);
            number_bang(x);
        }
        
        clock_delay(x->x_clock_reset, 3000);
        
        // don't handle these as number!
        return;
    }
    
    if(IS_A_FLOAT(av, 0) && (ac != 2 || !IS_A_SYMBOL(av, 1)))
    {
        number_set(x, atom_getfloatarg(0, ac, av));
        number_bang(x);
    }
}

static void *number_new(t_symbol *s, int argc, t_atom *argv)
{
    t_number *x = (t_number *)pd_new(number_class);
    int w = 3, h = 12;
    int ldx = 0, ldy = -8;
    int64_t min = 0, max = 0, v = 0;

    x->x_gui.x_bcol = 0xFCFCFC;
    x->x_gui.x_fcol = 0x00;
    x->x_gui.x_lcol = 0x00;

    pd_bind(&x->x_gui.x_obj.ob_pd, gensym("#keyname"));
    
    if((argc >= 16)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
       &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
       &&IS_A_FLOAT(argv,4)
       &&(IS_A_SYMBOL(argv,5)||IS_A_FLOAT(argv,5))
       &&(IS_A_SYMBOL(argv,6)||IS_A_FLOAT(argv,6))
       &&(IS_A_SYMBOL(argv,7)||IS_A_FLOAT(argv,7))
       &&IS_A_FLOAT(argv,8)&&IS_A_FLOAT(argv,9)
       &&IS_A_FLOAT(argv,10)&&IS_A_FLOAT(argv,11)&&IS_A_FLOAT(argv,15))
    {
        w = (int)atom_getfloatarg(0, argc, argv);
        h = (int)atom_getfloatarg(1, argc, argv);
        
        min = (double)atom_getfloatarg(2, argc, argv);
        max = (double)atom_getfloatarg(3, argc, argv);
        iem_inttosymargs(&x->x_gui.x_isa, atom_getfloatarg(4, argc, argv));
        iemgui_new_getnames(&x->x_gui, 5, argv);
        iem_inttofstyle(&x->x_gui.x_fsf, atom_getfloatarg(10, argc, argv));
        iemgui_all_loadcolors(&x->x_gui, argv+12, argv+13, argv+14);
        v = atom_getfloatarg(15, argc, argv);
    }
    else iemgui_new_getnames(&x->x_gui, 6, 0);
    x->x_gui.x_draw = (t_iemfunptr)number_draw;
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

    if(w < MINDIGITS)
        w = MINDIGITS;
    x->x_numwidth = w;
    if(h < IEM_GUI_MINSIZE)
        h = IEM_GUI_MINSIZE;
    x->x_gui.x_h = (h + 5) * IEMGUI_ZOOM(x);
    x->x_gui.x_fontsize = h * IEMGUI_ZOOM(x);
    
    x->x_buf[0] = 0;
    number_check_minmax(x, min, max);
    iemgui_verify_snd_ne_rcv(&x->x_gui);
    x->x_clock_reset = clock_new(x, (t_method)number_tick_reset);
    x->x_clock_wait = clock_new(x, (t_method)number_tick_wait);
    x->x_gui.x_fsf.x_change = 0;
    iemgui_newzoom(&x->x_gui);
    number_calc_fontwidth(x);
    outlet_new(&x->x_gui.x_obj, &s_float);
    return (x);
}

static void number_free(t_number *x)
{
    if(x->x_gui.x_fsf.x_rcv_able)
        pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
    clock_free(x->x_clock_reset);
    clock_free(x->x_clock_wait);
    gfxstub_deleteforkey(x);
}

CYCLONE_OBJ_API void number_setup(void)
{
    number_class = class_new(gensym("number"), (t_newmethod)number_new,
        (t_method)number_free, sizeof(t_number), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)number_new, gensym("number"), A_GIMME, 0);
    class_addbang(number_class, number_bang);
    class_addfloat(number_class, number_float);
    class_addlist(number_class, number_list);
    class_addmethod(number_class, (t_method)number_click,
        gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(number_class, (t_method)number_motion,
        gensym("motion"), A_FLOAT, A_FLOAT, A_DEFFLOAT, 0);
    class_addmethod(number_class, (t_method)number_dialog,
        gensym("dialog"), A_GIMME, 0);
    class_addmethod(number_class, (t_method)number_loadbang,
        gensym("loadbang"), A_DEFFLOAT, 0);
    class_addmethod(number_class, (t_method)number_set,
        gensym("set"), A_FLOAT, 0);
    class_addmethod(number_class, (t_method)number_size,
        gensym("size"), A_GIMME, 0);
    class_addmethod(number_class, (t_method)number_delta,
        gensym("delta"), A_GIMME, 0);
    class_addmethod(number_class, (t_method)number_pos,
        gensym("pos"), A_GIMME, 0);
    class_addmethod(number_class, (t_method)number_minimum,
        gensym("minimum"), A_GIMME, 0);
    class_addmethod(number_class, (t_method)number_maximum,
        gensym("maximum"), A_GIMME, 0);
    class_addmethod(number_class, (t_method)number_color,
        gensym("color"), A_GIMME, 0);
    class_addmethod(number_class, (t_method)number_send,
        gensym("send"), A_DEFSYM, 0);
    class_addmethod(number_class, (t_method)number_receive,
        gensym("receive"), A_DEFSYM, 0);
    class_addmethod(number_class, (t_method)number_init,
        gensym("init"), A_FLOAT, 0);
    class_addmethod(number_class, (t_method)iemgui_zoom,
        gensym("zoom"), A_CANT, 0);
        
    number_widgetbehavior.w_getrectfn =    number_getrect;
    number_widgetbehavior.w_displacefn =   iemgui_displace;
    number_widgetbehavior.w_selectfn =     iemgui_select;
    number_widgetbehavior.w_activatefn =   NULL;
    number_widgetbehavior.w_deletefn =     iemgui_delete;
    number_widgetbehavior.w_visfn =        iemgui_vis;
    number_widgetbehavior.w_clickfn =      number_newclick;

    class_setwidget(number_class, &number_widgetbehavior);
    class_setsavefn(number_class, number_save);
    class_setpropertiesfn(number_class, number_properties);
}

static void init_dialog(void)
{
    sys_gui("\n"
            "package provide dialog_number 0.1\n"
            "namespace eval ::dialog_number:: {\n"
            "    variable define_min_flashhold 50\n"
            "    variable define_min_flashbreak 10\n"
            "    variable define_min_fontsize 4\n"
            "\n"
            "    namespace export pdtk_number_dialog\n"
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
            "proc ::dialog_number::sched_rng {mytoplevel} {\n"
            "    set vid [string trimleft $mytoplevel .]\n"
            "\n"
            "    set var_iemgui_min_rng [concat iemgui_min_rng_$vid]\n"
            "    global $var_iemgui_min_rng\n"
            "    set var_iemgui_max_rng [concat iemgui_max_rng_$vid]\n"
            "    global $var_iemgui_max_rng\n"
            "    set var_iemgui_rng_sch [concat iemgui_rng_sch_$vid]\n"
            "    global $var_iemgui_rng_sch\n"
            "\n"
            "    variable define_min_flashhold\n"
            "    variable define_min_flashbreak\n"
            "\n"
            "    if {[eval concat $$var_iemgui_rng_sch] == 2} {\n"
            "        if {[eval concat $$var_iemgui_max_rng] < [eval concat $$var_iemgui_min_rng]} {\n"
            "            set hhh [eval concat $$var_iemgui_min_rng]\n"
            "            set $var_iemgui_min_rng [eval concat $$var_iemgui_max_rng]\n"
            "            set $var_iemgui_max_rng $hhh\n"
            "            $mytoplevel.rng.max_ent configure -textvariable $var_iemgui_max_rng\n"
            "            $mytoplevel.rng.min.ent configure -textvariable $var_iemgui_min_rng }\n"
            "        if {[eval concat $$var_iemgui_max_rng] < $define_min_flashhold} {\n"
            "            set $var_iemgui_max_rng $define_min_flashhold\n"
            "            $mytoplevel.rng.max_ent configure -textvariable $var_iemgui_max_rng\n"
            "        }\n"
            "        if {[eval concat $$var_iemgui_min_rng] < $define_min_flashbreak} {\n"
            "            set $var_iemgui_min_rng $define_min_flashbreak\n"
            "            $mytoplevel.rng.min.ent configure -textvariable $var_iemgui_min_rng\n"
            "        }\n"
            "    }\n"
            "    if {[eval concat $$var_iemgui_rng_sch] == 1} {\n"
            "        if {[eval concat $$var_iemgui_min_rng] == 0.0} {\n"
            "            set $var_iemgui_min_rng 1.0\n"
            "            $mytoplevel.rng.min.ent configure -textvariable $var_iemgui_min_rng\n"
            "        }\n"
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
            "        set helpstring [tk_chooseColor -title [_ \"Text color\"] -initialcolor [eval concat $$var_iemgui_lcol]]\n"
            "        if { $helpstring ne \"\" } {\n"
            "            set $var_iemgui_lcol $helpstring }\n"
            "    }\n"
            "    ::dialog_number::set_col_example $mytoplevel\n"
            "}\n"
            "\n"
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
            "    set var_iemgui_min_rng [concat iemgui_min_rng_$vid]\n"
            "    global $var_iemgui_min_rng\n"
            "    set var_iemgui_max_rng [concat iemgui_max_rng_$vid]\n"
            "    global $var_iemgui_max_rng\n"
            "    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]\n"
            "    global $var_iemgui_loadbang\n"
            "    set var_iemgui_snd [concat iemgui_snd_$vid]\n"
            "    global $var_iemgui_snd\n"
            "    set var_iemgui_rcv [concat iemgui_rcv_$vid]\n"
            "    global $var_iemgui_rcv\n"
            "    set var_iemgui_gui_nam [concat iemgui_gui_nam_$vid]\n"
            "    global $var_iemgui_gui_nam\n"
            "    set var_iemgui_bcol [concat iemgui_bcol_$vid]\n"
            "    global $var_iemgui_bcol\n"
            "    set var_iemgui_fcol [concat iemgui_fcol_$vid]\n"
            "    global $var_iemgui_fcol\n"
            "    set var_iemgui_lcol [concat iemgui_lcol_$vid]\n"
            "    global $var_iemgui_lcol\n"
            "\n"
            
            "    ::dialog_number::clip_dim $mytoplevel\n"
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
            
            
            "    pdsend [concat $mytoplevel dialog \\\n"
            "            [eval concat $$var_iemgui_wdt] \\\n"
            "            [eval concat $$var_iemgui_hgt] \\\n"
            "            [eval concat $$var_iemgui_min_rng] \\\n"
            "            [eval concat $$var_iemgui_max_rng] \\\n"
            "            0  \\\n"
            "            [eval concat $$var_iemgui_loadbang] \\\n"
            "            0  \\\n"
            "            $hhhsnd \\\n"
            "            $hhhrcv \\\n"
            "            $hhhgui_nam \\\n"
            "            0  \\\n"
            "            0  \\\n"
            "            0  \\\n"
            "            0  \\\n"
            "            [string tolower [eval concat $$var_iemgui_bcol]] \\\n"
            "            [string tolower [eval concat $$var_iemgui_fcol]] \\\n"
            "            [string tolower [eval concat $$var_iemgui_lcol]]] \\\n"
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
            "proc ::dialog_number::pdtk_number_dialog {mytoplevel dim_header \\\n"
            "                                       wdt min_wdt wdt_label \\\n"
            "                                       hgt min_hgt hgt_label \\\n"
            "                                       rng_header min_rng min_rng_label max_rng \\\n"
            "                                       max_rng_label rng_sched \\\n"
            "                                       loadbang steady \\\n"
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
            "    set var_iemgui_min_rng [concat iemgui_min_rng_$vid]\n"
            "    global $var_iemgui_min_rng\n"
            "    set var_iemgui_max_rng [concat iemgui_max_rng_$vid]\n"
            "    global $var_iemgui_max_rng\n"
            "    set var_iemgui_rng_sch [concat iemgui_rng_sch_$vid]\n"
            "    global $var_iemgui_rng_sch\n"
            "    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]\n"
            "    global $var_iemgui_loadbang\n"
            "    set var_iemgui_snd [concat iemgui_snd_$vid]\n"
            "    global $var_iemgui_snd\n"
            "    set var_iemgui_rcv [concat iemgui_rcv_$vid]\n"
            "    global $var_iemgui_rcv\n"
            "    set var_iemgui_gui_nam [concat iemgui_gui_nam_$vid]\n"
            "    global $var_iemgui_gui_nam\n"
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
            "    set $var_iemgui_min_rng $min_rng\n"
            "    set $var_iemgui_max_rng $max_rng\n"
            "    set $var_iemgui_loadbang $loadbang\n"
            "    if {$snd == \"empty\"} {set $var_iemgui_snd [format \"\"]\n"
            "    } else {set $var_iemgui_snd [respace_text [format \"%s\" $snd]]}\n"
            "    if {$rcv == \"empty\"} {set $var_iemgui_rcv [format \"\"]\n"
            "    } else {set $var_iemgui_rcv [respace_text [format \"%s\" $rcv]]}\n"
            "    if {$gui_name == \"empty\"} {set $var_iemgui_gui_nam [format \"\"]\n"
            "    } else {set $var_iemgui_gui_nam [respace_text [format \"%s\" $gui_name]]}\n"
            "\n"
            "    set $var_iemgui_bcol $bcol\n"
            "    set $var_iemgui_fcol $fcol\n"
            "    set $var_iemgui_lcol $lcol\n"
            "\n"
            "    set $var_iemgui_l2_f1_b0 0\n"
            "\n"
            "    # Override incoming values for known iem guis.\n"
            "    set iemgui_range_header [_ $rng_header]\n"
            "    set iemgui_type [_ \"Int Number\"]\n"
            "    set wdt_label [_ \"Width (digits):\"]\n"
            "    set hgt_label [_ \"Font Size:\"]\n"
            "    set iemgui_range_header [_ \"Output Range\"]\n"
            "    set min_rng_label [_ \"Lower:\"]\n"
            "    set max_rng_label [_ \"Upper:\"]\n"
            "\n"
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
            "    # range\n"
            "    labelframe $mytoplevel.rng\n"
            "    pack $mytoplevel.rng -side top -fill x\n"
            "    frame $mytoplevel.rng.min\n"
            "    label $mytoplevel.rng.min.lab -text [_ $min_rng_label]\n"
            "    entry $mytoplevel.rng.min.ent -textvariable $var_iemgui_min_rng -width 7\n"
            "    label $mytoplevel.rng.dummy1 -text \"\" -width 1\n"
            "    label $mytoplevel.rng.max_lab -text [_ $max_rng_label]\n"
            "    entry $mytoplevel.rng.max_ent -textvariable $var_iemgui_max_rng -width 7\n"
            "    if { $rng_header ne \"empty\" } {\n"
            "        $mytoplevel.rng config -borderwidth 1 -pady 4 -text [_ $iemgui_range_header]\n"
            "        if { $min_rng_label ne \"empty\" } {\n"
            "            pack $mytoplevel.rng.min\n"
            "            pack $mytoplevel.rng.min.lab $mytoplevel.rng.min.ent -side left }\n"
            "        if { $max_rng_label ne \"empty\" } {\n"
            "            $mytoplevel.rng config -padx 26\n"
            "            pack configure $mytoplevel.rng.min -side left\n"
            "            pack $mytoplevel.rng.dummy1 $mytoplevel.rng.max_lab $mytoplevel.rng.max_ent -side left}\n"
            "    }\n"
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
            "    # get the current font name \n"
            "    set current_font $::font_family\n"
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
            "        $var_iemgui_l2_f1_b0 -text [_ \"Text\"] -justify left\n"
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
            "        bind $mytoplevel.dim.h_ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.rng.min.ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.rng.max_ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.s_r.send.ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "        bind $mytoplevel.s_r.receive.ent <KeyPress-Return> \"::dialog_number::apply_and_rebind_return $mytoplevel\"\n"
            "\n"
            "        # unbind Return from ok button when an entry takes focus\n"
            "        $mytoplevel.dim.w_ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.dim.h_ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.rng.min.ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.rng.max_ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.s_r.send.ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
            "        $mytoplevel.s_r.receive.ent config -validate focusin -vcmd \"::dialog_number::unbind_return $mytoplevel\"\n"
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
