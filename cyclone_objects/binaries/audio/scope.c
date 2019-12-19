/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER cache gui commands */
/* LATER think about resizing scheme.  Currently mouse events are not bound
 to any part of Scope~'s 'widget' as such, but to a special item, which is
 created only for a selected Scope~.  For the other scheme see the 'comment'
 class (no indicator there, though -- neither a handle, nor a pointer change).
 One way or the other, the traffic from the gui layer should be kept possibly
 low, at least in run-mode. */

/* 2016 = haven't finished cleaning out old dependencies
 (common/grow, common/loud, common/fitter, unstable/forky)
 but methods (bufsize, period/calccount, range, delay, trigger, triglevel, frgb, brgb)
 for setting attributes are rewritten, as well as attr declaration.
 Have written the color version of the colorsetting methods that take vals 0-1
 instead of 0-255
 - Derek Kwan

2017 = Porres finished cleaning "sickle/sic, loud, fitter & forky" dependencies
2019 = Porres cleaned up the code a little and fixed a regression bung  */

#include "m_pd.h"
#include <common/api.h>
#include "g_canvas.h"
#include "g_all_guis.h"
#include "common/magicbit.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SCOPE_DEFWIDTH      130
#define SCOPE_MINWIDTH      10
#define SCOPE_DEFHEIGHT     130
#define SCOPE_MINHEIGHT     10
#define SCOPE_DEFPERIOD     256
#define SCOPE_MINPERIOD     2
#define SCOPE_MAXPERIOD     8192
#define SCOPE_DEFBUFSIZE    128
#define SCOPE_MINBUFSIZE    8
#define SCOPE_MAXBUFSIZE    256
#define SCOPE_WARNBUFSIZE   256
#define SCOPE_DEFMINVAL     -1.
#define SCOPE_DEFMAXVAL     1.
#define SCOPE_DEFDELAY      0
#define SCOPE_MINDELAY      0
#define SCOPE_DEFDRAWSTYLE  0
#define SCOPE_TRIGLINEMODE  0
#define SCOPE_TRIGUPMODE    1
#define SCOPE_TRIGDOWNMODE  2
#define SCOPE_DEFTRIGMODE   SCOPE_TRIGLINEMODE
#define SCOPE_MINTRIGMODE   SCOPE_TRIGLINEMODE
#define SCOPE_MAXTRIGMODE   SCOPE_TRIGDOWNMODE
#define SCOPE_DEFTRIGLEVEL  0.
#define SCOPE_MINRGB        0
#define SCOPE_MAXRGB        255
#define SCOPE_MINCOLOR      0.
#define SCOPE_MAXCOLOR      1.
#define SCOPE_DEFFGRED      205
#define SCOPE_DEFFGGREEN    229
#define SCOPE_DEFFGBLUE     232
#define SCOPE_DEFBGRED      74
#define SCOPE_DEFBGGREEN    79
#define SCOPE_DEFBGBLUE     77
#define SCOPE_DEFGRRED      96
#define SCOPE_DEFGRGREEN    98
#define SCOPE_DEFGRBLUE     102
#define SCOPE_SELCOLOR      "#4a4f4d"  // a bit lighter shade of blue
#define SCOPE_SELBORDER     "#5aadef" // border select color that seems to be in max
#define SCOPE_FGWIDTH        0.7  // line width is float
#define SCOPE_GRIDWIDTH      0.9
#define SCOPE_SELBDWIDTH     3.0
#define SCOPEHANDLE_WIDTH    10    // item size is int
#define SCOPEHANDLE_HEIGHT   10
#define SCOPE_GUICHUNK       128 // performance-related hacks, LATER investigate

typedef struct _scope
{
    t_object        x_obj;
    t_inlet        *x_rightinlet;
    t_glist        *x_glist;
    t_canvas       *x_canvas;
    char            x_tag[64];
    char            x_fgtag[64];
    char            x_bgtag[64];
    char            x_gridtag[64];
    char            x_margintag[64];
    int             x_width;
    int             x_height;
    float           x_minval;
    float           x_maxval;
    int             x_delay;
    int             x_trigmode;
    float           x_triglevel;
    int             x_drawstyle;
    unsigned char   x_fgrgb[3];
    unsigned char   x_bgrgb[3];
    unsigned char   x_grrgb[3];
    int             x_xymode;
    int             x_lastxymode;
    float           x_xbuffer[SCOPE_MAXBUFSIZE*4];
    float           x_ybuffer[SCOPE_MAXBUFSIZE*4];
    float           x_xbuflast[SCOPE_MAXBUFSIZE*4];
    float           x_ybuflast[SCOPE_MAXBUFSIZE*4];
    t_float        *x_signalscalar;
    int             x_allocsize;
    int             x_bufsize;
    int             x_lastbufsize;
    int             x_bufphase;
    int             x_period;
    int             x_phase;
    int             x_precount;
    int             x_retrigger;
    float           x_ksr;
    float           x_currx;
    float           x_curry;
    float           x_trigx;
    int             x_frozen;
    int             x_init;
    t_clock        *x_clock;
    t_pd           *x_handle;
} t_scope;

typedef struct _scopehandle
{
    t_pd       h_pd;
    t_scope   *h_master;
    t_symbol  *h_bindsym;
    char       h_pathname[64];
    char       h_outlinetag[64];
    int        h_dragon;
    int        h_dragx;
    int        h_dragy;
} t_scopehandle;

static t_class *scope_class;
static t_class *scopehandle_class;
static void scope_bufsize(t_scope *x, t_float bufsz);

static void scope_clear(t_scope *x, int withdelay)
{
    //x->x_bufphase = 0;
    //x->x_phase = 0;
    x->x_precount = (withdelay ? (int)(x->x_delay * x->x_ksr) : 0);
    //x->x_retrigger = (x->x_trigmode != SCOPE_TRIGLINEMODE);
    //x->x_trigx = x->x_triglevel;
}

static t_int *scope_perform(t_int *w)
{
    t_scope *x = (t_scope *)(w[1]);
    int xymode = x->x_xymode;
    if (!xymode)
        return (w + 5);
    int bufphase = x->x_bufphase;
    int bufsize = (int)*x->x_signalscalar;
    if (bufsize != x->x_bufsize)
    {
        scope_bufsize(x, bufsize);
        bufsize = x->x_bufsize;
    }
    if (bufphase < bufsize)
    {
        int nblock = (int)(w[2]);
        if (x->x_precount >= nblock)
            x->x_precount -= nblock;
        else
        {
            t_float *in1 = (t_float *)(w[3]);
            t_float *in2 = (t_float *)(w[4]);
            t_float *in;
            int phase = x->x_phase;
            int period = x->x_period;
            float freq = 1. / period;
            float *bp1;
            float *bp2;
            float currx = x->x_currx;
            float curry = x->x_curry;
            if (x->x_precount > 0)
            {
                nblock -= x->x_precount;
                in1 += x->x_precount;
                in2 += x->x_precount;
                phase = 0;
                bufphase = 0;
                x->x_precount = 0;
            }
            if (x->x_trigmode && (xymode == 1 || xymode == 2))
            {
                if (xymode == 1) in = in1;
                else in = in2;
                while (x->x_retrigger)
                {
                    float triglevel = x->x_triglevel;
                    if (x->x_trigmode == SCOPE_TRIGUPMODE)
                    {
                        if (x->x_trigx < triglevel)
                        {
                            while (nblock--) if (*in >= triglevel)
                            {
                                x->x_retrigger = 0;
                                phase = 0;
                                bufphase = 0;
                                break;
                            }
                            else in++;
                        }
                        else while (nblock--) if (*in++ < triglevel)
                        {
                            x->x_trigx = triglevel - 1.;
                            break;
                        }
                    }
                    else
                    {
                        if (x->x_trigx > triglevel)
                        {
                            while (nblock--) if (*in <= triglevel)
                            {
                                phase = 0;
                                bufphase = 0;
                                x->x_retrigger = 0;
                                break;
                            }
                            else in++;
                        }
                        else while (nblock--) if (*in++ > triglevel)
                        {
                            x->x_trigx = triglevel + 1.;
                            break;
                        }
                    }
                    if (nblock <= 0)
                    {
                        x->x_bufphase = bufphase;
                        x->x_phase = phase;
                        return (w + 5);
                    }
                }
                if (xymode == 1) in1 = in;
                else in2 = in;
            }
            else if (x->x_retrigger) x->x_retrigger = 0;
            while (nblock--)
            {
                bp1 = x->x_xbuffer + bufphase;
                bp2 = x->x_ybuffer + bufphase;
                if (phase)
                {
                    t_float f1 = *in1++;
                    t_float f2 = *in2++;
                    if (xymode == 1)
                    {
                        /* CHECKED */
                        if (!x->x_drawstyle)
                        {
                            if ((currx < 0 && (f1 < currx || f1 > -currx)) ||
                                (currx > 0 && (f1 > currx || f1 < -currx)))
                                currx = f1;
                        }
                        else
                        {
                            if (f1 < currx) currx = f1;
                        }
                        curry = 0.;
                    }
                    else if (xymode == 2)
                    {
                        if (!x->x_drawstyle)
                        {
                            if ((curry < 0 && (f2 < curry || f2 > -curry)) ||
                                (curry > 0 && (f2 > curry || f2 < -curry)))
                                curry = f2;
                        }
                        else
                        {
                            if (f2 < curry) curry = f2;
                        }
                        currx = 0.;
                    }
                    else
                    {
                        currx += f1;
                        curry += f2;
                    }
                }
                else
                {
                    currx = *in1++;
                    curry = *in2++;
                }
                if (currx != currx)
                    currx = 0.;  /* CHECKED NaNs bashed to zeros */
                if (curry != curry)
                    curry = 0.;
                if (++phase >= period)
                {
                    phase = 0;
                    if (xymode == 3)
                    {
                        currx *= freq;
                        curry *= freq;
                    }
                    if (++bufphase >= bufsize)
                    {
                        *bp1 = currx;
                        *bp2 = curry;
                        bufphase = 0;
                        x->x_lastxymode = xymode;
                        x->x_lastbufsize = bufsize;
                        memcpy(x->x_xbuflast, x->x_xbuffer,
                               bufsize * sizeof(*x->x_xbuffer));
                        memcpy(x->x_ybuflast, x->x_ybuffer,
                               bufsize * sizeof(*x->x_ybuffer));
                        x->x_retrigger = (x->x_trigmode != SCOPE_TRIGLINEMODE);
                        x->x_trigx = x->x_triglevel;
                        clock_delay(x->x_clock, 0);
                        //break;
                    }
                    else
                    {
                        *bp1 = currx;
                        *bp2 = curry;
                    }
                }
            }
            x->x_currx = currx;
            x->x_curry = curry;
            x->x_bufphase = bufphase;
            x->x_phase = phase;
        }
    }
    return (w + 5);
}

static void scope_setxymode(t_scope *x, int xymode);

static void scope_dsp(t_scope *x, t_signal **sp)
{
    x->x_ksr = sp[0]->s_sr * 0.001;
    int xfeeder, yfeeder;
    xfeeder = magic_inlet_connection((t_object *)x, x->x_glist, 0, &s_signal);
    yfeeder = magic_inlet_connection((t_object *)x, x->x_glist, 1, &s_signal);
    if(!x->x_init){
        x->x_init = 1;
        x->x_lastxymode = xfeeder + 2 * yfeeder;
    }
    scope_setxymode(x, xfeeder + 2 * yfeeder);
    dsp_add(scope_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void scope_period(t_scope *x, t_float per)
{
    if (per < SCOPE_MINPERIOD)
        per = SCOPE_MINPERIOD;
    else if (per > SCOPE_MAXPERIOD)
        per = SCOPE_MAXPERIOD;
    if (x->x_period != (int)per)
    {
        x->x_period = (int)per;
        x->x_phase = 0;
        x->x_bufphase = 0;
        scope_clear(x, 0);
    }
}

static void scope_float(t_scope *x, t_float f)
{
    scope_period(x, f);
}

static void scope_bufsize(t_scope *x, t_float bufsz)
{
    if (bufsz < SCOPE_MINBUFSIZE)
        x->x_bufsize = SCOPE_MINBUFSIZE;
    else if (bufsz > SCOPE_MAXBUFSIZE)
        x->x_bufsize = SCOPE_MAXBUFSIZE;
    else
        x->x_bufsize = bufsz;
    pd_float((t_pd *)x->x_rightinlet, x->x_bufsize);
    x->x_phase = 0;
    x->x_bufphase = 0;
    scope_clear(x, 0);
}

static void scope_range(t_scope *x, t_float min, t_float max)
{
    t_float minval = min;
    t_float maxval = max;
    if (minval < maxval) // swapping, ignoring if equal
    {
        x->x_minval = minval;
        x->x_maxval = maxval;
    }
    else
    {
        x->x_minval = maxval;
        x->x_maxval = minval;
    }
}

static void scope_delay(t_scope *x, t_float del)
{
    if (del < SCOPE_MINDELAY)
        del = SCOPE_MINDELAY;
    x->x_delay = del;
}

static void scope_drawstyle(t_scope *x, t_float drawstyle)
{
    x->x_drawstyle = (drawstyle == 0 ? 0 : 1);
}

static void scope_trigger(t_scope *x, t_float trig)
{
    float trigmode;
    if (trig < SCOPE_MINTRIGMODE)
        trigmode = SCOPE_MINTRIGMODE;
    else if (trig > SCOPE_MAXTRIGMODE)
        trigmode = SCOPE_MAXTRIGMODE;
    else
        trigmode = trig;
    x->x_trigmode = (int)trigmode;
    if (x->x_trigmode == SCOPE_TRIGLINEMODE)
        x->x_retrigger = 0;
}

static void scope_triglevel(t_scope *x, t_float lvl)
{
    x->x_triglevel = lvl;
}

static void scope_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_scope *x = (t_scope *)z;
    float x1, y1, x2, y2;
    x1 = text_xpix((t_text *)x, glist);
    y1 = text_ypix((t_text *)x, glist);
    x2 = x1 + x->x_width;
    y2 = y1 + x->x_height;
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

/* #ifdef PDL2ORK // begin purr data GUI code
 
 static t_canvas *scope_getcanvas(t_scope *x, t_glist *glist)
 {
     if (glist != x->x_glist)
     {
         pd_error(x, "scope~: list needs to only contain floats");
         x->x_glist = glist;
     }
     return (x->x_canvas = glist_getcanvas(glist));
 }
 
 static t_canvas *scope_isvisible(t_scope *x) // answers: "can we draw and where?"
 {
     return (glist_isvisible(x->x_canvas) ? x->x_canvas : 0);
 }

static unsigned char scope_color_f2c(t_float f, int oldstyle)
{
    if (oldstyle)
    {
        // f is 0.0...255.0
        unsigned char c = (unsigned char)f;
        if (c < SCOPE_MINRGB) c = SCOPE_MINRGB;
        else if (c > SCOPE_MAXRGB) c = SCOPE_MAXRGB;
        return c;
    }
    else
    {
        // f is 0.0...1.0
        if (f < SCOPE_MINCOLOR) f = SCOPE_MINCOLOR;
        else if (f > SCOPE_MAXCOLOR) f = SCOPE_MAXCOLOR;
        return (unsigned char)round(f * (float)SCOPE_MAXRGB);
    }
}

static void scope_do_setrgb(unsigned char *dest, t_float r, t_float g,
                            t_float b, int oldstyle)
{
    // clip to 0-1, scale to 0-255 then round
    dest[0] = scope_color_f2c(r, oldstyle);
    dest[1] = scope_color_f2c(g, oldstyle);
    dest[2] = scope_color_f2c(b, oldstyle);
}

static void scope_update_color(t_scope *x, char *layer, unsigned char *rgb)
{
    t_canvas *cv = scope_isvisible(x);
    if (cv)
    {
        char colbuf[8];
        sprintf(colbuf, "#%2.2x%2.2x%2.2x", rgb[0], rgb[1], rgb[2]);
        gui_vmess("gui_scope_configure_color", "xxss",
                  cv, x, layer, colbuf);
    }
}

static void scope_setrgb(unsigned char *dest, t_float r, t_float g, t_float b)
{
    scope_do_setrgb(dest, r, g, b, 0);
}

static void scope_fgcolor(t_scope *x, t_float r, t_float g, t_float b)
{
    scope_setrgb(x->x_fgrgb, r, g, b);
    scope_update_color(x, "fg", x->x_fgrgb);
}

static void scope_frgb(t_scope *x, t_float r, t_float g, t_float b)
{
    scope_do_setrgb(x->x_fgrgb, r, g, b, 1);
    scope_update_color(x, "fg", x->x_fgrgb);
}

static void scope_bgcolor(t_scope *x, t_float r, t_float g, t_float b)
{
    scope_setrgb(x->x_bgrgb, r, g, b);
    scope_update_color(x, "bg", x->x_bgrgb);
}

static void scope_brgb(t_scope *x, t_float r, t_float g, t_float b)
{
    scope_do_setrgb(x->x_bgrgb, r, g, b, 1);
    scope_update_color(x, "bg", x->x_bgrgb);
}

static void scope_gridcolor(t_scope *x, t_float r, t_float g, t_float b)
{
    scope_setrgb(x->x_grrgb, r, g, b);
    scope_update_color(x, "grid", x->x_grrgb);
}

static void scope_grgb(t_scope *x, t_float r, t_float g, t_float b)
{
    scope_do_setrgb(x->x_grrgb, r, g, b, 1);
    scope_update_color(x, "grid", x->x_grrgb);
}

static void scope_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)z;
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
    {
        t_canvas *cv = scope_getcanvas(x, glist);
        gui_vmess("gui_text_displace", "xxii",
                  cv,
                  x,
                  dx,
                  dy);
        canvas_fixlinesfor(cv, t);
    }
}

static void scope_displace_wtag(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)z;
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
    {
        t_canvas *cv = scope_getcanvas(x, glist);
        canvas_fixlinesfor(cv, t);
    }
}

static void scope_select(t_gobj *z, t_glist *glist, int state)
{
    t_scope *x = (t_scope *)z;
    t_canvas *cv = scope_getcanvas(x, glist);
    t_scopehandle *sh = (t_scopehandle *)x->x_handle;
    if (state)
        gui_vmess("gui_gobj_select", "xx", cv, x);
    else
        gui_vmess("gui_gobj_deselect", "xx", cv, x);
}

static void scope_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void scope_drawfg(t_scope *x, t_canvas *cv,
                         int x1, int y1, int x2, int y2)
{
    int i, xymode = x->x_lastxymode;
    float dx, dy, xx = 0, yy = 0, oldx, oldy, sc, xsc, ysc;
    float *xbp = x->x_xbuflast, *ybp = x->x_ybuflast;
    int bufsize = x->x_lastbufsize;
    if (xymode == 1)
    {
        dx = (float)(x2 - x1) / (float)bufsize;
        oldx = x1;
        sc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 2)
    {
        dy = (float)(y2 - y1) / (float)bufsize;
        oldy = y1;
        sc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 3)
    {
        xsc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
        ysc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    // Not sure whether we need the conditional here or not...
    if (x->x_bufsize)
    {
        gui_start_vmess("gui_scope_configure_fg_xy", "xx", cv, x);
        gui_start_array();
    }
    for (i = 0; i < bufsize; i++)
    {
        if (xymode == 1)
        {
            xx = oldx;
            yy = (y2 - 1) - sc * (*xbp++ - x->x_minval);
            if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
            oldx += dx;
        }
        else if (xymode == 2)
        {
            yy = oldy;
            xx = (x2 - 1) - sc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            oldy += dy;
        }
        else if (xymode == 3)
        {
            xx = x1 + xsc * (*xbp++ - x->x_minval);
            yy = y2 - ysc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
        }
        if (i == 0) gui_s("M");
        gui_i((int)xx);
        gui_i((int)yy);
    }
    if (x->x_bufsize)
    {
        gui_end_array();
        gui_end_vmess();
    }
}

static void scope_drawmargins(t_scope *x, t_canvas *cv,
                              int x1, int y1, int x2, int y2)
{
    // for Purr Data this is handled from scope_drawbg
}

static void scope_drawbg(t_scope *x, t_canvas *cv,
                         int x1, int y1, int x2, int y2)
{
    int i;
    float dx, dy, xx, yy;
    char fgcolor[20];
    char bgcolor[20];
    dx = (x2 - x1) * 0.125;
    dy = (y2 - y1) * 0.25;
    sprintf(fgcolor, "#%2.2x%2.2x%2.2x",
            x->x_fgrgb[0], x->x_fgrgb[1], x->x_fgrgb[2]);
    sprintf(bgcolor, "#%2.2x%2.2x%2.2x",
            x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2]);
    // Purr data; create all elements of the scope
    // here. scope_drawfg can then just change the path
    // data of the fg path element (instead of destroying
    // and recreating it
    gui_vmess("gui_scope_draw_bg", "xxssiifff",
              glist_getcanvas(cv),
              x,
              fgcolor,
              bgcolor,
              x2 - x1,
              y2 - y1,
              SCOPE_GRIDWIDTH,
              dx,
              dy);
}

static void scope_draw(t_scope *x, t_canvas *cv)
{
    int x1, y1, x2, y2;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    scope_drawbg(x, cv, x1, y1, x2, y2);
    if (x->x_lastxymode)
        scope_drawfg(x, cv, x1, y1, x2, y2);
    //scope_drawmargins(x, cv, x1, y1, x2, y2);
}

static void scope_redraw(t_scope *x, t_canvas *cv)
{
    int bufsize;
    int nleft = bufsize = x->x_lastbufsize;
    //float *bp = x->x_xbuflast;
    char chunk[32 * SCOPE_GUICHUNK];  // LATER estimate
    char *chunkp = chunk;
    
    int x1, y1, x2, y2, xymode = x->x_lastxymode;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    
    float dx, dy, xx, yy, oldx, oldy, sc, xsc, ysc;
    float *xbp = x->x_xbuflast, *ybp = x->x_ybuflast;
    if (xymode == 1)
    {
        dx = (float)(x2 - x1) / (float)bufsize;
        oldx = x1;
        sc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 2)
    {
        dy = (float)(y2 - y1) / (float)bufsize;
        oldy = y1;
        sc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 3)
    {
        xsc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
        ysc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    // Not sure whether we need the conditional here or not... 
    if (x->x_bufsize)
    {
        gui_start_vmess("gui_scope_configure_fg_xy", "xx", cv, x);
        gui_start_array();
        gui_s("M");
    }
    while (nleft > SCOPE_GUICHUNK)
    {
        int i = SCOPE_GUICHUNK;
        while (i--)
        {
            if (xymode == 1)
            {
                xx = oldx;
                yy = (y2 - 1) - sc * (*xbp++ - x->x_minval);
                if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
                oldx += dx;
            }
            else if (xymode == 2)
            {
                yy = oldy;
                xx = (x2 - 1) - sc * (*ybp++ - x->x_minval);
                if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
                oldy += dy;
            }
            else if (xymode == 3)
            {
                xx = x1 + xsc * (*xbp++ - x->x_minval);
                yy = y2 - ysc * (*ybp++ - x->x_minval);
                if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
                if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
            }
            sprintf(chunkp, "%d %d ", (int)xx - x1, (int)yy - y1);
            chunkp += strlen(chunkp);
        }
        //strcpy(chunkp, "\\\n");
        //sys_gui(chunk);
        gui_s(chunk);
        chunkp = chunk;
        nleft -= SCOPE_GUICHUNK;
    }
    while (nleft--)
    {
        if (xymode == 1)
        {
            xx = oldx;
            yy = (y2 - 1) - sc * (*xbp++ - x->x_minval);
            if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
            oldx += dx;
        }
        else if (xymode == 2)
        {
            yy = oldy;
            xx = (x2 - 1) - sc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            oldy += dy;
        }
        else if (xymode == 3)
        {
            xx = x1 + xsc * (*xbp++ - x->x_minval);
            yy = y2 - ysc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
        }
        sprintf(chunkp, "%d %d ", (int)xx - x1, (int)yy - y1);
        chunkp += strlen(chunkp);
    }
    //strcpy(chunkp, "\n");
    //sys_gui(chunk);
    gui_s(chunk);
    gui_end_array();
    gui_end_vmess();
}

static void scope_revis(t_scope *x, t_canvas *cv)
{
    gui_vmess("gui_scope_erase_innards", "xx", cv, x);
    scope_draw(x, cv);
}

static void scope_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)z;
    t_canvas *cv = scope_getcanvas(x, glist);
    if (vis)
    {
        t_scopehandle *sh = (t_scopehandle *)x->x_handle;
        sprintf(sh->h_pathname, ".x%lx.h%lx",
                (unsigned long)cv, (unsigned long)sh);
        int x1, y1, x2, y2;
        int xymode = x->x_xymode;
        int bufsize = x->x_bufsize;
        x->x_xymode = x->x_lastxymode;
        x->x_bufsize = x->x_lastbufsize;
        scope_getrect(z, glist, &x1, &y1, &x2, &y2);
        gui_vmess("gui_gobj_new", "xxsiii",
                  glist_getcanvas(glist),
                  x,
                  "obj",
                  x1,
                  y1,
                  glist_istoplevel(glist));
        scope_draw(x, cv);
        x->x_xymode = xymode;
        x->x_bufsize = bufsize;
        if (glist_isselected(cv, (t_gobj *)x))
            gui_vmess("gui_gobj_select", "xx", cv, x);
    }
    else
    {
        gui_vmess("gui_gobj_erase", "xx", glist_getcanvas(glist), x);
        x->x_canvas = 0;
    }
}

static void scope_motion(t_scope *x, t_floatarg dx, t_floatarg dy)
{
    
}

static int scope_click(t_gobj *z, t_glist *glist,
                       int xpix, int ypix, int shift, int alt, int dbl,
                       int doit)
{
    t_scope *x = (t_scope *)z;
    if (doit)
    {
        x->x_frozen = 1;
        glist_grab(x->x_glist, &x->x_obj.te_g, (t_glistmotionfn)scope_motion,
                   0, xpix, ypix);
    }
    else x->x_frozen = 0;
    return (CURSOR_RUNMODE_CLICKME);
}

// CHECKED there's only a copy of state variables, whether modified via messages or inspector
static void scope_save(t_gobj *z, t_binbuf *b)
{
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)x;
    binbuf_addv(b, "ssiisiiiiiffififiiiiiiiiii;", gensym("#X"), gensym("obj"),
                (int)t->te_xpix, (int)t->te_ypix,
                atom_getsymbol(binbuf_getvec(t->te_binbuf)),
                x->x_width, x->x_height, x->x_period, 3, x->x_bufsize,
                x->x_minval, x->x_maxval, x->x_delay, 0.,
                x->x_trigmode, x->x_triglevel,
                x->x_fgrgb[0], x->x_fgrgb[1], x->x_fgrgb[2],
                x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2],
                x->x_grrgb[0], x->x_grrgb[1], x->x_grrgb[2], 0);
}

static t_widgetbehavior scope_widgetbehavior =
{
    scope_getrect,
    scope_displace,
    scope_select,
    0,
    scope_delete,
    scope_vis,
    scope_click,
    0,0 // instead of FORKY_WIDGETPADDING
};

static void scope_setxymode(t_scope *x, int xymode)
{
    if (xymode != x->x_xymode)
    {
        t_canvas *cv;
        if ((cv = scope_isvisible(x)))
        {
            int x1, y1, x2, y2;
            scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
            if (xymode)
                scope_drawfg(x, cv, x1, y1, x2, y2);
        }
        x->x_xymode = xymode;
        scope_clear(x, 0);
    }
}

static void scope_tick(t_scope *x)
{
    t_canvas *cv = scope_isvisible(x);
    if (cv)
    {
        if (!x->x_canvas->gl_editor->e_onmotion)
            x->x_frozen = 0;
    }
    if (!x->x_frozen && cv)
    {
        if (x->x_xymode)
            scope_redraw(x, cv);
    }
    scope_clear(x, 1);
}

static void scope_resize(t_scope *x, t_float w, t_float h)
{
    t_canvas *cv;
    x->x_width  = (int)(w < SCOPE_MINWIDTH ? SCOPE_MINWIDTH : w);
    x->x_height = (int)(h < SCOPE_MINHEIGHT ? SCOPE_MINHEIGHT : h);
    if ((cv = scope_isvisible(x)))
    {
        if (x->x_xymode)
            scope_redraw(x, x->x_canvas);
        scope_revis(x, x->x_canvas);
        canvas_fixlinesfor(x->x_glist, (t_text *)x);
    }
}

static void scopehandle__clickhook(t_scopehandle *sh, t_floatarg f)
{
    t_scope *x = (t_scope *)(sh->h_master);
    int newstate = (int)f;
    if (newstate)
    {
        canvas_apply_setundo(x->x_glist, (t_gobj *)x);
    }
    sh->h_dragon = newstate;
}

static void scopehandle__motionhook(t_scopehandle *sh,
                                    t_floatarg mouse_x, t_floatarg mouse_y)
{
    if (sh->h_dragon)
    {
        t_scope *x = sh->h_master;
        int x1, y1, x2, y2, width, height;
        scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
        width = mouse_x - x1;
        height = mouse_y - y1;
        x->x_width =  width < SCOPE_MINWIDTH ? SCOPE_MINWIDTH : width;
        x->x_height = height < SCOPE_MINHEIGHT ? SCOPE_MINHEIGHT : height;
        t_canvas *cv;
        if ((cv = scope_isvisible(x)))
        {
            scope_vis((t_gobj *)x, cv, 0);
            scope_vis((t_gobj *)x, cv, 1);
        }
    }
}

// wrapper method for forwarding "scalehandle" data
static void scope_click_for_resizing(t_scope *x, t_floatarg f,
                                     t_floatarg xxx, t_floatarg yyy)
{
    t_scopehandle *sh = (t_scopehandle *)x->x_handle;
    scopehandle__clickhook(sh, f);
}

// another wrapper for forwarding "scalehandle" motion data
static void scope_motion_for_resizing(t_scope *x, t_floatarg xxx,
                                      t_floatarg yyy)
{
    t_scopehandle *sh = (t_scopehandle *)x->x_handle;
    scopehandle__motionhook(sh, xxx, yyy);
}

static void scope_free(t_scope *x)
{
    if (x->x_clock)
        clock_free(x->x_clock);
    if (x->x_handle)
    {
        pd_unbind(x->x_handle, ((t_scopehandle *)x->x_handle)->h_bindsym);
        pd_free(x->x_handle);
    }
}

static void scope_dim(t_scope *x, t_float width, t_float height)
{
    if (width < SCOPE_MINWIDTH)
        width = SCOPE_MINWIDTH;
    if (height < SCOPE_MINHEIGHT)
        height = SCOPE_MINHEIGHT;
    x->x_width = (int)width;
    x->x_height = (int)height;
};

static void scope_properties(t_gobj *z, t_glist *owner)
{
    t_scope *x = (t_scope *)z;
    int bgcol, grcol, fgcol;
    char bgsym[8], grsym[8], fgsym[8];
    bgcol = ((int)x->x_bgrgb[0] << 16) + ((int)x->x_bgrgb[1] << 8) +
    (int)x->x_bgrgb[2];
    grcol = ((int)x->x_grrgb[0] << 16) + ((int)x->x_grrgb[1] << 8) +
    (int)x->x_grrgb[2];
    fgcol = ((int)x->x_fgrgb[0] << 16) + ((int)x->x_fgrgb[1] << 8) +
    (int)x->x_fgrgb[2];
    sprintf(bgsym, "#%06x", bgcol);
    sprintf(grsym, "#%06x", grcol);
    sprintf(fgsym, "#%06x", fgcol);
    char *gfx_tag = gfxstub_new2(&x->x_obj.ob_pd, x);
    gui_start_vmess("gui_external_dialog", "ss", gfx_tag, "scope~");
    gui_start_array();
    
    gui_s("int"); gui_s("width"); gui_i(x->x_width);
    gui_s("int"); gui_s("height"); gui_i(x->x_height);
    gui_s("int"); gui_s("period"); gui_i(x->x_period);
    gui_s("int"); gui_s("buffer size"); gui_i(x->x_bufsize);
    gui_s("float"); gui_s("min"); gui_f(x->x_minval);
    gui_s("float"); gui_s("max"); gui_f(x->x_maxval);
    gui_s("float"); gui_s("delay"); gui_f(x->x_delay);
    gui_s("toggle"); gui_s("style"); gui_i(x->x_drawstyle);
    
    gui_s("enum"); gui_s("trigmode"); gui_s("none");
    gui_s("enum"); gui_s("trigmode"); gui_s("up");
    gui_s("enum"); gui_s("trigmode"); gui_s("down");
    gui_s("enum_index"); gui_s("trigmode"); gui_i(x->x_trigmode);
    
    gui_s("float"); gui_s("triglevel"); gui_f(x->x_triglevel);
    gui_s("color"); gui_s("bgcolor"); gui_s(bgsym);
    gui_s("color"); gui_s("grcolor"); gui_s(grsym);
    gui_s("color"); gui_s("fgcolor"); gui_s(fgsym);
    
    gui_end_array();
    gui_end_vmess();
}

// #else // end purr data GUI code

// begin pd vanilla GUI code */

static void scope_fgcolor(t_scope *x, t_float fr, t_float fg, t_float fb)
{   //scale is 0-1
    if (fr < SCOPE_MINCOLOR)
        fr = SCOPE_MINCOLOR;
    else if (fr > SCOPE_MAXCOLOR)
        fr = SCOPE_MAXCOLOR;
    if (fg < SCOPE_MINCOLOR)
        fg = SCOPE_MINCOLOR;
    else if (fg > SCOPE_MAXCOLOR)
        fg = SCOPE_MAXCOLOR;
    if (fb < SCOPE_MINCOLOR)
        fb = SCOPE_MINCOLOR;
    else if (fb > SCOPE_MAXCOLOR)
        fb = SCOPE_MAXCOLOR;
// scaling to 255 and rounding
    fr *= (float)SCOPE_MAXRGB;
    fr = round(fr);
    fg *= (float)SCOPE_MAXRGB;
    fg = round(fg);
    fb *= (float)SCOPE_MAXRGB;
    fb = round(fb);
    x->x_fgrgb[0] = (int)fr;
    x->x_fgrgb[1] = (int)fg;
    x->x_fgrgb[2] = (int)fb;
    if(glist_isvisible(x->x_canvas))
        sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
            x->x_canvas, x->x_fgtag, x->x_fgrgb[0], x->x_fgrgb[1], x->x_fgrgb[2]);
}

static void scope_frgb(t_scope *x, t_float fr, t_float fg, t_float fb)
{   //scale is 0-255
    if (fr < SCOPE_MINRGB)
        fr = SCOPE_MINRGB;
    else if (fr > SCOPE_MAXRGB)
        fr = SCOPE_MAXRGB;
    if (fg < SCOPE_MINRGB)
        fg = SCOPE_MINRGB;
    else if (fg > SCOPE_MAXRGB)
        fg = SCOPE_MAXRGB;
    if (fb < SCOPE_MINRGB)
        fb = SCOPE_MINRGB;
    else if (fb > SCOPE_MAXRGB)
        fb = SCOPE_MAXRGB;
    x->x_fgrgb[0] = (int)fr;
    x->x_fgrgb[1] = (int)fg;
    x->x_fgrgb[2] = (int)fb;
    if(glist_isvisible(x->x_canvas))
        sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
            x->x_canvas, x->x_fgtag, x->x_fgrgb[0], x->x_fgrgb[1], x->x_fgrgb[2]);
}

static void scope_bgcolor(t_scope *x, t_float br, t_float bg, t_float bb)
{   //scale is 0-1
    if (br < SCOPE_MINCOLOR)
        br = SCOPE_MINCOLOR;
    else if (br > SCOPE_MAXCOLOR)
        br = SCOPE_MAXCOLOR;
    if (bg < SCOPE_MINCOLOR)
        bg = SCOPE_MINCOLOR;
    else if (bg > SCOPE_MAXCOLOR)
        bg = SCOPE_MAXCOLOR;
    if (bb < SCOPE_MINCOLOR)
        bb = SCOPE_MINCOLOR;
    else if (bb > SCOPE_MAXCOLOR)
        bb = SCOPE_MAXCOLOR;
// scaling to 255 and rounding
    br *= (float)SCOPE_MAXRGB;
    br = round(br);
    bg *= (float)SCOPE_MAXRGB;
    bg = round(bg);
    bb *= (float)SCOPE_MAXRGB;
    bb = round(bb);
    x->x_bgrgb[0] = (int)br;
    x->x_bgrgb[1] = (int)bg;
    x->x_bgrgb[2] = (int)bb;
    if(glist_isvisible(x->x_canvas))
        sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
                 x->x_canvas, x->x_bgtag, x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2]);
}

static void scope_brgb(t_scope *x, t_float br, t_float bg, t_float bb)
{   //scale is 0-255
    if (br < SCOPE_MINRGB)
        br = SCOPE_MINRGB;
    else if (br > SCOPE_MAXRGB)
        br = SCOPE_MAXRGB;
    if (bg < SCOPE_MINRGB)
        bg = SCOPE_MINRGB;
    else if (bg > SCOPE_MAXRGB)
        bg = SCOPE_MAXRGB;
    if (bb < SCOPE_MINRGB)
        bb = SCOPE_MINRGB;
    else if (bb > SCOPE_MAXRGB)
        bb = SCOPE_MAXRGB;
    x->x_bgrgb[0] = (int)br;
    x->x_bgrgb[1] = (int)bg;
    x->x_bgrgb[2] = (int)bb;
    if(glist_isvisible(x->x_canvas))
        sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
                 x->x_canvas, x->x_bgtag, x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2]);
}

static void scope_gridcolor(t_scope *x, t_float gr, t_float gg, t_float gb)
{   //scale is 0-1
    if (gr < SCOPE_MINCOLOR)
        gr = SCOPE_MINCOLOR;
    else if (gr > SCOPE_MAXCOLOR)
        gr = SCOPE_MAXCOLOR;
    if (gg < SCOPE_MINCOLOR)
        gg = SCOPE_MINCOLOR;
    else if (gg > SCOPE_MAXCOLOR)
        gg = SCOPE_MAXCOLOR;
    if (gb < SCOPE_MINCOLOR)
        gb = SCOPE_MINCOLOR;
    else if (gb > SCOPE_MAXCOLOR)
        gb = SCOPE_MAXCOLOR;
// scaling to 255 and rounding
    gr *= (float)SCOPE_MAXRGB;
    gr = round(gr);
    gg *= (float)SCOPE_MAXRGB;
    gg = round(gg);
    gb *= (float)SCOPE_MAXRGB;
    gb = round(gb);
    x->x_grrgb[0] = (int)gr;
    x->x_grrgb[1] = (int)gg;
    x->x_grrgb[2] = (int)gb;
    if(glist_isvisible(x->x_canvas))
        sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
            x->x_canvas, x->x_gridtag, x->x_grrgb[0], x->x_grrgb[1], x->x_grrgb[2]);
}

static void scope_grgb(t_scope *x, t_float gr, t_float gg, t_float gb)
{   //scale 0-255
    if (gr < SCOPE_MINRGB)
        gr = SCOPE_MINRGB;
    else if (gr > SCOPE_MAXRGB)
        gr = SCOPE_MAXRGB;
    if (gg < SCOPE_MINRGB)
        gg = SCOPE_MINRGB;
    else if (gg > SCOPE_MAXRGB)
        gg = SCOPE_MAXRGB;
    if (gb < SCOPE_MINRGB)
        gb = SCOPE_MINRGB;
    else if (gb > SCOPE_MAXRGB)
        gb = SCOPE_MAXRGB;
    x->x_grrgb[0]   = (int)gr;
    x->x_grrgb[1] = (int)gg;
    x->x_grrgb[2]  = (int)gb;
    if(glist_isvisible(x->x_canvas))
        sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
            x->x_canvas, x->x_gridtag, x->x_grrgb[0], x->x_grrgb[1], x->x_grrgb[2]);
}

static void scope_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)z;
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
    {
        sys_vgui(".x%lx.c move %s %d %d\n", x->x_canvas, x->x_tag, dx, dy);
        canvas_fixlinesfor(x->x_canvas, t);
    }
}

static void scope_select(t_gobj *z, t_glist *glist, int state)
{
    t_scope *x = (t_scope *)z;
    t_scopehandle *sh = (t_scopehandle *)x->x_handle;
    if (state)
    {
        int x1, y1, x2, y2;
        scope_getrect(z, glist, &x1, &y1, &x2, &y2);
        sys_vgui(".x%lx.c itemconfigure %s -outline %s "
                 "-width %f -fill #%2.2x%2.2x%2.2x\n",
                 x->x_canvas, x->x_bgtag, SCOPE_SELBORDER, SCOPE_SELBDWIDTH,
                 x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2]);
        sys_vgui("canvas %s -width %d -height %d -bg #fedc00 -bd 0\n",
                 sh->h_pathname, SCOPEHANDLE_WIDTH, SCOPEHANDLE_HEIGHT);
        sys_vgui(".x%lx.c create window %f %f -anchor nw\
                 -width %d -height %d -window %s -tags %s\n",
                 x->x_canvas, x2 - (SCOPEHANDLE_WIDTH - SCOPE_SELBDWIDTH),
                 y2 - (SCOPEHANDLE_HEIGHT - SCOPE_SELBDWIDTH),
                 SCOPEHANDLE_WIDTH, SCOPEHANDLE_HEIGHT,
                 sh->h_pathname, x->x_tag);
        sys_vgui("bind %s <Button> {pdsend [concat %s _click 1 \\;]}\n",
                 sh->h_pathname, sh->h_bindsym->s_name);
        sys_vgui("bind %s <ButtonRelease> {pdsend [concat %s _click 0 \\;]}\n",
                 sh->h_pathname, sh->h_bindsym->s_name);
        sys_vgui("bind %s <Motion> {pdsend [concat %s _motion %%x %%y \\;]}\n",
                 sh->h_pathname, sh->h_bindsym->s_name);
    }
    else
    {
        sys_vgui(".x%lx.c itemconfigure %s -outline black -width %f\
                 -fill #%2.2x%2.2x%2.2x\n", x->x_canvas, x->x_bgtag, SCOPE_GRIDWIDTH,
                 x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2]);
        sys_vgui("destroy %s\n", sh->h_pathname);
    }
}

static void scope_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void scope_drawfg(t_scope *x, t_canvas *cv, int x1, int y1, int x2, int y2)
{
    int i, xymode = x->x_lastxymode;
    float dx, dy, xx = 0, yy = 0, oldx, oldy, sc, xsc, ysc;
    float *xbp = x->x_xbuflast, *ybp = x->x_ybuflast;
    int bufsize = x->x_lastbufsize;
    if (xymode == 1)
    {
        dx = (float)(x2 - x1) / (float)bufsize;
        oldx = x1;
        sc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 2)
    {
        dy = (float)(y2 - y1) / (float)bufsize;
        oldy = y1;
        sc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 3)
    {
        xsc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
        ysc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    sys_vgui(".x%lx.c create line \\\n", cv);
    for (i = 0; i < bufsize; i++)
    {
        if (xymode == 1)
        {
            xx = oldx;
            yy = (y2 - 1) - sc * (*xbp++ - x->x_minval);
            if (yy > y2) yy = y2;
            else if (yy < y1) yy = y1;
            oldx += dx;
        }
        else if (xymode == 2)
        {
            yy = oldy;
            xx = (x2 - 1) - sc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            oldy += dy;
        }
        else if (xymode == 3)
        {
            xx = x1 + xsc * (*xbp++ - x->x_minval);
            yy = y2 - ysc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
        }
        sys_vgui("%d %d \\\n", (int)xx, (int)yy);
    }
    sys_vgui("-fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
             x->x_fgrgb[0], x->x_fgrgb[1], x->x_fgrgb[2],
             SCOPE_FGWIDTH, x->x_fgtag, x->x_tag);
}

static void scope_drawmargins(t_scope *x, t_canvas *cv, int x1, int y1, int x2, int y2)
{   // margin lines:  masking overflows, so that they appear as gaps,
    // rather than clipped signal values, LATER rethink
    sys_vgui(".x%lx.c create line %d %d\
             %d %d\
             %d %d\
             %d %d\
             %d %d\
             -fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
             cv, x1, y1 , x2, y1, x2, y2, x1, y2, x1, y1,
             x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2],
             1., x->x_margintag, x->x_tag);
}

static void scope_drawbg(t_scope *x, t_canvas *cv, int x1, int y1, int x2, int y2)
{
    int i;
    float dx, dy, xx, yy;
    dx = (x2 - x1) * 0.125;
    dy = (y2 - y1) * 0.25;
    sys_vgui(".x%lx.c create rectangle %d %d %d %d\
             -fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
             cv, x1, y1, x2, y2,
             x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2],
             SCOPE_GRIDWIDTH, x->x_bgtag, x->x_tag);
    for (i = 0, xx = x1 + dx; i < 7; i++, xx += dx)
        sys_vgui(".x%lx.c create line %f %d %f %d\
                 -width %f -tags {%s %s} -fill #%2.2x%2.2x%2.2x\n", cv, xx, y1, xx, y2,
                 SCOPE_GRIDWIDTH, x->x_gridtag, x->x_tag,
                 x->x_grrgb[0], x->x_grrgb[1], x->x_grrgb[2]);
    for (i = 0, yy = y1 + dy; i < 3; i++, yy += dy)
        sys_vgui(".x%lx.c create line %d %f %d %f\
                 -width %f -tags {%s %s} -fill #%2.2x%2.2x%2.2x\n", cv, x1, yy, x2, yy,
                 SCOPE_GRIDWIDTH, x->x_gridtag, x->x_tag,
                 x->x_grrgb[0], x->x_grrgb[1], x->x_grrgb[2]);
}

static void scope_draw(t_scope *x, t_canvas *cv)
{
    int x1, y1, x2, y2;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    scope_drawbg(x, cv, x1, y1, x2, y2);
    if (x->x_lastxymode)
        scope_drawfg(x, cv, x1, y1, x2, y2);
    scope_drawmargins(x, cv, x1, y1, x2, y2);
    //sys_vgui("bind .x%lx.c <ButtonRelease-1> {pdsend [concat %s release 0 \\;]}\n",
    //     cv, x->x_bindsym->s_name);
}

static void scope_redraw(t_scope *x, t_canvas *cv)
{
    int bufsize;
    int nleft = bufsize = x->x_lastbufsize;
    char chunk[32 * SCOPE_GUICHUNK];  // LATER estimate
    char *chunkp = chunk;
    int x1, y1, x2, y2, xymode = x->x_lastxymode;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    float dx, dy, xx, yy, oldx, oldy, sc, xsc, ysc;
    float *xbp = x->x_xbuflast, *ybp = x->x_ybuflast;
    if (xymode == 1)
    {
        dx = (float)(x2 - x1) / (float)bufsize;
        oldx = x1;
        sc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 2)
    {
        dy = (float)(y2 - y1) / (float)bufsize;
        oldy = y1;
        sc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    else if (xymode == 3)
    {
        xsc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
        ysc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    }
    sys_vgui(".x%lx.c coords %s \\\n", cv, x->x_fgtag);
    while (nleft > SCOPE_GUICHUNK)
    {
        int i = SCOPE_GUICHUNK;
        while (i--)
        {
            if (xymode == 1)
            {
                xx = oldx;
                yy = (y2 - 1) - sc * (*xbp++ - x->x_minval);
                if (yy > y2) yy = y2;
                else if (yy < y1) yy = y1;
                oldx += dx;
            }
            else if (xymode == 2)
            {
                yy = oldy;
                xx = (x2 - 1) - sc * (*ybp++ - x->x_minval);
                if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
                oldy += dy;
            }
            else if (xymode == 3)
            {
                xx = x1 + xsc * (*xbp++ - x->x_minval);
                yy = y2 - ysc * (*ybp++ - x->x_minval);
                if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
                if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
            }
            sprintf(chunkp, "%d %d ", (int)xx, (int)yy);
            chunkp += strlen(chunkp);
        }
        strcpy(chunkp, "\\\n");
        sys_gui(chunk);
        chunkp = chunk;
        nleft -= SCOPE_GUICHUNK;
    }
    while (nleft--)
    {
        if (xymode == 1)
        {
            xx = oldx;
            yy = (y2 - 1) - sc * (*xbp++ - x->x_minval);
            if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
            oldx += dx;
        }
        else if (xymode == 2)
        {
            yy = oldy;
            xx = (x2 - 1) - sc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            oldy += dy;
        }
        else if (xymode == 3)
        {
            xx = x1 + xsc * (*xbp++ - x->x_minval);
            yy = y2 - ysc * (*ybp++ - x->x_minval);
            if (xx > x2) xx = x2; else if (xx < x1) xx = x1;
            if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
        }
        sprintf(chunkp, "%d %d ", (int)xx, (int)yy);
        chunkp += strlen(chunkp);
    }
    strcpy(chunkp, "\n");
    sys_gui(chunk);
}

static void scope_revis(t_scope *x, t_canvas *cv)
{
    sys_vgui(".x%lx.c delete %s\n", cv, x->x_tag);
    scope_draw(x, cv);
}

static void scope_vis(t_gobj *z, t_glist *glist, int vis)
{
    glist = NULL;
    t_scope *x = (t_scope *)z;
    if(vis){
        t_scopehandle *sh = (t_scopehandle *)x->x_handle;
        sprintf(sh->h_pathname, ".x%lx.h%lx", (unsigned long)x->x_canvas, (unsigned long)sh);
        int xymode = x->x_xymode;
        int bufsize = x->x_bufsize;
        x->x_xymode = x->x_lastxymode;
        x->x_bufsize = x->x_lastbufsize;
        scope_draw(x, x->x_canvas);
        x->x_xymode = xymode;
        x->x_bufsize = bufsize;
    }
    else
        sys_vgui(".x%lx.c delete %s\n", (unsigned long)x->x_canvas, x->x_tag);
}

static void scope_motion(void){ // dummy
}

static int scope_click(t_gobj *z, t_glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_scope *x = (t_scope *)z;
    if (doit){
        x->x_frozen = 1;
        glist_grab(x->x_glist, &x->x_obj.te_g, (t_glistmotionfn)scope_motion,
                   0, xpix, ypix);
    }
    else
        x->x_frozen = 0;
    glist = NULL;
    shift = alt = dbl = 0;
    return (CURSOR_RUNMODE_CLICKME);
}

static void scope_save(t_gobj *z, t_binbuf *b){
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)x;
    binbuf_addv(b, "ssiisiiiiiffififiiiiiiiiii;", gensym("#X"), gensym("obj"),
                (int)t->te_xpix, (int)t->te_ypix,
                atom_getsymbol(binbuf_getvec(t->te_binbuf)),
                x->x_width, x->x_height, x->x_period, 3, x->x_bufsize,
                x->x_minval, x->x_maxval, x->x_delay, 0.,
                x->x_trigmode, x->x_triglevel,
                x->x_fgrgb[0], x->x_fgrgb[1], x->x_fgrgb[2],
                x->x_bgrgb[0], x->x_bgrgb[1], x->x_bgrgb[2],
                x->x_grrgb[0], x->x_grrgb[1], x->x_grrgb[2], 0);
}

static t_widgetbehavior scope_widgetbehavior =
{
    scope_getrect,
    scope_displace,
    scope_select,
    0, // activate
    scope_delete,
    scope_vis,
    scope_click,
};

static void scope_setxymode(t_scope *x, int xymode)
{
    if(xymode != x->x_xymode){
        if(glist_isvisible(x->x_canvas)){
            sys_vgui(".x%lx.c delete %s %s\n", x->x_canvas, x->x_fgtag, x->x_margintag);
            int x1, y1, x2, y2;
            scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
            if(xymode)
                scope_drawfg(x, x->x_canvas, x1, y1, x2, y2);
            scope_drawmargins(x, x->x_canvas, x1, y1, x2, y2);
        }
        x->x_xymode = xymode;
        scope_clear(x, 0);
    }
}

static void scope_tick(t_scope *x)
{
    int vis = glist_isvisible(x->x_canvas);
    if(vis && !x->x_canvas->gl_editor->e_onmotion)
        x->x_frozen = 0;
    if(vis && !x->x_frozen && x->x_xymode)
        scope_redraw(x, x->x_canvas);
    scope_clear(x, 1);
}

static void scope_resize(t_scope *x, t_float w, t_float h)
{
    x->x_width  = (int)(w < SCOPE_MINWIDTH ? SCOPE_MINWIDTH : w);
    x->x_height = (int)(h < SCOPE_MINHEIGHT ? SCOPE_MINHEIGHT : h);
    if(glist_isvisible(x->x_canvas)){
        if(x->x_xymode)
            scope_redraw(x, x->x_canvas);
        scope_revis(x, x->x_canvas);
        canvas_fixlinesfor(x->x_glist, (t_text *)x);
    }
}

static void scopehandle__clickhook(t_scopehandle *sh, t_floatarg f)
{
    int newstate = (int)f;
    if(sh->h_dragon && newstate == 0){
        t_scope *x = sh->h_master;
        x->x_width += sh->h_dragx;
        x->x_height += sh->h_dragy;
        if(glist_isvisible(x->x_canvas)){
            sys_vgui(".x%lx.c delete %s\n",  x->x_canvas, sh->h_outlinetag);
            scope_revis(x, x->x_canvas);
            sys_vgui("destroy %s\n", sh->h_pathname);
            scope_select((t_gobj *)x, x->x_glist, 1);
            canvas_fixlinesfor(x->x_glist, (t_text *)x);  // 2nd inlet
        }
    }
    else if(!sh->h_dragon && newstate){
        t_scope *x = sh->h_master;
        if(glist_isvisible(x->x_canvas)){
            int x1, y1, x2, y2;
            scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
            sys_vgui("lower %s\n", sh->h_pathname);
            sys_vgui(".x%lx.c create rectangle %d %d %d %d\
                     -outline %s -width %f -tags %s\n",
                     x->x_canvas, x1, y1, x2, y2,
                     SCOPE_SELBORDER, SCOPE_SELBDWIDTH, sh->h_outlinetag);
        }
        sh->h_dragx = 0;
        sh->h_dragy = 0;
    }
    sh->h_dragon = newstate;
}

static void scopehandle__motionhook(t_scopehandle *sh, t_floatarg f1, t_floatarg f2)
{
    if (sh->h_dragon){
        t_scope *x = sh->h_master;
        int dx = (int)f1, dy = (int)f2;
        int x1, y1, x2, y2, newx, newy;
        scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
        newx = x2 + dx;
        newy = y2 + dy;
        if (newx > x1 + SCOPE_MINWIDTH && newy > y1 + SCOPE_MINHEIGHT){
            if(glist_isvisible(x->x_canvas))
                sys_vgui(".x%lx.c coords %s %d %d %d %d\n", x->x_canvas, sh->h_outlinetag,
                    x1, y1, newx, newy);
            sh->h_dragx = dx;
            sh->h_dragy = dy;
        }
    }
}

static void scope_free(t_scope *x)
{
    if (x->x_clock)
        clock_free(x->x_clock);
    if (x->x_handle){
        pd_unbind(x->x_handle, ((t_scopehandle *)x->x_handle)->h_bindsym);
        pd_free(x->x_handle);
    }
}

static void scope_dim(t_scope *x, t_float width, t_float height)
{
    if (width < SCOPE_MINWIDTH)
        width = SCOPE_MINWIDTH;
    if (height < SCOPE_MINHEIGHT)
        height = SCOPE_MINHEIGHT;
    x->x_width = (int) width;
    x->x_height = (int)height;
}

static void scope_properties(t_gobj *z, t_glist *owner)
{
    owner = NULL;
    t_scope *x = (t_scope *)z;
    int bgcol, grcol, fgcol;
    bgcol = ((int)x->x_bgrgb[0] << 16) + ((int)x->x_bgrgb[1] << 8) + (int)x->x_bgrgb[2];
    grcol = ((int)x->x_grrgb[0] << 16) + ((int)x->x_grrgb[1] << 8) + (int)x->x_grrgb[2];
    fgcol = ((int)x->x_fgrgb[0] << 16) + ((int)x->x_fgrgb[1] << 8) + (int)x->x_fgrgb[2];
    char buf[1000];
    sprintf(buf, "::dialog_scope::pdtk_scope_dialog %%s \
            dim %d wdt: %d hgt: \
            buf %d cal: %d bfs: \
            rng %g min: %g max: \
            del %d del: drs %d drs: \
            trg %d tmd: %g tlv: \
            dim_mins %d %d \
            cal_min_max %d %d bfs_min_max %d %d \
            del_mins %d \
            #%06x #%06x #%06x\n",
            x->x_width, x->x_height,
            x->x_period, x->x_bufsize,
            x->x_minval, x->x_maxval,
            x->x_delay, x->x_drawstyle,
            x->x_trigmode, x->x_triglevel,
            SCOPE_MINWIDTH, SCOPE_MINHEIGHT,
            SCOPE_MINPERIOD, SCOPE_MAXPERIOD,
            SCOPE_MINBUFSIZE, SCOPE_MAXBUFSIZE,
            SCOPE_MINDELAY,
            bgcol, grcol, fgcol);
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

// #endif // end pd vanilla GUI code

static int scope_getcolorarg(int index, int argc, t_atom *argv)
{
    if (index < 0 || index >= argc)
        return 0;
    if (IS_A_FLOAT(argv,index))
        return atom_getintarg(index, argc, argv);
    if (IS_A_SYMBOL(argv,index))
    {
        t_symbol*s=atom_getsymbolarg(index, argc, argv);
        if ('#' == s->s_name[0])
            return strtol(s->s_name+1, 0, 16);
    }
    return(0);
}

static void scope_dialog(t_scope *x, t_symbol *s, int argc, t_atom *argv)
{
    s = NULL;
    int width = (int)atom_getintarg(0, argc, argv);
    int height = (int)atom_getintarg(1, argc, argv);
    int period = (int)atom_getintarg(2, argc, argv);
    int bufsize = (int)atom_getintarg(3, argc, argv);
    float minval = (float)atom_getfloatarg(4, argc, argv);
    float maxval = (float)atom_getfloatarg(5, argc, argv);
    int delay = (int)atom_getintarg(6, argc, argv);
    int drawstyle = (int)atom_getintarg(7, argc, argv);
    int trigmode = (int)atom_getintarg(8, argc, argv);
    float triglevel = (float)atom_getfloatarg(9, argc, argv);
    int bgcol = (int)scope_getcolorarg(10, argc, argv);
    int grcol = (int)scope_getcolorarg(11, argc, argv);
    int fgcol = (int)scope_getcolorarg(12, argc, argv);
    
    int bgred = (bgcol & 0xFF0000) >> 16;
    int bggreen = (bgcol & 0x00FF00) >> 8;
    int bgblue = (bgcol & 0x0000FF);
    
    int grred = (grcol & 0xFF0000) >> 16;
    int grgreen = (grcol & 0x00FF00) >> 8;
    int grblue = (grcol & 0x0000FF);
    
    int fgred = (fgcol & 0xFF0000) >> 16;
    int fggreen = (fgcol & 0x00FF00) >> 8;
    int fgblue = (fgcol & 0x0000FF);
    
    scope_period(x, period);
    scope_bufsize(x, bufsize);
    scope_range(x, minval, maxval);
    scope_delay(x, delay);
    scope_drawstyle(x, drawstyle);
    scope_trigger(x, trigmode);
    scope_triglevel(x, triglevel);
    if (x->x_width != width || x->x_height != height ||
        x->x_bgrgb[0] != bgred || x->x_bgrgb[1] != bggreen || x->x_bgrgb[2] != bgblue ||
        x->x_grrgb[0] != grred || x->x_grrgb[1] != grgreen || x->x_grrgb[2] != grblue ||
        x->x_fgrgb[0] != fgred || x->x_fgrgb[1] != fggreen || x->x_fgrgb[2] != fgblue)
    {
        scope_brgb(x, bgred, bggreen, bgblue);
        scope_grgb(x, grred, grgreen, grblue);
        scope_frgb(x, fgred, fggreen, fgblue);
        scope_resize(x, width, height);
        canvas_dirty(x->x_canvas, 1);
    }
}

static void *scope_new(t_symbol *s, int argc, t_atom *argv)
{
    s = NULL;
    t_scope *x = (t_scope *)pd_new(scope_class);
    t_scopehandle *sh;
    char hbuf[64];
    x->x_canvas = canvas_getcurrent();
    x->x_glist = (t_glist *)x->x_canvas;
    x->x_allocsize =  (int) SCOPE_DEFBUFSIZE;
    x->x_bufsize = 0;
    int argnum = 0;
    t_float width = SCOPE_DEFWIDTH;
    t_float height = SCOPE_DEFHEIGHT;
    t_float period = (t_float)SCOPE_DEFPERIOD;
    t_float bufsize = (t_float)SCOPE_DEFBUFSIZE;
    x->x_lastbufsize = (int)bufsize;
    t_float minval = (t_float)SCOPE_DEFMINVAL;
    t_float maxval = (t_float)SCOPE_DEFMAXVAL;
    t_float delay = (t_float)SCOPE_DEFDELAY;
    t_float drawstyle = (t_float)SCOPE_DEFDRAWSTYLE;
    t_float trigger = (t_float)SCOPE_DEFTRIGMODE;
    t_float triglevel = (t_float)SCOPE_DEFTRIGLEVEL;
    t_float fgred = (t_float)SCOPE_DEFFGRED;
    t_float fggreen = (t_float)SCOPE_DEFFGGREEN;
    t_float fgblue = (t_float)SCOPE_DEFFGBLUE;
    t_float bgred = (t_float)SCOPE_DEFBGRED;
    t_float bggreen = (t_float)SCOPE_DEFBGGREEN;
    t_float bgblue = (t_float)SCOPE_DEFBGBLUE;
    t_float grred = (t_float)SCOPE_DEFGRRED;
    t_float grgreen = (t_float)SCOPE_DEFGRGREEN;
    t_float grblue = (t_float)SCOPE_DEFGRBLUE;
// default to using rgb but if color version is specified, set flags fcolset, bcolset, gcolset
    int fcolset = 0;
    int bcolset = 0;
    int gcolset = 0;
    t_float fcred = (t_float)SCOPE_MINCOLOR;
    t_float fcgreen = (t_float)SCOPE_MINCOLOR;
    t_float fcblue = (t_float)SCOPE_MINCOLOR;
    t_float bcred = (t_float)SCOPE_MINCOLOR;
    t_float bcgreen = (t_float)SCOPE_MINCOLOR;
    t_float bcblue = (t_float)SCOPE_MINCOLOR;
    t_float gcred = (t_float)SCOPE_MINCOLOR;
    t_float gcgreen = (t_float)SCOPE_MINCOLOR;
    t_float gcblue = (t_float)SCOPE_MINCOLOR;
    while(argc > 0){
        if (argv -> a_type == A_FLOAT){
            t_float argval = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    width = argval;
                    break;
                case 1:
                    height = argval;
                    break;
                case 2:
                    period = argval;
                    break;
                case 3:
                    break;
                case 4:
                    bufsize = argval;
                    break;
                case 5:
                    minval = argval;
                    break;
                case 6:
                    maxval = argval;
                    break;
                case 7:
                    delay = argval;
                    break;
                case 8:
                    break;
                case 9:
                    trigger = argval;
                    break;
                case 10:
                    triglevel = argval;
                    break;
                case 11:
                    fgred = argval;
                    break;
                case 12:
                    fggreen = argval;
                    break;
                case 13:
                    fgblue = argval;
                    break;
                case 14:
                    bgred = argval;
                    break;
                case 15:
                    bggreen = argval;
                    break;
                case 16:
                    bgblue = argval;
                    break;
                case 17:
                    grred = argval;
                    break;
                case 18:
                    grgreen = argval;
                    break;
                case 19:
                    grblue = argval;
                    break;
                default:
                    break;
            };
            argnum++;
            argc--;
            argv++;
        }
        else if (argv -> a_type == A_SYMBOL){
            t_symbol *curarg = atom_getsymbolarg(0, argc, argv);
            if (strcmp(curarg->s_name, "@calccount") == 0){
                if (argc >= 2){
                    period = atom_getfloatarg(1, argc, argv);
                    argc-=2;
                    argv+=2;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@bufsize") == 0){
                if (argc >= 2){
                    bufsize = atom_getfloatarg(1, argc, argv);
                    argc-=2;
                    argv+=2;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@range") == 0){
                if (argc >= 3){
                    minval = atom_getfloatarg(1, argc, argv);
                    maxval = atom_getfloatarg(2, argc, argv);
                    argc-=3;
                    argv+=3;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@delay") == 0){
                if (argc >= 2){
                    delay = atom_getfloatarg(1, argc, argv);
                    argc-=2;
                    argv+=2;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@drawstyle") == 0){
                if (argc >= 2){
                    drawstyle = atom_getfloatarg(1, argc, argv);
                    argc-=2;
                    argv+=2;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@trigger") == 0){
                if (argc >= 2){
                    trigger = atom_getfloatarg(1, argc, argv);
                    argc-=2;
                    argv+=2;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@triglevel") == 0){
                if (argc >= 2){
                    triglevel = atom_getfloatarg(1, argc, argv);
                    argc-=2;
                    argv+=2;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@frgb") == 0){
                if (argc >= 4){
                    fgred = atom_getfloatarg(1, argc, argv);
                    fggreen = atom_getfloatarg(2, argc, argv);
                    fgblue = atom_getfloatarg(3, argc, argv);
                    argc-=4;
                    argv+=4;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@brgb") == 0){
                if (argc >= 4){
                    bgred = atom_getfloatarg(1, argc, argv);
                    bggreen = atom_getfloatarg(2, argc, argv);
                    bgblue = atom_getfloatarg(3, argc, argv);
                    argc-=4;
                    argv+=4;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@grgb") == 0){
                if (argc >= 4){
                    grred = atom_getfloatarg(1, argc, argv);
                    grgreen = atom_getfloatarg(2, argc, argv);
                    grblue = atom_getfloatarg(3, argc, argv);
                    argc-=4;
                    argv+=4;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@fgcolor") == 0){
                if (argc >= 4){
                    fcolset = 1;
                    fcred = atom_getfloatarg(1, argc, argv);
                    fcgreen = atom_getfloatarg(2, argc, argv);
                    fcblue = atom_getfloatarg(3, argc, argv);
                    argc-=4;
                    argv+=4;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@bgcolor") == 0){
                if (argc >= 4){
                    bcolset = 1;
                    bcred = atom_getfloatarg(1, argc, argv);
                    bcgreen = atom_getfloatarg(2, argc, argv);
                    bcblue = atom_getfloatarg(3, argc, argv);
                    argc-=4;
                    argv+=4;
                }
                else
                    goto errstate;
            }
            else if (strcmp(curarg->s_name, "@gridcolor") == 0){
                if (argc >= 4){
                    gcolset = 1;
                    gcred = atom_getfloatarg(1, argc, argv);
                    gcgreen = atom_getfloatarg(2, argc, argv);
                    gcblue = atom_getfloatarg(3, argc, argv);
                    argc-=4;
                    argv+=4;
                }
                else
                    goto errstate;
            }
            else
                goto errstate;
        }
        else
            goto errstate;
    }
    x->x_rightinlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    scope_dim(x, width, height);
    scope_period(x, period);
    scope_bufsize(x, bufsize);
    x->x_signalscalar = obj_findsignalscalar((t_object *)x, 1);
    scope_range(x, minval, maxval);
    scope_delay(x, delay);
    scope_drawstyle(x, drawstyle);
    scope_trigger(x, trigger);
    scope_triglevel(x, triglevel);
    scope_frgb(x, fgred, fggreen, fgblue);
    scope_brgb(x, bgred, bggreen, bgblue);
    scope_grgb(x, grred, grgreen, grblue);
    if (fcolset) // see if we're calling the color versions
        scope_fgcolor(x, fcred, fcgreen, fcblue);
    if (bcolset)
        scope_bgcolor(x, bcred, bcgreen, bcblue);
    if (gcolset)
        scope_gridcolor(x, gcred, gcgreen, gcblue);
    sprintf(x->x_tag, "all%lx", (unsigned long)x);
    sprintf(x->x_bgtag, "bg%lx", (unsigned long)x);
    sprintf(x->x_gridtag, "gr%lx", (unsigned long)x);
    sprintf(x->x_fgtag, "fg%lx", (unsigned long)x);
    sprintf(x->x_margintag, "ma%lx", (unsigned long)x);
    x->x_xymode = x->x_lastxymode = x->x_init = 0;
    x->x_frozen = 0;
    x->x_clock = clock_new(x, (t_method)scope_tick);
    scope_clear(x, 0);
    x->x_handle = pd_new(scopehandle_class);
    sh = (t_scopehandle *)x->x_handle;
    sh->h_master = x;
    sprintf(hbuf, "_h%lx", (unsigned long)sh);
    pd_bind(x->x_handle, sh->h_bindsym = gensym(hbuf));
    sprintf(sh->h_outlinetag, "h%lx", (unsigned long)sh);
    sh->h_dragon = 0;
    return (x);
errstate:
    pd_error(x, "scope~: improper creation arguments");
    return NULL;
}

CYCLONE_OBJ_API void scope_tilde_setup(void){
    scope_class = class_new(gensym("scope~"), (t_newmethod)scope_new,
            (t_method)scope_free, sizeof(t_scope), 0, A_GIMME, 0);
//    class_addcreator((t_newmethod)scope_new, gensym("Scope~"), A_GIMME, 0); // backwards compatible
//    class_addcreator((t_newmethod)scope_new, gensym("cyclone/Scope~"), A_GIMME, 0); // backwards compatible
    class_addmethod(scope_class, nullfn, gensym("signal"), 0);
    class_addmethod(scope_class, (t_method) scope_dsp, gensym("dsp"), A_CANT, 0);
    class_addfloat(scope_class, (t_method)scope_float);
    class_addmethod(scope_class, (t_method)scope_period, gensym("calccount"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_bufsize, gensym("bufsize"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_resize, gensym("dim"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_range, gensym("range"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_delay, gensym("delay"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_drawstyle, gensym("drawstyle"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_trigger, gensym("trigger"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_triglevel, gensym("triglevel"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_frgb, gensym("frgb"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_brgb, gensym("brgb"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_grgb, gensym("grgb"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_fgcolor, gensym("fgcolor"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_bgcolor, gensym("bgcolor"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_gridcolor, gensym("gridcolor"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_dialog, gensym("dialog"), A_GIMME, 0);
    class_addmethod(scope_class, (t_method)scope_click, gensym("click"), A_FLOAT,
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_motion, gensym("motion"), 0);
    class_addmethod(scope_class, (t_method)scope_resize, gensym("resize"), A_FLOAT, A_FLOAT, 0);
    class_setwidget(scope_class, &scope_widgetbehavior);
    class_setsavefn(scope_class, scope_save);
    class_setpropertiesfn(scope_class, scope_properties);
    
    scopehandle_class = class_new(gensym("_scopehandle"), 0, 0, sizeof(t_scopehandle), CLASS_PD, 0);
    class_addmethod(scopehandle_class, (t_method)scopehandle__clickhook, gensym("_click"), A_FLOAT, 0);
    class_addmethod(scopehandle_class, (t_method)scopehandle__motionhook, gensym("_motion"), A_FLOAT, A_FLOAT, 0);
//    class_sethelpsymbol(scope_class, gensym("scope~"));
/* #ifdef PDL2ORK // extra methods and widgetbehavior for purr data
    class_addmethod(scope_class, (t_method)scope_click_for_resizing,
                    gensym("_click"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_motion_for_resizing,
                    gensym("_motion"), A_FLOAT, A_FLOAT, 0);
    scope_widgetbehavior.w_displacefnwtag = scope_displace_wtag;
#else // vanilla requires an avalanche of tcl code injections */
    #include "scope_dialog.c"
// #endif
}
