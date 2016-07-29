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

// 2016 = Porres cleaned "sickle/sic" old dependency

/* 2016 = haven't finished cleaning out old dependencies
 (common/grow, common/loud, common/fitter, unstable/forky)
but methods (bufsize, period/calccount, range, delay, trigger, triglevel, frgb, brgb)
for setting attributes are rewritten, as well as attr declaration. 
Have written the color version of the colorsetting methods that take vals 0-1
instead of 0-255
- Derek Kwan
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"
#include "g_all_guis.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/fitter.h"
#include "unstable/forky.h"


#ifdef KRZYSZCZ
//#define SCOPE_DEBUG
#endif

/* these are powers of 2 + margins */
#define SCOPE_DEFWIDTH     130  /* CHECKED */
#define SCOPE_MINWIDTH      10 //66
#define SCOPE_DEFHEIGHT    130  /* CHECKED */
#define SCOPE_MINHEIGHT     10 //34
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
#define SCOPE_DEFDRAWSTYLE	 0
#define SCOPE_TRIGLINEMODE   0
#define SCOPE_TRIGUPMODE     1
#define SCOPE_TRIGDOWNMODE   2
#define SCOPE_DEFTRIGMODE    SCOPE_TRIGLINEMODE
#define SCOPE_MINTRIGMODE    SCOPE_TRIGLINEMODE
#define SCOPE_MAXTRIGMODE    SCOPE_TRIGDOWNMODE
#define SCOPE_DEFTRIGLEVEL   0.
#define SCOPE_MINRGB         0
#define SCOPE_MAXRGB         255
#define SCOPE_MINCOLOR       0.
#define SCOPE_MAXCOLOR       1.
#define SCOPE_DEFFGRED       205
#define SCOPE_DEFFGGREEN     229
#define SCOPE_DEFFGBLUE      232
#define SCOPE_DEFBGRED     	 74
#define SCOPE_DEFBGGREEN   	 79
#define SCOPE_DEFBGBLUE    	 77
#define SCOPE_DEFGRRED       96
#define SCOPE_DEFGRGREEN     98
#define SCOPE_DEFGRBLUE      102
#define SCOPE_SELCOLOR       "#4a4f4d"  /* a bit lighter shade of blue */
#define SCOPE_SELBORDER      "#5aadef" /* border select color that seems to be in max */
#define SCOPE_FGWIDTH        0.7  /* line width is float */
#define SCOPE_GRIDWIDTH      0.9
#define SCOPE_SELBDWIDTH     3.0
#define SCOPEHANDLE_WIDTH    10    /* item size is int */
#define SCOPEHANDLE_HEIGHT   10
/* these are performance-related hacks, LATER investigate */
#define SCOPE_GUICHUNK  128
//#define SCOPE_GUICHUNKXY    32


typedef struct _scope
{
    t_object   x_obj;
    t_inlet   *x_rightinlet;
    t_glist   *x_glist;
    t_canvas  *x_canvas;  /* also an 'isvised' flag */
    char       x_tag[64];
    char       x_fgtag[64];
    char       x_bgtag[64];
    char       x_gridtag[64];
    char	   x_margintag[64];
    int        x_width;
    int        x_height;
    float      x_minval;
    float      x_maxval;
    int        x_delay;
    int        x_trigmode;
    float      x_triglevel;
    int		   x_drawstyle;
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
    int		   x_lastxymode;
//     float	  *x_xbuffer;
//     float	  *x_ybuffer;
    float      x_xbuffer[SCOPE_MAXBUFSIZE*4];
    float      x_ybuffer[SCOPE_MAXBUFSIZE*4];
    float	   x_xbuflast[SCOPE_MAXBUFSIZE*4];
    float	   x_ybuflast[SCOPE_MAXBUFSIZE*4];
    t_float	  *x_signalscalar;
    int        x_allocsize;
    int        x_bufsize;
    int		   x_lastbufsize;
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
static void scope_bufsize(t_scope *x, t_float bufsz);
EXTERN t_float *obj_findsignalscalar(t_object *x, int m);

static void scope_clear(t_scope *x, int withdelay)
{
    //x->x_bufphase = 0;
    //x->x_phase = 0;
    x->x_precount = (withdelay ? (int)(x->x_delay * x->x_ksr) : 0);
//     x->x_retrigger = (x->x_trigmode != SCOPE_TRIGLINEMODE);
//     x->x_trigx = x->x_triglevel;
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
   					memcpy(x->x_xbuflast, x->x_xbuffer, bufsize * sizeof(*x->x_xbuffer));
    				memcpy(x->x_ybuflast, x->x_ybuffer, bufsize * sizeof(*x->x_ybuffer));
    				x->x_retrigger = (x->x_trigmode != SCOPE_TRIGLINEMODE);
    				x->x_trigx = x->x_triglevel;
    				{
    				
    				}
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
    xfeeder = forky_hasfeeders((t_object *)x, x->x_glist, 0, &s_signal);
    yfeeder = forky_hasfeeders((t_object *)x, x->x_glist, 1, &s_signal);
    scope_setxymode(x, xfeeder + 2 * yfeeder);
	dsp_add(scope_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
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
	};
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
	if(bufsz < SCOPE_MINBUFSIZE){
		x->x_bufsize = SCOPE_MINBUFSIZE;
	}
	else if(bufsz > SCOPE_MAXBUFSIZE){
		x->x_bufsize = SCOPE_MAXBUFSIZE;
	}
	else{
		x->x_bufsize = bufsz;
	}
	pd_float((t_pd *)x->x_rightinlet, x->x_bufsize);
	x->x_phase = 0;
	x->x_bufphase = 0;
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
    }
}

static void scope_delay(t_scope *x, t_float del)
{
    if(del < SCOPE_MINDELAY){
		del = SCOPE_MINDELAY;
	}
	x->x_delay = del;
}

static void scope_drawstyle(t_scope *x, t_float drawstyle)
{
	x->x_drawstyle = (drawstyle == 0 ? 0 : 1);
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
	}
    x->x_trigmode = (int)trigmode;
    if (x->x_trigmode == SCOPE_TRIGLINEMODE){
		x->x_retrigger = 0;
	}
}

static void scope_triglevel(t_scope *x, t_float lvl)
{
    x->x_triglevel = lvl;
}

static void scope_fgcolor(t_scope *x, t_float fr, t_float fg, t_float fb)
{//scale is 0-1
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

	//scaling to 255 and rounding
	
	fr *= (float)SCOPE_MAXRGB;
	fr = round(fr);
	fg *= (float)SCOPE_MAXRGB;
	fg = round(fg);
	fb *= (float)SCOPE_MAXRGB;
	fb = round(fb);
	t_canvas *cv;
    x->x_fgred = (int)fr;
    x->x_fggreen = (int)fg;
    x->x_fgblue = (int)fb;
	cv = scope_isvisible(x);
    if (cv) {
		sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_fgtag, x->x_fgred, x->x_fggreen, x->x_fgblue);
	};
}


static void scope_frgb(t_scope *x, t_float fr, t_float fg, t_float fb)
{//scale is 0-255
	if(fr < SCOPE_MINRGB){
		fr = SCOPE_MINRGB;
	}
	else if(fr > SCOPE_MAXRGB){
		fr = SCOPE_MAXRGB;
	};
    if(fg < SCOPE_MINRGB){
		fg = SCOPE_MINRGB;
	}
	else if(fg > SCOPE_MAXRGB){
		fg = SCOPE_MAXRGB;
	};
	if(fb < SCOPE_MINRGB){
		fb = SCOPE_MINRGB;
	}
	else if(fb > SCOPE_MAXRGB){
		fb = SCOPE_MAXRGB;
	};

	t_canvas *cv;
    x->x_fgred = (int)fr;
    x->x_fggreen = (int)fg;
    x->x_fgblue = (int)fb;
	cv = scope_isvisible(x);
    if (cv) {
		sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_fgtag, x->x_fgred, x->x_fggreen, x->x_fgblue);
	};
}

static void scope_bgcolor(t_scope *x, t_float br, t_float bg, t_float bb)
{//scale is 0-1
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

	//scaling to 255 and rounding
	
	br *= (float)SCOPE_MAXRGB;
	br = round(br);
	bg *= (float)SCOPE_MAXRGB;
	bg = round(bg);
	bb *= (float)SCOPE_MAXRGB;
	bb = round(bb);
	t_canvas *cv;
    x->x_bgred = (int)br;
    x->x_bggreen = (int)bg;
    x->x_bgblue = (int)bb;
	cv = scope_isvisible(x);
    if (cv) {
		sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_bgtag, x->x_bgred, x->x_bggreen, x->x_bgblue);
	};
}



static void scope_brgb(t_scope *x, t_float br, t_float bg, t_float bb)
{//scale is 0-255
    if(br < SCOPE_MINRGB){
		br = SCOPE_MINRGB;
	}
	else if(br > SCOPE_MAXRGB){
		br = SCOPE_MAXRGB;
	};
    if(bg < SCOPE_MINRGB){
		bg = SCOPE_MINRGB;
	}
	else if(bg > SCOPE_MAXRGB){
		bg = SCOPE_MAXRGB;
	};
	if(bb < SCOPE_MINRGB){
		bb = SCOPE_MINRGB;
	}
	else if(bb > SCOPE_MAXRGB){
		bb = SCOPE_MAXRGB;
	};
	
	t_canvas *cv;
    x->x_bgred = (int)br;
    x->x_bggreen = (int)bg;
    x->x_bgblue = (int)bb;
	cv = scope_isvisible(x);
    if (cv) {
		sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_bgtag, x->x_bgred, x->x_bggreen, x->x_bgblue);
	};
}

static void scope_gridcolor(t_scope *x, t_float gr, t_float gg, t_float gb)
{//scale is 0-1
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

	//scaling to 255 and rounding
	
	gr *= (float)SCOPE_MAXRGB;
	gr = round(gr);
	gg *= (float)SCOPE_MAXRGB;
	gg = round(gg);
	gb *= (float)SCOPE_MAXRGB;
	gb = round(gb);
	t_canvas *cv;
    x->x_grred = (int)gr;
    x->x_grgreen = (int)gg;
    x->x_grblue = (int)gb;
	cv = scope_isvisible(x);
    if (cv) {
		sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_gridtag, x->x_grred, x->x_grgreen, x->x_grblue);
	};
}



static void scope_grgb(t_scope *x, t_float gr, t_float gg, t_float gb)
{  //scale 0-255 
	if(gr < SCOPE_MINRGB){
		gr = SCOPE_MINRGB;
	}
	else if(gr > SCOPE_MAXRGB){
		gr = SCOPE_MAXRGB;
	};
    if(gg < SCOPE_MINRGB){
		gg = SCOPE_MINRGB;
	}
	else if(gg > SCOPE_MAXRGB){
		gg = SCOPE_MAXRGB;
	};
	if(gb < SCOPE_MINRGB){
		gb = SCOPE_MINRGB;
	}
	else if(gb > SCOPE_MAXRGB){
		gb = SCOPE_MAXRGB;
	};
	

    t_canvas *cv;
    x->x_grred   = (int)gr;
    x->x_grgreen = (int)gg;
    x->x_grblue  = (int)gb;
	cv = scope_isvisible(x);
    if (cv) {
		sys_vgui(".x%lx.c itemconfigure %s -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_gridtag, x->x_grred, x->x_grgreen, x->x_grblue);
	};
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

	sys_vgui(".x%lx.c itemconfigure %s -outline %s  -width %f -fill #%2.2x%2.2x%2.2x\n",
		 cv, x->x_bgtag, SCOPE_SELBORDER, SCOPE_SELBDWIDTH, x->x_bgred, x->x_bggreen, x->x_bgblue);

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

static void scope_drawfg(t_scope *x, t_canvas *cv,
			     int x1, int y1, int x2, int y2)
{
    int i, xymode = x->x_lastxymode;
    float dx, dy, xx, yy, oldx, oldy, sc, xsc, ysc;
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
		sys_vgui("%d %d \\\n", (int)xx, (int)yy);
    }
    sys_vgui("-fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
	     x->x_fgred, x->x_fggreen, x->x_fgblue,
	     SCOPE_FGWIDTH, x->x_fgtag, x->x_tag);
}



static void scope_drawmargins(t_scope *x, t_canvas *cv,
		int x1, int y1, int x2, int y2)
{
    /* margin lines:  masking overflows, so that they appear as gaps,
       rather than clipped signal values, LATER rethink */
    sys_vgui(".x%lx.c create line %d %d\
 %d %d\
 %d %d\
 %d %d\
 %d %d\
 -fill #%2.2x%2.2x%2.2x -width %f -tags {%s %s}\n",
	     cv, x1, y1 , x2, y1, x2, y2, x1, y2, x1, y1,
	     x->x_bgred, x->x_bggreen, x->x_bgblue,
	     1., x->x_margintag, x->x_tag);
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

static void scope_draw(t_scope *x, t_canvas *cv)
{
    int x1, y1, x2, y2;
    scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
    scope_drawbg(x, cv, x1, y1, x2, y2);
    if (x->x_lastxymode)
    	scope_drawfg(x, cv, x1, y1, x2, y2);
    scope_drawmargins(x, cv, x1, y1, x2, y2);
}

static void scope_redraw(t_scope *x, t_canvas *cv)
{
	int bufsize;
    int nleft = bufsize = x->x_lastbufsize;
    //float *bp = x->x_xbuflast;
    char chunk[32 * SCOPE_GUICHUNK];  /* LATER estimate */
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
//     int xymode = x->x_xymode;
//     int bufsize = x->x_lastbufsize;
//     x->x_xymode = x->x_lastxymode;
//     x->x_xbuffer = x->x_xbuflast;
//     x->x_ybuffer = x->x_ybuflast;
	scope_draw(x, cv);
// 	x->x_xymode = xymode;
// 	x->x_bufsize = bufsize;
// 	x->x_xbuffer = x->x_xbuf;
// 	x->x_ybuffer = x->x_ybuf;
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
    int xymode = x->x_xymode;
    int bufsize = x->x_bufsize;
    x->x_xymode = x->x_lastxymode;
    x->x_bufsize = x->x_lastbufsize;
//     x->x_xbuffer = x->x_xbuflast;
//     x->x_ybuffer = x->x_ybuflast;
	scope_draw(x, cv);
	x->x_xymode = xymode;
	x->x_bufsize = bufsize;
// 	x->x_xbuffer = x->x_xbuf;
// 	x->x_ybuffer = x->x_ybuf;
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
	    sys_vgui(".x%lx.c delete %s %s\n", cv, x->x_fgtag, x->x_margintag);
	    
	    
		int x1, y1, x2, y2;
		scope_getrect((t_gobj *)x, x->x_glist, &x1, &y1, &x2, &y2);
		if (xymode)
			scope_drawfg(x, cv, x1, y1, x2, y2);
		scope_drawmargins(x, cv, x1, y1, x2, y2);
	    
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
		scope_redraw(x, cv);
    }
    scope_clear(x, 1);
}

static void scope_resize(t_scope *x, t_float w, t_float h)
{
    x->x_width  = (int)(w < SCOPE_MINWIDTH ? SCOPE_MINWIDTH : w);
    x->x_height = (int)(h < SCOPE_MINHEIGHT ? SCOPE_MINHEIGHT : h);
    if (x->x_xymode)
        scope_redraw(x, x->x_canvas);
    scope_revis(x, x->x_canvas);
    canvas_fixlinesfor(x->x_glist, (t_text *)x);
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
 -outline %s -width %f -tags %s\n",
		     cv, x1, y1, x2, y2, SCOPE_SELBORDER, SCOPE_SELBDWIDTH, sh->h_outlinetag);
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
    if (x->x_clock){
		clock_free(x->x_clock);
	};
    if (x->x_handle)
    {
		pd_unbind(x->x_handle, ((t_scopehandle *)x->x_handle)->h_bindsym);
		pd_free(x->x_handle);
    };
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
	

static void scope_properties(t_gobj *z, t_glist *owner)
{
    t_scope *x = (t_scope *)z;
    int bgcol, grcol, fgcol;
    
    bgcol = ((int)x->x_bgred << 16) + ((int)x->x_bggreen << 8) + (int)x->x_bgblue;
    grcol = ((int)x->x_grred << 16) + ((int)x->x_grgreen << 8) + (int)x->x_grblue;
    fgcol = ((int)x->x_fgred << 16) + ((int)x->x_fggreen << 8) + (int)x->x_fgblue;
    
    
    char buf[1000];
    //t_symbol *srl[3];

   // iemgui_properties(&x->x_gui, srl);
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
    //post("%s", buf);
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static int scope_getcolorarg(int index, int argc, t_atom *argv)
{
    if(index < 0 || index >= argc)
        return 0;
    if(IS_A_FLOAT(argv,index))
        return atom_getintarg(index, argc, argv);
    if(IS_A_SYMBOL(argv,index))
    {
        t_symbol*s=atom_getsymbolarg(index, argc, argv);
        if ('#' == s->s_name[0])
            return strtol(s->s_name+1, 0, 16);
    }
    return 0;
}

static void scope_dialog(t_scope *x, t_symbol *s, int argc, t_atom *argv)
{
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
	
	//post ("drawstyle %d", drawstyle);
	scope_period(x, period);
	scope_bufsize(x, bufsize);
	scope_range(x, minval, maxval);
	scope_delay(x, delay);
	scope_drawstyle(x, drawstyle);
	scope_trigger(x, trigmode);
	scope_triglevel(x, triglevel);
	if (x->x_width != width || x->x_height != height || 
		x->x_bgred != bgred || x->x_bggreen != bggreen || x->x_bgblue != bgblue ||
		x->x_grred != grred || x->x_grgreen != grgreen || x->x_grblue != grblue ||
		x->x_fgred != fgred || x->x_fggreen != fggreen || x->x_fgblue != fgblue)
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
	x->x_allocsize =  (int) SCOPE_DEFBUFSIZE;
    x->x_bufsize = 0;


	//int pastargs = 0;
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

	//default to using rgb methods but if color version
	//(instead of rgb) version is specified, set indicator flags
	//fcolset, bcolset, gcolset
	
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
		if(argv -> a_type == A_FLOAT){//if curarg is a number
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
		else if (argv -> a_type == A_SYMBOL){//curarg is a string
			t_symbol *curarg = atom_getsymbolarg(0, argc, argv);
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
			else if(strcmp(curarg->s_name, "@drawstyle") == 0){
				if(argc >= 2){
					drawstyle = atom_getfloatarg(1, argc, argv);
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
			else if(strcmp(curarg->s_name, "@grgb") == 0){
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
			else if(strcmp(curarg->s_name, "@fgcolor") == 0){
				if(argc >= 4){
					fcolset = 1;
					fcred = atom_getfloatarg(1, argc, argv);
					fcgreen = atom_getfloatarg(2, argc, argv);
					fcblue = atom_getfloatarg(3, argc, argv);
					argc-=4;
					argv+=4;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@bgcolor") == 0){
				if(argc >= 4){
					bcolset = 1;
					bcred = atom_getfloatarg(1, argc, argv);
					bcgreen = atom_getfloatarg(2, argc, argv);
					bcblue = atom_getfloatarg(3, argc, argv);
					argc-=4;
					argv+=4;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(curarg->s_name, "@gridcolor") == 0){
				if(argc >= 4){
					gcolset = 1;
					gcred = atom_getfloatarg(1, argc, argv);
					gcgreen = atom_getfloatarg(2, argc, argv);
					gcblue = atom_getfloatarg(3, argc, argv);
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
		}
		else{
			goto errstate;
		};
	};
	x->x_rightinlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    scope_dim(x, width, height);
	scope_period(x, period);
    /* CHECKME 6th argument (default 3 for mono, 1 for xy */
//     x->x_xbuffer = x->x_xbuf;
//     x->x_ybuffer = x->x_ybuf;
    scope_bufsize(x, bufsize);
    
    x->x_signalscalar = obj_findsignalscalar(x, 1);
    scope_range(x, minval, maxval);
    scope_delay(x, delay);
    scope_drawstyle(x, drawstyle);
    /* CHECKME 11th argument (default 0.) */
    scope_trigger(x, trigger);
    scope_triglevel(x, triglevel);
    /* CHECKME last argument (default 0) */
	scope_frgb(x, fgred, fggreen, fgblue);
	scope_brgb(x, bgred, bggreen, bgblue);
	scope_grgb(x, grred, grgreen, grblue);

	//now to see if we're calling the color versions
	if(fcolset){
		scope_fgcolor(x, fcred, fcgreen, fcblue);
	};
	if(bcolset){
		scope_bgcolor(x, bcred, bcgreen, bcblue);
	};
	if(gcolset){
		scope_gridcolor(x, gcred, gcgreen, gcblue);
	};

	
    sprintf(x->x_tag, "all%lx", (unsigned long)x);
    sprintf(x->x_bgtag, "bg%lx", (unsigned long)x);
    sprintf(x->x_gridtag, "gr%lx", (unsigned long)x);
    sprintf(x->x_fgtag, "fg%lx", (unsigned long)x);
    sprintf(x->x_margintag, "ma%lx", (unsigned long)x);
    x->x_xymode = 0;
    x->x_ksr = sys_getsr() * 0.001;  /* redundant */
    x->x_frozen = 0;
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
		pd_error(x, "scope~: improper creation arguments");
		return NULL;
}

void scope_tilde_setup(void)
{
	#include "scope_dialog.c"
	t_symbol *dirsym;
    scope_class = class_new(gensym("scope~"),
			    (t_newmethod)scope_new,
			    (t_method)scope_free,
			    sizeof(t_scope), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)scope_new, gensym("Scope~"), A_GIMME, 0); // back compatible
    class_addcreator((t_newmethod)scope_new, gensym("cyclone/Scope~"), A_GIMME, 0); // back compatible
    class_addmethod(scope_class, (t_method)scope_dialog, gensym("dialog"),
    				A_GIMME, 0);
    class_addmethod(scope_class, nullfn, gensym("signal"), 0);
    class_addmethod(scope_class, (t_method) scope_dsp, gensym("dsp"), 0);
    class_addfloat(scope_class, (t_method)scope_float);
	class_addmethod(scope_class, (t_method)scope_period, gensym("calccount"),
                    A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_bufsize, gensym("bufsize"),
                    A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_resize, gensym("dim"),
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_range, gensym("range"),
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_delay, gensym("delay"),
                    A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_drawstyle, gensym("drawstyle"),
                    A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_trigger, gensym("trigger"),
                    A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_triglevel, gensym("triglevel"),
                    A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_frgb, gensym("frgb"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_brgb, gensym("brgb"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_grgb, gensym("grgb"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_fgcolor, gensym("fgcolor"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_bgcolor, gensym("bgcolor"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_gridcolor, gensym("gridcolor"),
                    A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(scope_class, (t_method)scope_resize, gensym("resize"),
                    A_FLOAT, A_FLOAT, 0);

    
    class_setwidget(scope_class, &scope_widgetbehavior);
    
    forky_setsavefn(scope_class, scope_save);
    
    scopehandle_class = class_new(gensym("_scopehandle"), 0, 0, sizeof(t_scopehandle), CLASS_PD, 0);
    class_addmethod(scopehandle_class, (t_method)scopehandle__clickhook, gensym("_click"),
                    A_FLOAT, 0);
    class_addmethod(scopehandle_class, (t_method)scopehandle__motionhook, gensym("_motion"),
                    A_FLOAT, A_FLOAT, 0);
    fitter_setup(scope_class, 0);
    class_setpropertiesfn(scope_class, scope_properties);
    dirsym = scope_class->c_externdir;
    //sys_vgui("source {%s/dialog_scope.tcl}\n", dirsym->s_name);
    

}

void Scope_tilde_setup(void)
{
	
    scope_tilde_setup();
}
