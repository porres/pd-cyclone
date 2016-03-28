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

/*2016 = haven't finished cleaning out old dependencies
 (common/grow, common/loud, common/fitter, unstable/forky, sickle/sic)
but methods (bufsize, period/calccount, range, delay, trigger, triglevel, frgb, brgb)
for setting attributes are rewritten, as well as attr declaration.
also if(cv = scope_isvisible(x)) seems to be incorrect but is also everywhere the same way so I haven't touched it just in case everything breaks
- Derek Kwan
*/

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/fitter.h"
#include "unstable/forky.h"
#include "sickle/sic.h"

#ifdef KRZYSZCZ
//#define SCOPE_DEBUG
#endif

/* these are powers of 2 + margins */
#define SCOPE_DEFWIDTH     130  /* CHECKED */
#define SCOPE_MINWIDTH      66
#define SCOPE_DEFHEIGHT    130  /* CHECKED */
#define SCOPE_MINHEIGHT     34
#define SCOPE_DEFPERIOD    256
#define SCOPE_MINPERIOD      2
#define SCOPE_MAXPERIOD   8192
#define SCOPE_DEFBUFSIZE   128
#define SCOPE_MINBUFSIZE     8
#define SCOPE_MAXBUFSIZE   256  /* LATER rethink */
#define SCOPE_WARNBUFSIZE  256
#define SCOPE_DEFMINVAL     -1.
#define SCOPE_DEFMAXVAL      1.
#define SCOPE_DEFDELAY       0
#define SCOPE_MINDELAY       0
#define SCOPE_TRIGLINEMODE   0
#define SCOPE_TRIGUPMODE     1
#define SCOPE_TRIGDOWNMODE   2
#define SCOPE_DEFTRIGMODE    SCOPE_TRIGLINEMODE
#define SCOPE_MINTRIGMODE    SCOPE_TRIGLINEMODE
#define SCOPE_MAXTRIGMODE    SCOPE_TRIGDOWNMODE
#define SCOPE_DEFTRIGLEVEL   0.
#define SCOPE_MINCOLOR       0
#define SCOPE_MAXCOLOR     255
#define SCOPE_DEFFGRED     205
#define SCOPE_DEFFGGREEN   229
#define SCOPE_DEFFGBLUE    232
#define SCOPE_DEFBGRED     	74
#define SCOPE_DEFBGGREEN   	79
#define SCOPE_DEFBGBLUE    	77
#define SCOPE_DEFGRRED       96
#define SCOPE_DEFGRGREEN     98
#define SCOPE_DEFGRBLUE      102
#define SCOPE_SELCOLOR       "#8080ff"  /* a bit lighter shade of blue */
#define SCOPE_FGWIDTH        0.7  /* line width is float */
#define SCOPE_GRIDWIDTH      0.9
#define SCOPE_SELBDWIDTH     3.0
#define SCOPEHANDLE_WIDTH   10    /* item size is int */
#define SCOPEHANDLE_HEIGHT  10
/* these are performance-related hacks, LATER investigate */
#define SCOPE_GUICHUNKMONO  16
#define SCOPE_GUICHUNKXY    32

typedef struct _scope
{
    t_sic      x_sic;
    t_glist   *x_glist;
    t_canvas  *x_canvas;  /* also an 'isvised' flag */
    char       x_tag[64];
    char       x_fgtag[64];
    char       x_bgtag[64];
    char       x_gridtag[64];
    int        x_width;
    int        x_height;
    float      x_minval;
    float      x_maxval;
    int        x_delay;
    int        x_trigmode;
    float      x_triglevel;
    unsigned char  x_fgred;
    unsigned char  x_fggreen;
    unsigned char  x_fgblue;
    unsigned char  x_bgred;
    unsigned char  x_bggreen;
    unsigned char  x_bgblue;
    unsigned char  x_grred; 
    unsigned char  x_grgreen; 
    unsigned char  x_grblue;
    int        x_xymode;
    float     *x_xbuffer;
    float     *x_ybuffer;
    float      x_xbufini[SCOPE_DEFBUFSIZE];
    float      x_ybufini[SCOPE_DEFBUFSIZE];
    int        x_allocsize;
    int        x_bufsize;
    int        x_bufphase;
    int        x_period;
    int        x_phase;
    int        x_precount;
    int        x_retrigger;
    float      x_ksr;
    float      x_currx;
    float      x_curry;
    float      x_trigx;
    int        x_frozen;
    t_clock   *x_clock;
    t_pd      *x_handle;
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

static void scope_clear(t_scope *x, int withdelay)
{
    x->x_bufphase = 0;
    x->x_phase = 0;
    x->x_precount = (withdelay ? (int)(x->x_delay * x->x_ksr) : 0);
    /* CHECKED delay does not matter (refman is wrong) */
    x->x_retrigger = (x->x_trigmode != SCOPE_TRIGLINEMODE);
    x->x_trigx = x->x_triglevel;
}

static t_int *scope_monoperform(t_int *w)
{
    t_scope *x = (t_scope *)(w[1]);
    int bufphase = x->x_bufphase;
    int bufsize = x->x_bufsize;
    if (bufphase < bufsize)
    {
	int nblock = (int)(w[2]);
	if (x->x_precount >= nblock)
	    x->x_precount -= nblock;
	else
	{
	    t_float *in = (t_float *)(w[3]);
	    int phase = x->x_phase;
	    int period = x->x_period;
	    float *bp1 = x->x_xbuffer + bufphase;
	    float *bp2 = x->x_ybuffer + bufphase;
	    float currx = x->x_currx;
	    if (x->x_precount > 0)
	    {
		nblock -= x->x_precount;
		in += x->x_precount;
		x->x_precount = 0;
	    }
	    while (x->x_retrigger)
	    {
		float triglevel = x->x_triglevel;
		if (x->x_trigmode == SCOPE_TRIGUPMODE)
		{
		    if (x->x_trigx < triglevel)
		    {
			while (nblock--) if (*in++ >= triglevel)
			{
			    x->x_retrigger = 0;
			    break;
			}
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
			while (nblock--) if (*in++ <= triglevel)
			{
			    x->x_retrigger = 0;
			    break;
			}
		    }
		    else while (nblock--) if (*in++ > triglevel)
		    {
			x->x_trigx = triglevel + 1.;
			break;
		    }
		}
		if (nblock <= 0)
		    return (w + 4);
	    }
	    while (nblock--)
	    {
		if (phase)
		{
		    float f = *in++;
		    /* CHECKED */
		    if ((currx < 0 && (f < currx || f > -currx)) ||
			(currx > 0 && (f > currx || f < -currx)))
			currx = f;
		}
		else currx = *in++;
		if (currx != currx)
		    currx = 0.;  /* CHECKED NaNs bashed to zeros */
		if (++phase == period)
		{
		    phase = 0;
		    if (++bufphase == bufsize)
		    {
			*bp1 = *bp2 = currx;
			clock_delay(x->x_clock, 0);
			break;
		    }
		    else *bp1++ = *bp2++ = currx;
		}
	    }
	    x->x_currx = currx;
	    x->x_bufphase = bufphase;
	    x->x_phase = phase;
	}
    }
    return (w + 4);
}

static t_int *scope_xyperform(t_int *w)
{
    t_scope *x = (t_scope *)(w[1]);
    int bufphase = x->x_bufphase;
    int bufsize = x->x_bufsize;
    if (bufphase < bufsize)
    {
	int nblock = (int)(w[2]);
	if (x->x_precount >= nblock)
	    x->x_precount -= nblock;
	else
	{
	    t_float *in1 = (t_float *)(w[3]);
	    t_float *in2 = (t_float *)(w[4]);
	    int phase = x->x_phase;
	    int period = x->x_period;
	    float freq = 1. / period;
	    float *bp1 = x->x_xbuffer + bufphase;
	    float *bp2 = x->x_ybuffer + bufphase;
	    float currx = x->x_currx;
	    float curry = x->x_curry;
	    if (x->x_precount > 0)
	    {
		nblock -= x->x_precount;
		in1 += x->x_precount;
		in2 += x->x_precount;
		x->x_precount = 0;
	    }
	    if (x->x_retrigger)
	    {
		/* CHECKME and FIXME */
		x->x_retrigger = 0;
	    }
	    while (nblock--)
	    {
		if (phase)
		{
		    /* CHECKME */
		    currx += *in1++;
		    curry += *in2++;
		}
		else
		{
		    currx = *in1++;
		    curry = *in2++;
		}
		if (currx != currx)
		    currx = 0.;  /* CHECKME NaNs bashed to zeros */
		if (curry != curry)
		    curry = 0.;  /* CHECKME NaNs bashed to zeros */
		if (++phase == period)
		{
		    phase = 0;
		    if (++bufphase == bufsize)
		    {
			*bp1 = currx * freq;
			*bp2 = curry * freq;
			clock_delay(x->x_clock, 0);
			break;
		    }
		    else
		    {
			*bp1++ = currx * freq;
			*bp2++ = curry * freq;
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
    scope_setxymode(x,
		    forky_hasfeeders((t_object *)x, x->x_glist, 1, &s_signal));
    if (x->x_xymode)
	dsp_add(scope_xyperform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
    else
	dsp_add(scope_monoperform, 3, x, sp[0]->s_n, sp[0]->s_vec);
}

static t_canvas *scope_getcanvas(t_scope *x, t_glist *glist)
{
    if (glist != x->x_glist)
    {
	loudbug_bug("scope_getcanvas");
	x->x_glist = glist;
    }
    return (x->x_canvas = glist_getcanvas(glist));
}

/* answers the question:  ``can we draw and where to?'' */
static t_canvas *scope_isvisible(t_scope *x)
{
    return (glist_isvisible(x->x_glist) ? x->x_canvas : 0);
}

static void scope_period(t_scope *x, t_float per)
{ 
	if(per < SCOPE_MINPERIOD){
		per = SCOPE_MINPERIOD;
	}
	else if(per > SCOPE_MAXPERIOD){
		per = SCOPE_MAXPERIOD;
	}
	else{
		per = (int)per;
	};
	x->x_period = per;
	scope_clear(x, 0);
}

static void scope_float(t_scope *x, t_float f)
{
    scope_period(x, f);
}

static void scope_bufsize(t_scope *x, t_float bufsz)
{ 	float bufsize;
	if(bufsz < SCOPE_MINBUFSIZE){
		bufsize = SCOPE_MINBUFSIZE;
	}
	else if(bufsz > SCOPE_MAXBUFSIZE){
		bufsize = SCOPE_MAXBUFSIZE;
	}
	else{
		bufsize = bufsz;
	};
	int newsize = (int)bufsize;
	if (newsize > x->x_allocsize)
	{
	    int nrequested = newsize;
	    int allocsize = x->x_allocsize;
	    int oldsize = x->x_bufsize;
	    x->x_xbuffer = grow_withdata(&nrequested, &oldsize,
					 &allocsize, x->x_xbuffer,
					 SCOPE_DEFBUFSIZE, x->x_xbufini,
					 sizeof(*x->x_xbuffer));
	    if (nrequested == newsize)
	    {
		allocsize = x->x_allocsize;
		oldsize = x->x_bufsize;
		x->x_ybuffer = grow_withdata(&nrequested, &oldsize,
					     &allocsize, x->x_ybuffer,
					     SCOPE_DEFBUFSIZE, x->x_ybufini,
					     sizeof(*x->x_ybuffer));
	    }
	    if (nrequested == newsize)
	    {
		x->x_allocsize = allocsize;
		x->x_bufsize = newsize;
	    }
	    else
	    {
		if (x->x_xbuffer != x->x_xbufini)
		    freebytes(x->x_xbuffer,
			      x->x_allocsize * sizeof(*x->x_xbuffer));
		if (x->x_ybuffer != x->x_ybufini)
		    freebytes(x->x_ybuffer,
			      x->x_allocsize * sizeof(*x->x_ybuffer));
		x->x_allocsize = SCOPE_DEFBUFSIZE;
		x->x_bufsize = SCOPE_DEFBUFSIZE;
		x->x_xbuffer = x->x_xbufini;
		x->x_ybuffer = x->x_ybufini;
	    }
	}
	else{
		x->x_bufsize = newsize;
	};
	scope_clear(x, 0);
}

static void scope_range(t_scope *x, t_float min, t_float max)
{
    t_float minval = min;
    t_float maxval = max;
    /* CHECKME swapping, ignoring if equal */
    if (minval < maxval)
    {
	x->x_minval = minval;
	x->x_maxval = maxval;
    }
    
	else{
	x->x_minval = maxval;
	x->x_maxval = minval;
    };
}

static void scope_delay(t_scope *x, t_float del)
{
    if(del < SCOPE_MINDELAY){
		del = SCOPE_MINDELAY;
	};
	x->x_delay = del;
}

static void scope_trigger(t_scope *x, t_float trig)
{
	float trigmode;
	if(trig < SCOPE_MINTRIGMODE){
		trigmode = SCOPE_MINTRIGMODE;
	}
	else if(trig > SCOPE_MAXTRIGMODE){
		trigmode = SCOPE_MAXTRIGMODE;
	}
	else{
		trigmode = trig;
	};
    x->x_trigmode = (int)trigmode;
    if (x->x_trigmode == SCOPE_TRIGLINEMODE){
		x->x_retrigger = 0;
	};
}

static void scope_triglevel(t_scope *x, t_float lvl)
{
    x->x_triglevel = lvl;
}

static void scope_frgb(t_scope *x, t_float fr, t_float fg, t_float fb)
{
	if(fr < SCOPE_MINCOLOR){
		fr = SCOPE_MINCOLOR;
	}
	else if(fr > SCOPE_MAXCOLOR){
		fr = SCOPE_MAXCOLOR;
	};
    if(fg < SCOPE_MINCOLOR){
		fg = SCOPE_MINCOLOR;
	}
	else if(fg > SCOPE_MAXCOLOR){
		fg = SCOPE_MAXCOLOR;
	};
	if(fb < SCOPE_MINCOLOR){
		fb = SCOPE_MINCOLOR;
	}
	else if(fb > SCOPE_MAXCOLOR){
		fb = SCOPE_MAXCOLOR;
	};

	t_canvas *cv;
    x->x_fgred = (int)fr;
    x->x_fggreen = (int)fg;
    x->x_fgblue = (int)fb;
    if (cv = scope_isvisible(x))
	sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_fgtag, x->x_fgred, x->x_fggreen, x->x_fgblue);
}

static void scope_brgb(t_scope *x, t_float br, t_float bg, t_float bb)
{
    if(br < SCOPE_MINCOLOR){
		br = SCOPE_MINCOLOR;
	}
	else if(br > SCOPE_MAXCOLOR){
		br = SCOPE_MAXCOLOR;
	};
    if(bg < SCOPE_MINCOLOR){
		bg = SCOPE_MINCOLOR;
	}
	else if(bg > SCOPE_MAXCOLOR){
		bg = SCOPE_MAXCOLOR;
	};
	if(bb < SCOPE_MINCOLOR){
		bb = SCOPE_MINCOLOR;
	}
	else if(bb > SCOPE_MAXCOLOR){
		bb = SCOPE_MAXCOLOR;
	};
	
	t_canvas *cv;
    x->x_bgred = (int)br;
    x->x_bggreen = (int)bg;
    x->x_bgblue = (int)bb;
    if (cv = scope_isvisible(x))
	sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_bgtag, x->x_bgred, x->x_bggreen, x->x_bgblue);
}

static void scope_grgb(t_scope *x, t_float gr, t_float gg, t_float gb)
{   
	if(gr < SCOPE_MINCOLOR){
		gr = SCOPE_MINCOLOR;
	}
	else if(gr > SCOPE_MAXCOLOR){
		gr = SCOPE_MAXCOLOR;
	};
    if(gg < SCOPE_MINCOLOR){
		gg = SCOPE_MINCOLOR;
	}
	else if(gg > SCOPE_MAXCOLOR){
		gg = SCOPE_MAXCOLOR;
	};
	if(gb < SCOPE_MINCOLOR){
		gb = SCOPE_MINCOLOR;
	}
	else if(gb > SCOPE_MAXCOLOR){
		gb = SCOPE_MAXCOLOR;
	};
	

    t_canvas *cv;
    x->x_grred   = (int)gr;
    x->x_grgreen = (int)gg;
    x->x_grblue  = (int)gb;
    if (cv = scope_isvisible(x))
	sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_gridtag, x->x_grred, x->x_grgreen, x->x_grblue);
}

static void scope_getrect(t_gobj *z, t_glist *glist,
			  int *xp1, int *yp1, int *xp2, int *yp2)
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

static void scope_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)z;
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
    {
	t_canvas *cv = scope_getcanvas(x, glist);
	sys_vgui(".x%lx.c move %s %d %d\n", cv, x->x_tag, dx, dy);
	canvas_fixlinesfor(cv, t);
    }
}

static void scope_select(t_gobj *z, t_glist *glist, int state)
{
    t_scope *x = (t_scope *)z;
    t_canvas *cv = scope_getcanvas(x, glist);
    t_scopehandle *sh = (t_scopehandle *)x->x_handle;
    if (state)
    {
	int x1, y1, x2, y2;
	scope_getrect(z, glist, &x1, &y1, &x2, &y2);

	sys_vgui(".x%lx.c itemconfigure %s -outline blue -width %f -fill %s\n",
		 cv, x->x_bgtag, SCOPE_SELBDWIDTH, SCOPE_SELCOLOR);

	sys_vgui("canvas %s -width %d -height %d -bg #fedc00 -bd 0\n",
		 sh->h_pathname, SCOPEHANDLE_WIDTH, SCOPEHANDLE_HEIGHT);
	sys_vgui(".x%lx.c create window %f %f -anchor nw\
 -width %d -height %d -window %s -tags %s\n",
		 cv, x2 - (SCOPEHANDLE_WIDTH - SCOPE_SELBDWIDTH),
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
 -fill #%2.2x%2.2x%2.2x\n", cv, x->x_bgtag, SCOPE_GRIDWIDTH,
		 x->x_bgred, x->x_bggreen, x->x_bgblue);
	sys_vgui("destroy %s\n", sh->h_pathname);
    }
}

static void scope_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void scope_drawfgmono(t_scope *x, t_canvas *cv,
			     int x1, int y1, int x2, int y2)
{
    int i;
    float dx, dy, xx, yy, sc;
    float *bp;
    dx = (float)(x2 - x1) / (float)x->x_bufsize;
    sc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    sys_vgui(".x%lx.c create line \\\n", cv);
    for (i = 0, xx = x1, bp = x->x_xbuffer;
	 i < x->x_bufsize; i++, xx += dx, bp++)
    {
	yy = (y2 - 1) - sc * (*bp - x->x_minval);
#ifndef SCOPE_DEBUG
	if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
#endif
	sys_vgui("%d %d \\\n", (int)xx, (int)yy);
    }
    sys_vgui("-fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
	     x->x_fgred, x->x_fggreen, x->x_fgblue,
	     SCOPE_FGWIDTH, x->x_fgtag, x->x_tag);

    /* margin lines:  masking overflows, so that they appear as gaps,
       rather than clipped signal values, LATER rethink */
    sys_vgui(".x%lx.c create line %d %d %d %d\
 -fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
	     cv, x1, y1, x2, y1, x->x_bgred, x->x_bggreen, x->x_bgblue,
	     1., x->x_fgtag, x->x_tag);
    sys_vgui(".x%lx.c create line %d %d %d %d\
 -fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
	     cv, x1, y2, x2, y2, x->x_bgred, x->x_bggreen, x->x_bgblue,
	     1., x->x_fgtag, x->x_tag);
}

static void scope_drawfgxy(t_scope *x, t_canvas *cv,
			   int x1, int y1, int x2, int y2)
{
    int nleft = x->x_bufsize;
    float *xbp = x->x_xbuffer, *ybp = x->x_ybuffer;
    char chunk[200 * SCOPE_GUICHUNKXY];  /* LATER estimate */
    char *chunkp = chunk;
    char cmd1[64], cmd2[64];
    float xx, yy, xsc, ysc;
    xx = yy = 0;
    /* subtract 1-pixel margins, see below */
    xsc = ((float)x->x_width - 2.) / (float)(x->x_maxval - x->x_minval);
    ysc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    sprintf(cmd1, ".x%lx.c create line", (unsigned long)cv);
    sprintf(cmd2, "-fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n ",
	    x->x_fgred, x->x_fggreen, x->x_fgblue,
	    SCOPE_FGWIDTH, x->x_fgtag, x->x_tag);
    while (nleft > SCOPE_GUICHUNKXY)
    {
	int i = SCOPE_GUICHUNKXY;
	while (i--)
	{
	    float oldx = xx, oldy = yy, dx, dy;
	    xx = x1 + xsc * (*xbp++ - x->x_minval);
	    yy = y2 - ysc * (*ybp++ - x->x_minval);
	    /* using 1-pixel margins */
	    dx = (xx > oldx ? 1. : -1.);
	    dy = (yy > oldy ? 1. : -1.);
#ifndef SCOPE_DEBUG
	    if (xx < x1 || xx > x2 || yy < y1 || yy > y2)
		continue;
#endif
	    sprintf(chunkp, "%s %d %d %d %d %s", cmd1,
		    (int)(xx - dx), (int)(yy - dy),
		    (int)(xx + dx), (int)(yy + dy), cmd2);
	    chunkp += strlen(chunkp);
	}
	if (chunkp > chunk)
	    sys_gui(chunk);
	chunkp = chunk;
	nleft -= SCOPE_GUICHUNKXY;
    }
    while (nleft--)
    {
	float oldx = xx, oldy = yy, dx, dy;
	xx = x1 + xsc * (*xbp++ - x->x_minval);
	yy = y2 - ysc * (*ybp++ - x->x_minval);
	/* using 1-pixel margins */
	dx = (xx > oldx ? 1. : -1.);
	dy = (yy > oldy ? 1. : -1.);
#ifndef SCOPE_DEBUG
	if (xx < x1 || xx > x2 || yy < y1 || yy > y2)
	    continue;
#endif
	sprintf(chunkp, "%s %d %d %d %d %s", cmd1,
		(int)(xx - dx), (int)(yy - dy),
		(int)(xx + dx), (int)(yy + dy), cmd2);
	chunkp += strlen(chunkp);
    }
    if (chunkp > chunk)
	sys_gui(chunk);
}

static void scope_drawbg(t_scope *x, t_canvas *cv,
			 int x1, int y1, int x2, int y2)
{
    int i;
    float dx, dy, xx, yy;
    dx = (x2 - x1) * 0.125;
    dy = (y2 - y1) * 0.25;
    sys_vgui(".x%lx.c create rectangle %d %d %d %d\
 -fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
	     cv, x1, y1, x2, y2,
	     x->x_bgred, x->x_bggreen, x->x_bgblue,
	     SCOPE_GRIDWIDTH, x->x_bgtag, x->x_tag);
                 
    for (i = 0, xx = x1 + dx; i < 7; i++, xx += dx) 
	sys_vgui(".x%lx.c create line %f %d %f %d\
 -width %f -tags {%s %s} -fill #%2.2x%2.2x%2.2x\n", cv, xx, y1, xx, y2,
		 SCOPE_GRIDWIDTH, x->x_gridtag, x->x_tag, 
		 x->x_grred, x->x_grgreen, x->x_grblue);
    for (i = 0, yy = y1 + dy; i < 3; i++, yy += dy)
	sys_vgui(".x%lx.c create line %d %f %d %f\
 -width %f -tags {%s %s} -fill #%2.2x%2.2x%2.2x\n", cv, x1, yy, x2, yy,
		 SCOPE_GRIDWIDTH, x->x_gridtag, x->x_tag,
		 x->x_grred, x->x_grgreen, x->x_grblue);
}

static void scope_drawmono(t_scope *x, t_canvas *cv)
{
    int x1, y1, x2, y2;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    scope_drawbg(x, cv, x1, y1, x2, y2);
    scope_drawfgmono(x, cv, x1, y1, x2, y2);
}

static void scope_redrawmono(t_scope *x, t_canvas *cv)
{
    int nleft = x->x_bufsize;
    float *bp = x->x_xbuffer;
    char chunk[32 * SCOPE_GUICHUNKMONO];  /* LATER estimate */
    char *chunkp = chunk;
    int x1, y1, x2, y2;
    float dx, dy, xx, yy, sc;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    dx = (float)(x2 - x1) / (float)x->x_bufsize;
    sc = ((float)x->x_height - 2.) / (float)(x->x_maxval - x->x_minval);
    xx = x1;
    sys_vgui(".x%lx.c coords %s \\\n", cv, x->x_fgtag);
    while (nleft > SCOPE_GUICHUNKMONO)
    {
	int i = SCOPE_GUICHUNKMONO;
	while (i--)
	{
	    yy = (y2 - 1) - sc * (*bp++ - x->x_minval);
#ifndef SCOPE_DEBUG
	    if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
#endif
	    sprintf(chunkp, "%d %d ", (int)xx, (int)yy);
	    chunkp += strlen(chunkp);
	    xx += dx;
	}
	strcpy(chunkp, "\\\n");
	sys_gui(chunk);
	chunkp = chunk;
	nleft -= SCOPE_GUICHUNKMONO;
    }
    while (nleft--)
    {
	yy = (y2 - 1) - sc * (*bp++ - x->x_minval);
#ifndef SCOPE_DEBUG
	if (yy > y2) yy = y2; else if (yy < y1) yy = y1;
#endif
	sprintf(chunkp, "%d %d ", (int)xx, (int)yy);
	chunkp += strlen(chunkp);
	xx += dx;
    }
    strcpy(chunkp, "\n");
    sys_gui(chunk);
}

static void scope_drawxy(t_scope *x, t_canvas *cv)
{
    int x1, y1, x2, y2;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    scope_drawbg(x, cv, x1, y1, x2, y2);
    scope_drawfgxy(x, cv, x1, y1, x2, y2);
}

static void scope_redrawxy(t_scope *x, t_canvas *cv)
{
    int x1, y1, x2, y2;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    sys_vgui(".x%lx.c delete %s\n", cv, x->x_fgtag);
    scope_drawfgxy(x, cv, x1, y1, x2, y2);
}

static void scope_revis(t_scope *x, t_canvas *cv)
{
    sys_vgui(".x%lx.c delete %s\n", cv, x->x_tag);
    if (x->x_xymode)
	scope_drawxy(x, cv);
    else
	scope_drawmono(x, cv);
}

static void scope_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_scope *x = (t_scope *)z;
    t_text *t = (t_text *)z;
    t_canvas *cv = scope_getcanvas(x, glist);
    if (vis)
    {
	t_scopehandle *sh = (t_scopehandle *)x->x_handle;
#if FORKY_VERSION < 37
	rtext_new(glist, t, glist->gl_editor->e_rtext, 0);
#endif
	sprintf(sh->h_pathname, ".x%lx.h%lx", (unsigned long)cv, (unsigned long)sh);
	if (x->x_xymode)
	    scope_drawxy(x, cv);
	else
	    scope_drawmono(x, cv);
    }
    else
    {
#if FORKY_VERSION < 37
	t_rtext *rt = glist_findrtext(glist, t);
	if (rt) rtext_free(rt);
#endif
	sys_vgui(".x%lx.c delete %s\n", (unsigned long)cv, x->x_tag);
	x->x_canvas = 0;
    }
}

static int scope_click(t_gobj *z, t_glist *glist,
		       int xpix, int ypix, int shift, int alt, int dbl,
		       int doit)
{
    t_scope *x = (t_scope *)z;
    x->x_frozen = doit;
    return (CURSOR_RUNMODE_CLICKME);
}

/* CHECKED there is only one copy of state variables,
   the same, whether modified with messages, or in the inspector */
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
		x->x_fgred, x->x_fggreen, x->x_fgblue,
		x->x_bgred, x->x_bggreen, x->x_bgblue,
		x->x_grred, x->x_grgreen, x->x_grblue, 0);
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
    FORKY_WIDGETPADDING
};

static void scope_setxymode(t_scope *x, int xymode)
{
    if (xymode != x->x_xymode)
    {
	t_canvas *cv;
	if (cv = scope_isvisible(x))
	{
	    sys_vgui(".x%lx.c delete %s\n", cv, x->x_fgtag);
	    if (!xymode)
	    {
		int x1, y1, x2, y2;
		scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
		scope_drawfgmono(x, cv, x1, y1, x2, y2);
	    }
	}
	x->x_xymode = xymode;
	scope_clear(x, 0);
    }
}

static void scope_tick(t_scope *x)
{
    t_canvas *cv;
    if (!x->x_frozen && (cv = scope_isvisible(x)))
    {
	if (x->x_xymode)
	    scope_redrawxy(x, cv);
	else
	    scope_redrawmono(x, cv);
    }
    scope_clear(x, 1);
}

static void scope_resize(t_scope *x, t_float w, t_float h)
{
    x->x_width  = (int)(w > 0.0f) ? w : x->x_width;
    x->x_height = (int)(h > 0.0f) ? h : x->x_height;
    if (x->x_xymode)
        scope_redrawxy(x, x->x_canvas);
    else
        scope_redrawmono(x, x->x_canvas);
    scope_revis(x, x->x_canvas);
}

static void scopehandle__clickhook(t_scopehandle *sh, t_floatarg f)
{
    int newstate = (int)f;
    if (sh->h_dragon && newstate == 0)
    {
	t_scope *x = sh->h_master;
	t_canvas *cv;
	x->x_width += sh->h_dragx;
	x->x_height += sh->h_dragy;
	if (cv = scope_isvisible(x))
	{
	    sys_vgui(".x%lx.c delete %s\n", cv, sh->h_outlinetag);
	    scope_revis(x, cv);
	    sys_vgui("destroy %s\n", sh->h_pathname);
	    scope_select((t_gobj *)x, x->x_glist, 1);
	    canvas_fixlinesfor(x->x_glist, (t_text *)x);  /* 2nd inlet */
	}
    }
    else if (!sh->h_dragon && newstate)
    {
	t_scope *x = sh->h_master;
	t_canvas *cv;
	if (cv = scope_isvisible(x))
	{
	    int x1, y1, x2, y2;
	    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
	    sys_vgui("lower %s\n", sh->h_pathname);
	    sys_vgui(".x%lx.c create rectangle %d %d %d %d\
 -outline blue -width %f -tags %s\n",
		     cv, x1, y1, x2, y2, SCOPE_SELBDWIDTH, sh->h_outlinetag);
	}
	sh->h_dragx = 0;
	sh->h_dragy = 0;
    }
    sh->h_dragon = newstate;
}

static void scopehandle__motionhook(t_scopehandle *sh,
				    t_floatarg f1, t_floatarg f2)
{
    if (sh->h_dragon)
    {
	t_scope *x = sh->h_master;
	int dx = (int)f1, dy = (int)f2;
	int x1, y1, x2, y2, newx, newy;
	scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
	newx = x2 + dx;
	newy = y2 + dy;
	if (newx > x1 + SCOPE_MINWIDTH && newy > y1 + SCOPE_MINHEIGHT)
	{
	    t_canvas *cv;
	    if (cv = scope_isvisible(x))
		sys_vgui(".x%lx.c coords %s %d %d %d %d\n",
			 cv, sh->h_outlinetag, x1, y1, newx, newy);
	    sh->h_dragx = dx;
	    sh->h_dragy = dy;
	}
    }
}

static void scope_free(t_scope *x)
{
    if (x->x_clock) clock_free(x->x_clock);
    if (x->x_xbuffer != x->x_xbufini)
	freebytes(x->x_xbuffer, x->x_allocsize * sizeof(*x->x_xbuffer));
    if (x->x_ybuffer != x->x_ybufini)
	freebytes(x->x_ybuffer, x->x_allocsize * sizeof(*x->x_ybuffer));
    if (x->x_handle)
    {
	pd_unbind(x->x_handle, ((t_scopehandle *)x->x_handle)->h_bindsym);
	pd_free(x->x_handle);
    }
}

static void scope_dim(t_scope *x, t_float _width, t_float _height){
	if(_width < SCOPE_MINWIDTH){
		_width = SCOPE_MINWIDTH;
	};
	if(_height < SCOPE_MINHEIGHT){
		_height = SCOPE_MINHEIGHT;
	};
	x->x_width = (int) _width;
	x->x_height = (int)_height;
};


static void *scope_new(t_symbol *s, int argc, t_atom *argv)
{
    t_scope *x = (t_scope *)pd_new(scope_class);
    t_scopehandle *sh;
    char buf[64];
    x->x_glist = canvas_getcurrent();
    x->x_canvas = 0;
	/*
    loud_floatarg(*(t_pd *)x, 0, ac, av, &width,
		  SCOPE_MINWIDTH, 0,
		  LOUD_CLIP | LOUD_WARN, 0, "width");
    x->x_width = (int)width;
    loud_floatarg(*(t_pd *)x, 1, ac, av, &height,
		  SCOPE_MINHEIGHT, 0,
		  LOUD_CLIP | LOUD_WARN, 0, "height");
    x->x_height = (int)height;
	*/
	//x->x_allocsize = 0;
	x->x_allocsize = SCOPE_DEFBUFSIZE;
	x->x_xbuffer = x->x_xbufini;
	x->x_ybuffer = x->x_ybufini;
    x->x_bufsize = 0;


	//int pastargs = 0;
	int argnum = 0;
	t_float width = SCOPE_DEFWIDTH;
	t_float height = SCOPE_DEFHEIGHT;
	t_float period = (t_float)SCOPE_DEFPERIOD;
	t_float bufsize = (t_float)SCOPE_DEFBUFSIZE;
	t_float minval = (t_float)SCOPE_DEFMINVAL;
	t_float maxval = (t_float)SCOPE_DEFMAXVAL;
	t_float delay = (t_float)SCOPE_DEFDELAY;
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
	while(argc > 0){
		t_symbol *curarg = atom_getsymbolarg(0, argc, argv);
		if(curarg == &s_){//if curarg is a number
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
		else{//curarg is a string
			if(strcmp(curarg->s_name, "@calccount") == 0){
				if(argc >= 2){
					period = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};

			}
			else if(strcmp(curarg->s_name, "@bufsize") == 0){
				if(argc >= 2){
					bufsize = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@range") == 0){
				if(argc >= 3){
					minval = atom_getfloatarg(1, argc, argv);
					maxval = atom_getfloatarg(2, argc, argv);
					argc-=3;
					argv+=3;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@delay") == 0){
				if(argc >= 2){
					delay = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@trigger") == 0){
				if(argc >= 2){
					trigger = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@triglevel") == 0){
				if(argc >= 2){
					triglevel = atom_getfloatarg(1, argc, argv);
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@frgb") == 0){
				if(argc >= 4){
					fgred = atom_getfloatarg(1, argc, argv);
					fggreen = atom_getfloatarg(2, argc, argv);
					fgblue = atom_getfloatarg(3, argc, argv);
					argc-=4;
					argv+=4;
				}
				else{
					goto errstate;
				};
		}
		else if(strcmp(curarg->s_name, "@brgb") == 0){
			if(argc >= 4){
				bgred = atom_getfloatarg(1, argc, argv);
				bggreen = atom_getfloatarg(2, argc, argv);
				bgblue = atom_getfloatarg(3, argc, argv);
				argc-=4;
				argv+=4;
			}
			else{
				goto errstate;
			};
		}
		else if(strcmp(curarg->s_name, "@gridcolor") == 0){
			if(argc >= 4){
				grred = atom_getfloatarg(1, argc, argv);
				grgreen = atom_getfloatarg(2, argc, argv);
				grblue = atom_getfloatarg(3, argc, argv);
				argc-=4;
				argv+=4;
			}
			else{
				goto errstate;
			};
		}
		else{
			goto errstate;
		};
		};
	};
    scope_dim(x, width, height);
	scope_period(x, period);
    /* CHECKME 6th argument (default 3 for mono, 1 for xy */
    scope_bufsize(x, bufsize);
    scope_range(x, minval, maxval);
    scope_delay(x, delay);
    /* CHECKME 11th argument (default 0.) */
    scope_trigger(x, trigger);
    scope_triglevel(x, triglevel);
    /* CHECKME last argument (default 0) */
	scope_frgb(x, fgred, fggreen, fgblue);
	scope_brgb(x, bgred, bggreen, bgblue);
	scope_grgb(x, grred, grgreen, grblue);

	
    sprintf(x->x_tag, "all%lx", (unsigned long)x);
    sprintf(x->x_bgtag, "bg%lx", (unsigned long)x);
    sprintf(x->x_gridtag, "gr%lx", (unsigned long)x);
    sprintf(x->x_fgtag, "fg%lx", (unsigned long)x);
    x->x_xymode = 0;
    x->x_ksr = sys_getsr() * 0.001;  /* redundant */
    x->x_frozen = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->x_clock = clock_new(x, (t_method)scope_tick);
    scope_clear(x, 0);

    x->x_handle = pd_new(scopehandle_class);
    sh = (t_scopehandle *)x->x_handle;
    sh->h_master = x;
    sprintf(buf, "_h%lx", (unsigned long)sh);
    pd_bind(x->x_handle, sh->h_bindsym = gensym(buf));
    sprintf(sh->h_outlinetag, "h%lx", (unsigned long)sh);
    sh->h_dragon = 0;
    return (x);
	
	errstate:
		pd_error(x, "Scope~: improper args");
		return NULL;
}

void Scope_tilde_setup(void)
{
    scope_class = class_new(gensym("Scope~"),
			    (t_newmethod)scope_new,
			    (t_method)scope_free,
			    sizeof(t_scope), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)scope_new, gensym("scope~"), A_GIMME, 0);
    class_addcreator((t_newmethod)scope_new, gensym("cyclone/scope~"), A_GIMME, 0);
    sic_setup(scope_class, scope_dsp, scope_float);
	class_addmethod(scope_class, (t_method)scope_period,
			gensym("calccount"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_bufsize,
		    gensym("bufsize"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_range,
		    gensym("range"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_delay,
		    gensym("delay"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_trigger,
		    gensym("trigger"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_triglevel,
		    gensym("triglevel"), A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_frgb,
		    gensym("frgb"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_brgb,
		    gensym("brgb"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_grgb,
		    gensym("gridcolor"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
     class_addmethod(scope_class, (t_method)scope_resize,
		    gensym("resize"),
		    A_FLOAT, A_FLOAT, 0);
    class_setwidget(scope_class, &scope_widgetbehavior);
    forky_setsavefn(scope_class, scope_save);
    scopehandle_class = class_new(gensym("_scopehandle"), 0, 0,
				  sizeof(t_scopehandle), CLASS_PD, 0);
    class_addmethod(scopehandle_class, (t_method)scopehandle__clickhook,
		    gensym("_click"), A_FLOAT, 0);
    class_addmethod(scopehandle_class, (t_method)scopehandle__motionhook,
		    gensym("_motion"), A_FLOAT, A_FLOAT, 0);
    fitter_setup(scope_class, 0);
}

void scope_tilde_setup(void)
{
    Scope_tilde_setup();
}
