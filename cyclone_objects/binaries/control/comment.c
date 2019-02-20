/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* FIXME creation lag (X-specific) */
/* LATER think about pushing text to the text editor (ctrl-t)
 -- not easy, because we are not 'textedfor' */
/* LATER think about making the <Button> binding for the entire bbox,
 instead of the text item, to ease the pain of resizing, somewhat. */

#include <string.h>
#include <ctype.h>
#include "m_pd.h"
//#include <common/api.h>
#include "g_canvas.h"

/* our proxy of the text_class (not in the API), LATER do not cheat */
static t_class *makeshift_class;

/* #ifdef KRZYSZCZ
 #define COMMENT_DEBUG
 #endif */

#define COMMENT_LMARGIN     1
#define COMMENT_RMARGIN     1
#define COMMENT_TMARGIN     2
#define COMMENT_BMARGIN     2
#define COMMENT_MINWIDTH    8
#define COMMENT_HANDLEWIDTH 8
#define COMMENT_OUTBUFSIZE  1000
#define COMMENT_NUMCOLORS   3

typedef struct _comment{
    t_object   x_ob;
    t_glist   *x_glist;
    t_canvas  *x_canvas;
    t_symbol  *x_bindsym;
    char       x_tag[32];
    char       x_texttag[32];
    char       x_outlinetag[32];
    t_clock   *x_transclock;
    t_binbuf  *x_binbuf;
    char      *x_textbuf;
    int        x_textbufsize;
    int        x_pixwidth;
    int        x_bbset;
    int        x_bbpending;
    int        x_x1;
    int        x_y1;
    int        x_x2;
    int        x_y2;
    int        x_newx2;
    int        x_dragon;
    int        x_fontsize;    // requested size
    t_symbol  *x_fontfamily;  // requested family
    int        x_fontprops;   // 0: reg, 1: bold, 2: italic, 3: bolditalic (unused)
    t_symbol  *x_encoding;    // requested encoding (unused)
    unsigned char  x_red;
    unsigned char  x_green;
    unsigned char  x_blue;
    char       x_color[8];
    int        x_selstart;
    int        x_selend;
    int        x_active;
    int        x_ready;
    t_symbol  *x_receive_sym;
    t_symbol  *x_selector;
/* new args that currently do nothing - DK 2017
    t_float     x_bgcolor[COMMENT_NUMCOLORS];
    int         x_textjust; //0: left, 1: center, 2: right
    int         x_underline; //0: no, 1: yes
    int         x_suppressinlet; //0: no, 1: yes */
}t_comment;

static t_class *comment_class;
static t_class *commentsink_class;

static t_pd *commentsink = 0;

static void comment_receive(t_comment *x, t_symbol *s){
    if(s != &s_){
        if(x->x_receive_sym != &s_) pd_unbind(&x->x_ob.ob_pd, x->x_receive_sym);
        pd_bind(&x->x_ob.ob_pd, x->x_receive_sym = s);
    }
}

static void comment_draw(t_comment *x){
    char buf[COMMENT_OUTBUFSIZE], *outbuf, *outp;
    unsigned long cv = (unsigned long)x->x_canvas;
    int reqsize = x->x_textbufsize + 250;  // FIXME estimation
    if(reqsize > COMMENT_OUTBUFSIZE){ // <= seems unnecessary (porres)
        // #ifdef COMMENT_DEBUG
        // post("allocating %d outbuf bytes", reqsize);
         // #endif
        if(!(outbuf = getbytes(reqsize)))
            return;
    }
    outp = outbuf = buf;
    sprintf(outp, "comment_draw %s .x%lx.c %s %s %f %f %s -%d %s %s {%.*s} %d\n",
            x->x_bindsym->s_name, cv, x->x_texttag, x->x_tag,
            (float)(text_xpix((t_text *)x, x->x_glist) + COMMENT_LMARGIN),
            (float)(text_ypix((t_text *)x, x->x_glist) + COMMENT_TMARGIN),
            x->x_fontfamily->s_name, x->x_fontsize,
            (glist_isselected(x->x_glist, &x->x_glist->gl_gobj) ? "blue" : x->x_color),
            "\"\"", // encoding
            x->x_textbufsize, x->x_textbuf, x->x_pixwidth);
    x->x_bbpending = 1;
    sys_gui(outbuf);
    if(outbuf != buf) freebytes(outbuf, reqsize);
}

static void comment_update(t_comment *x){
    char buf[COMMENT_OUTBUFSIZE], *outbuf, *outp;
    unsigned long cv = (unsigned long)x->x_canvas;
    int reqsize = x->x_textbufsize + 250;  // FIXME estimation
    if (reqsize > COMMENT_OUTBUFSIZE){ // <= seems unnecessary (porres)
        /* #ifdef COMMENT_DEBUG
         post("allocating %d outbuf bytes", reqsize);
         #endif */
        if(!(outbuf = getbytes(reqsize)))
            return;
    }
    outp = outbuf = buf;
    sprintf(outp, "comment_update .x%lx.c %s %s {%.*s} %d\n", cv,
            x->x_texttag, (x->x_encoding ? x->x_encoding->s_name : "\"\""),
            x->x_textbufsize, x->x_textbuf, x->x_pixwidth);
    outp += strlen(outp);
    if(x->x_active){
        if(x->x_selend > x->x_selstart){
            sprintf(outp, ".x%lx.c select from %s %d\n", cv, x->x_texttag, x->x_selstart);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c select to %s %d\n", cv, x->x_texttag, x->x_selend);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c focus {}\n", cv);
        }
        else{
            sprintf(outp, ".x%lx.c select clear\n", cv);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c icursor %s %d\n", cv, x->x_texttag, x->x_selstart);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c focus %s\n", cv, x->x_texttag);
        }
        outp += strlen(outp);
    }
    sprintf(outp, "comment_bbox %s .x%lx.c %s\n",
            x->x_bindsym->s_name, cv, x->x_texttag);
    x->x_bbpending = 1;
    sys_gui(outbuf);
    if (outbuf != buf) freebytes(outbuf, reqsize);
}

static void comment_validate(t_comment *x, t_glist *glist){
    if(!x->x_ready){
        t_text *t = (t_text *)x;
        binbuf_free(t->te_binbuf);
        t->te_binbuf = x->x_binbuf;
        if(x->x_textbuf) freebytes(x->x_textbuf, x->x_textbufsize);
        binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
        x->x_ready = 1;
        /* #ifdef COMMENT_DEBUG
         post("validation done");
         #endif */
    }
    if(glist){
        if(glist != x->x_glist){
            post("bug [comment]: comment_getcanvas");
            x->x_glist = glist;
        }
        x->x_canvas = glist_getcanvas(glist);
    }
}

static void comment_grabbedkey(void *z, t_floatarg f){
    z = NULL;
    f = 0;  /* LATER think about replacing #key binding/comment_float() with grabbing */
    /* #ifdef COMMENT_DEBUG
     post("comment_grabbedkey %g", f);
     #endif */
}

static void comment_dograb(t_comment *x){
    /* LATER investigate the grabbing feature.
     Here we use it just to prevent backspace from erasing entire text.
     This has to be done also when we are already active, because
     after being clicked at we have lost our previous grab. */
    glist_grab(x->x_glist, (t_gobj *)x, 0, comment_grabbedkey, 0, 0);
}

static void comment__bboxhook(t_comment *x, t_symbol *bindsym,
    t_floatarg x1, t_floatarg y1, t_floatarg x2, t_floatarg y2){
    bindsym = NULL;
    /* #ifdef COMMENT_DEBUG
     post("bbox %g %g %g %g", x1, y1, x2, y2);
     #endif */
    x->x_x1 = x1;
    x->x_y1 = y1;
    x->x_x2 = x2;
    x->x_y2 = y2;
    x->x_bbset = 1;
    x->x_bbpending = 0;
}

static void comment__clickhook(t_comment *x, t_symbol *s, int ac, t_atom *av){
    t_symbol *dummy = s;
    dummy = NULL;
    int xx, yy, ndx;
    if(ac == 8 && av->a_type == A_SYMBOL
       && av[1].a_type == A_FLOAT && av[2].a_type == A_FLOAT
       && av[3].a_type == A_FLOAT
       && av[4].a_type == A_FLOAT && av[5].a_type == A_FLOAT
       && av[6].a_type == A_FLOAT && av[7].a_type == A_FLOAT){
        xx = (int)av[1].a_w.w_float;
        yy = (int)av[2].a_w.w_float;
        ndx = (int)av[3].a_w.w_float;
        comment__bboxhook(x, av->a_w.w_symbol,
                          av[4].a_w.w_float, av[5].a_w.w_float,
                          av[6].a_w.w_float, av[7].a_w.w_float);
    }
    else{
        post("bug [comment]: comment__clickhook");
        return;
    }
    if(x->x_glist->gl_edit){
        if(x->x_active){
            if(ndx >= 0 && ndx < x->x_textbufsize){
                // set selection, LATER shift-click and drag
                x->x_selstart = x->x_selend = ndx;
                comment_dograb(x);
                comment_update(x);
            }
        }
        else if(xx > x->x_x2 - COMMENT_HANDLEWIDTH){ // start resizing
            char buf[COMMENT_OUTBUFSIZE], *outp = buf;
            unsigned long cv = (unsigned long)x->x_canvas;
            sprintf(outp, ".x%lx.c bind %s <ButtonRelease> \
                    {pdsend {%s _release %s}}\n", cv, x->x_texttag,
                    x->x_bindsym->s_name, x->x_bindsym->s_name);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c bind %s <Motion> \
                    {pdsend {%s _motion %s %%x %%y}}\n", cv, x->x_texttag,
                    x->x_bindsym->s_name, x->x_bindsym->s_name);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c create rectangle %d %d %d %d -outline blue \
                    -tags {%s %s}\n",
                    cv, x->x_x1, x->x_y1, x->x_x2, x->x_y2,
                    x->x_outlinetag, x->x_tag);
            sys_gui(buf);
            x->x_newx2 = x->x_x2;
            x->x_dragon = 1;
        }
    }
}

static void comment__releasehook(t_comment *x, t_symbol *bindsym){
    bindsym = NULL;
    unsigned long cv = (unsigned long)x->x_canvas;
    sys_vgui(".x%lx.c bind %s <ButtonRelease> {}\n", cv, x->x_texttag);
    sys_vgui(".x%lx.c bind %s <Motion> {}\n", cv, x->x_texttag);
    sys_vgui(".x%lx.c delete %s\n", cv, x->x_outlinetag);
    x->x_dragon = 0;
    if(x->x_newx2 != x->x_x2){
        x->x_pixwidth = x->x_newx2 - x->x_x1;
        x->x_x2 = x->x_newx2;
        comment_update(x);
    }
}

static void comment__motionhook(t_comment *x, t_symbol *bindsym, t_floatarg xx, t_floatarg yy){
    bindsym = NULL;
    yy = 0;
    unsigned long cv = (unsigned long)x->x_canvas;
    if(xx > x->x_x1 + COMMENT_MINWIDTH)
        sys_vgui(".x%lx.c coords %s %d %d %d %d\n", cv, x->x_outlinetag,
            x->x_x1, x->x_y1, x->x_newx2 = xx, x->x_y2);
}

static void commentsink__bboxhook(t_pd *x, t_symbol *bindsym,
t_floatarg x1, t_floatarg y1, t_floatarg x2, t_floatarg y2){
    x1 = x2 = y1 = y2 = 0;
    if(bindsym->s_thing == x){
        pd_unbind(x, bindsym);  // if comment gone, unbind
        /* #ifdef COMMENT_DEBUG
         post("sink: %s unbound", bindsym->s_name);
         #endif */
    }
}

static void commentsink_anything(t_pd *x, t_symbol *s, int ac, t_atom *av){
    // nop (just avoid Pd warnings)
    x = NULL;
    s = NULL;
    ac = (int)av;
}

static void comment_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2){
    t_comment *x = (t_comment *)z;
    int width,  height;
    float x1, y1, x2, y2;
    comment_validate(x, glist);
    if((width = x->x_pixwidth) < 1) // FIXME estimation
        width = x->x_fontsize * x->x_textbufsize;
    width += COMMENT_LMARGIN + COMMENT_RMARGIN;
    // FIXME estimation
    height = x->x_fontsize + COMMENT_TMARGIN + COMMENT_BMARGIN;
    x1 = text_xpix((t_text *)x, glist);
    y1 = text_ypix((t_text *)x, glist) + 1;  // LATER revisit
    x2 = x1 + width;
    y2 = y1 + height - 2;  // LATER revisit
    // #ifdef COMMENT_DEBUG
    // post("estimated rectangle: %g %g %g %g", x1, y1, x2, y2);
    // #endif
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void comment_displace(t_gobj *z, t_glist *glist, int dx, int dy){
    t_comment *x = (t_comment *)z;
    if(!x->x_active && !x->x_dragon){  // LATER rethink
        t_text *t = (t_text *)z;
        comment_validate(x, glist);
        t->te_xpix += dx;
        t->te_ypix += dy;
        if(x->x_bbset){
            x->x_x1 += dx;
            x->x_y1 += dy;
            x->x_x2 += dx;
            x->x_y2 += dy;
        }
        if(glist_isvisible(glist)){
            t_canvas *cv = glist_getcanvas(glist);
            sys_vgui(".x%lx.c move %s %d %d\n", x->x_canvas, x->x_tag, dx, dy);
            canvas_fixlinesfor(cv, t);
        }
    }
}

static void comment_activate(t_gobj *z, t_glist *glist, int state){
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if(state){
        comment_dograb(x);
        if (x->x_active)
            return;
        sys_vgui(".x%lx.c focus %s\n", x->x_canvas, x->x_texttag);
        x->x_selstart = 0;
        x->x_selend = x->x_textbufsize;
        x->x_active = 1;
        pd_bind((t_pd *)x, gensym("#key"));
        pd_bind((t_pd *)x, gensym("#keyname"));
    }
    else{
        if (!x->x_active)
            return;
        pd_unbind((t_pd *)x, gensym("#key"));
        pd_unbind((t_pd *)x, gensym("#keyname"));
        sys_vgui("selection clear .x%lx.c\n", x->x_canvas);
        sys_vgui(".x%lx.c focus {}\n", x->x_canvas);
        x->x_active = 0;
    }
    comment_update(x);
}

static void comment_select(t_gobj *z, t_glist *glist, int state){
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if(!state && x->x_active) comment_activate(z, glist, 0);
    sys_vgui(".x%lx.c itemconfigure %s -fill %s\n", x->x_canvas,
        x->x_texttag, (state ? "blue" : x->x_color));
    /* A regular rtext should now set 'canvas_editing' variable to its canvas,
     but we do not do that, because we get the keys through a global binding
     to "#key" (and because 'canvas_editing' is not exported). */
}

static void comment_delete(t_gobj *z, t_glist *glist){
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void comment_vis(t_gobj *z, t_glist *glist, int vis){
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if(vis) comment_draw(x);
    else sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
}

/*
static void iemgui_init_sym2dollararg(t_iemgui *iemgui, t_symbol **symp,
                                      int indx, t_symbol *fallback)
{
    if(!*symp){
        t_binbuf *b = iemgui->x_obj.ob_binbuf;
        if (binbuf_getnatom(b) > indx){
            char buf[80];
            atom_string(binbuf_getvec(b) + indx, buf, 80);
            *symp = gensym(buf);
        }
        else if (fallback)
            *symp = fallback;
        else *symp = gensym("empty");
    }
}*/
 

static void comment_save(t_gobj *z, t_binbuf *b){
    t_comment *x = (t_comment *)z;
    t_text *t = (t_text *)x;
    comment_validate(x, 0);
    t_symbol *receive = x->x_receive_sym;
/*    t_binbuf *binbuf = x->x_ob.ob_binbuf;
    char buf[80];
    t_int i = 3;
    atom_string(binbuf_getvec(binbuf) + i, buf, 80);
    atom_string(binbuf_getvec(b) + i, buf, 80);
    receive = gensym(buf);*/
    if(receive == &s_) receive = gensym("?");
    binbuf_addv(b, "ssiisiissiiii",
                gensym("#X"),
                gensym("obj"),
                (int)t->te_xpix,
                (int)t->te_ypix, x->x_selector,
                x->x_pixwidth,
                x->x_fontsize,
                x->x_fontfamily,
                receive,
                x->x_fontprops,
                (int)x->x_red,
                (int)x->x_green,
                (int)x->x_blue);
    binbuf_addbinbuf(b, t->te_binbuf); // the actual comment
    binbuf_addv(b, ";");
}

static t_widgetbehavior comment_widgetbehavior ={
    comment_getrect,
    comment_displace,
    comment_select,
    comment_activate,
    comment_delete,
    comment_vis,
    0, // click
};

// this fires if a transform request was sent to a symbol we are bound to
static void comment_transtick(t_comment *x){
    glist_delete(x->x_glist, (t_gobj *)x);
}

// what follows is basically the original code of rtext_key()
static void comment_float(t_comment *x, t_float f){
    if(x->x_active){
        int keynum = (int)f;
        if(keynum){
            int i, newsize, ndel;
            int n = keynum;
            if (n == '\r') n = '\n';
            if (n == '\b'){
                if ((!x->x_selstart) && (x->x_selend == x->x_textbufsize)){
                    /* LATER delete the box... this causes reentrancy
                     problems now. */
                    /* glist_delete(x->x_glist, &x->x_text->te_g); */
                    goto donefloat;
                }
                else if (x->x_selstart && (x->x_selstart == x->x_selend))
                    x->x_selstart--;
            }
            ndel = x->x_selend - x->x_selstart;
            for (i = x->x_selend; i < x->x_textbufsize; i++)
                x->x_textbuf[i- ndel] = x->x_textbuf[i];
            newsize = x->x_textbufsize - ndel;
            x->x_textbuf = resizebytes(x->x_textbuf, x->x_textbufsize, newsize);
            x->x_textbufsize = newsize;
            if(n == '\n' || !iscntrl(n)){
                /* #ifdef COMMENT_DEBUG
                 post("%d accepted", n);
                 #endif */
                newsize = x->x_textbufsize+1;
                x->x_textbuf = resizebytes(x->x_textbuf, x->x_textbufsize, newsize);
                for(i = x->x_textbufsize; i > x->x_selstart; i--)
                    x->x_textbuf[i] = x->x_textbuf[i-1];
                x->x_textbuf[x->x_selstart] = n;
                x->x_textbufsize = newsize;
                x->x_selstart = x->x_selstart + 1;
            }
            /* #ifdef COMMENT_DEBUG
             else post("%d rejected", n);
             #endif */
            x->x_selend = x->x_selstart;
            x->x_glist->gl_editor->e_textdirty = 1;
            binbuf_text(x->x_binbuf, x->x_textbuf, x->x_textbufsize);
            comment_update(x);
        }
    }
    else post("bug [comment]: comment_float");
donefloat:;
    /* #ifdef COMMENT_DEBUG
     post("donefloat");
     #endif */
}

static void comment_list(t_comment *x, t_symbol *s, int ac, t_atom *av){
    t_symbol *dummy = s;
    dummy = NULL;
    if (!x->x_active)
        post("bug [comment]: comment_list");
    else if(ac > 1 && av->a_type == A_FLOAT && (int)av->a_w.w_float && av[1].a_type == A_SYMBOL){
        t_symbol *keysym = av[1].a_w.w_symbol;
        if (!strcmp(keysym->s_name, "Right")){
            if (x->x_selend == x->x_selstart &&
                x->x_selstart < x->x_textbufsize)
                x->x_selend = x->x_selstart = x->x_selstart + 1;
            else
                x->x_selstart = x->x_selend;
        }
        else if (!strcmp(keysym->s_name, "Left")){
            if (x->x_selend == x->x_selstart && x->x_selstart > 0)
                x->x_selend = x->x_selstart = x->x_selstart - 1;
            else
                x->x_selend = x->x_selstart;
        }
        // this should be improved...  life's too short
        else if (!strcmp(keysym->s_name, "Up")){
            if (x->x_selstart)
                x->x_selstart--;
            while (x->x_selstart > 0 && x->x_textbuf[x->x_selstart] != '\n')
                x->x_selstart--;
            x->x_selend = x->x_selstart;
        }
        else if (!strcmp(keysym->s_name, "Down")){
            while (x->x_selend < x->x_textbufsize &&
                   x->x_textbuf[x->x_selend] != '\n')
                x->x_selend++;
            if (x->x_selend < x->x_textbufsize)
                x->x_selend++;
            x->x_selstart = x->x_selend;
        }
        else if (!strcmp(keysym->s_name, "F4")){
            t_text *newt, *oldt = (t_text *)x;
            t_binbuf *bb = binbuf_new();
            int argc = binbuf_getnatom(x->x_binbuf);
            binbuf_addv(bb, "siissiiii", x->x_selector, x->x_pixwidth,
                        x->x_fontsize, x->x_fontfamily,
                        (x->x_receive_sym != &s_ ? x->x_receive_sym : gensym("?")),
                        x->x_fontprops,
                        (int)x->x_red, (int)x->x_green, (int)x->x_blue);
            binbuf_add(bb, argc, binbuf_getvec(x->x_binbuf));
            canvas_setcurrent(x->x_glist);
            newt = (t_text *)pd_new(makeshift_class);
            newt->te_width = 0;
            newt->te_type = T_OBJECT;
            newt->te_binbuf = bb;
            newt->te_xpix = oldt->te_xpix;
            newt->te_ypix = oldt->te_ypix;
            glist_add(x->x_glist, &newt->te_g);
            glist_noselect(x->x_glist);
            glist_select(x->x_glist, &newt->te_g);
            gobj_activate(&newt->te_g, x->x_glist, 1);
            x->x_glist->gl_editor->e_textdirty = 1;  // force evaluation
            canvas_unsetcurrent(x->x_glist);
            canvas_dirty(x->x_glist, 1);
            clock_delay(x->x_transclock, 0);  // LATER rethink
            goto donelist;
        }
        else if (!strcmp(keysym->s_name, "F5")){
            t_text *t = (t_text *)x;
            t_binbuf *bb = binbuf_new();
            int argc = binbuf_getnatom(x->x_binbuf);
            binbuf_addv(bb, "ii", (int)t->te_xpix + 5, (int)t->te_ypix + 5);
            binbuf_add(bb, argc, binbuf_getvec(x->x_binbuf));
            canvas_setcurrent(x->x_glist);
            typedmess((t_pd *)x->x_glist, gensym("text"),
                      argc + 2, binbuf_getvec(bb));
            canvas_unsetcurrent(x->x_glist);
            canvas_dirty(x->x_glist, 1);
            binbuf_free(bb);
            goto donelist;
        }
        else goto donelist;
        comment_update(x);
    }
donelist:;
    /* #ifdef COMMENT_DEBUG
     post("donelist");
     #endif */
}

static void comment_free(t_comment *x){
    if (x->x_active){
        pd_unbind((t_pd *)x, gensym("#key"));
        pd_unbind((t_pd *)x, gensym("#keyname"));
    }
    if(x->x_receive_sym != &s_) pd_unbind(&x->x_ob.ob_pd, x->x_receive_sym);
    if (x->x_transclock) clock_free(x->x_transclock);
    if(x->x_bindsym){
        pd_unbind((t_pd *)x, x->x_bindsym);
        if(!x->x_bbpending)
            pd_unbind(commentsink, x->x_bindsym);
    }
    if(x->x_binbuf && !x->x_ready) binbuf_free(x->x_binbuf);
    if(x->x_textbuf) freebytes(x->x_textbuf, x->x_textbufsize);
}

static void comment_append(t_comment *x, t_symbol *s, int argc, t_atom * argv){
    t_symbol *dummy = s;
    dummy = NULL;
    t_binbuf *bb = binbuf_new();
    binbuf_restore(bb, argc, argv);
    binbuf_addbinbuf(x->x_binbuf, bb);
    t_text *t = (t_text *)x;
    t->te_binbuf = x->x_binbuf;
    binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
    sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
    canvas_dirty(x->x_glist, 1);
    comment_draw(x);
}

static void comment_prepend(t_comment *x, t_symbol *s, int argc, t_atom * argv){
    t_symbol *dummy = s;
    dummy = NULL;
    t_binbuf *bb = binbuf_new();
    binbuf_restore(bb, argc, argv);
    binbuf_addbinbuf(bb, x->x_binbuf);
    binbuf_clear(x->x_binbuf);
    binbuf_addbinbuf(x->x_binbuf, bb);
    t_text *t = (t_text *)x;
    t->te_binbuf = x->x_binbuf;
    binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
    sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
    canvas_dirty(x->x_glist, 1);
    comment_draw(x);
}

static void comment_set(t_comment *x, t_symbol *s, int argc, t_atom * argv){
    t_symbol *dummy = s;
    dummy = NULL;
    binbuf_clear(x->x_binbuf);
    binbuf_restore(x->x_binbuf, argc, argv);
    t_text *t = (t_text *)x;
    t->te_binbuf = x->x_binbuf;
    binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
    sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
    canvas_dirty(x->x_glist, 1);
    comment_draw(x);
}

static void comment_textcolor(t_comment *x, t_floatarg r, t_floatarg g, t_floatarg b){
    x->x_red = (unsigned char)r;
    x->x_green = (unsigned char)g;
    x->x_blue = (unsigned char)b;
    sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
    comment_update(x);
    sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
    canvas_dirty(x->x_glist, 1);
    comment_draw(x);
}

static void comment_fontname(t_comment *x, t_symbol *name){
    x->x_fontfamily = name;
    comment_update(x);
    sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
    canvas_dirty(x->x_glist, 1);
    comment_draw(x);
}

static void comment_fontsize(t_comment *x, t_floatarg f){
    x->x_fontsize = (int)f < 5 ? 5 : (int)f;
    comment_update(x);
    sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
    canvas_dirty(x->x_glist, 1);
    comment_draw(x);
}

/* placeholders
static void comment_bgcolor(t_comment *x, t_float f1, t_float f2, t_float f3){
    x->x_bgcolor[0] = f1;
    x->x_bgcolor[1] = f2;
    x->x_bgcolor[2] = f3;
}

static void comment_fontface(t_comment *x, t_float f) // testing{
    x->x_fontprops = f;
}

static void comment_textjustification(t_comment *x, t_float f){
    x->x_textjust= f < 0 ? 0 : (f > 2 ? 2 : (int)f);
}

static void comment_underline(t_comment *x, t_float f){
    x->x_underline = f == 0 ? 0 : 1;
}

static void comment_suppressinlet(t_comment *x, t_float f){
    x->x_suppressinlet = f == 0 ? 0 : 1;
} */

/* properites: not yet
static void comment_properties(t_gobj *z, t_glist *owner){
    t_glist *dummy = owner;
    dummy = NULL;
    t_comment *x = (t_comment *)z;
    int color = 0;
    color = ((int)x->x_red << 16) + ((int)x->x_green << 8) + (int)x->x_blue;
    char buf[1000];
    sprintf(buf, "::dialog_comment::pdtk_comment_dialog %%s \
            trg %s tmd: %d tlv: \
            #%06x\n",
            x->x_fontfamily->s_name, x->x_fontsize, color);
    post("%s", buf);
//    gfxstub_new(&x->x_ob.ob_pd, x, buf);
} */

static void comment_attrparser(t_comment *x, int argc, t_atom * argv){
    t_atom* comlist = t_getbytes(argc * sizeof(*comlist));
    int i, comlen = 0; //eventual length of comment list comlist
    for(i = 0; i < argc; i++){
        if(argv[i].a_type == A_FLOAT){
            SETFLOAT(&comlist[comlen], argv[i].a_w.w_float);
            comlen++;
        }
        else if(argv[i].a_type == A_SYMBOL){
            t_symbol * cursym = argv[i].a_w.w_symbol;
            if(!strcmp(cursym->s_name, "@fontsize")){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        int fontsize = (int)argv[i].a_w.w_float;
                        x->x_fontsize = fontsize;
                    }
                    else i--;
                };
            }
            else if(!strcmp(cursym->s_name, "@fontname")){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_SYMBOL) x->x_fontfamily = argv[i].a_w.w_symbol;
                    else i--;
                };
            }
            else if(!strcmp(cursym->s_name, "@receive")){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_SYMBOL)
                        comment_receive(x, atom_getsymbolarg(i, argc, argv));
                    else i--;
                };
            }
            else if(!strcmp(cursym->s_name, "@textcolor")){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        int rgb = (unsigned char)argv[i].a_w.w_float;
                        x->x_red = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
                    }
                    else i--;
                };
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        i++;
                        int rgb = (unsigned char)argv[i].a_w.w_float;
                        x->x_green = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
                    }
                    else i--;
                };
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        i++;
                        int rgb = (unsigned char)argv[i].a_w.w_float;
                        x->x_blue = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
                    }
                    else i--;
                };
            }
            else if(!strcmp(cursym->s_name, "@text")){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_SYMBOL){
                        SETSYMBOL(&comlist[comlen], argv[i].a_w.w_symbol);
                        comlen++;
                    }
                    else i--;
                };
            }
/*            else if(strcmp(cursym->s_name, "@bgcolor") == 0){ // new attributes
                i++; //done with symbol move on
                int numcolors = COMMENT_NUMCOLORS; //number of colors to specify
                while(numcolors){
                    if((argc-i) > 0){ //if there are args left to parse
                        if(argv[i].a_type == A_FLOAT){
                            x->x_bgcolor[COMMENT_NUMCOLORS-numcolors] = argv[i].a_w.w_float;
                            if(numcolors > 1) i++; //don't need to increment the last time
                            //taken care of by for loop
                            numcolors--;
                        }
                        else{
                            i--; //the next arg wasn't a float so we didn't have to parse it
                            //leave it for the next go around in the for loop
                            break;
                        };
                    };
                };
            }
            else if(strcmp(cursym->s_name, "@fontface") == 0){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        int fontface = (int)argv[i].a_w.w_float;
                        x->x_fontprops = fontface < 0 ? 0 : (fontface > 3 ? 3 : fontface);
                    }
                    else i--;
                };
            }
            else if(strcmp(cursym->s_name, "@textjustification") == 0){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        int textjust = (int)argv[i].a_w.w_float;
                        x->x_textjust = textjust < 0 ? 0 : (textjust > 2 ? 2 : textjust);
                    }
                    else i--;
                };
            }
            else if(strcmp(cursym->s_name, "@underline") == 0){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        int underline = (int)argv[i].a_w.w_float;
                        x->x_underline = underline == 0 ? 0 : 1;
                    }
                    else
                        i--;
                };
            }
            else if(strcmp(cursym->s_name, "@suppressinlet") == 0){
                i++;
                if((argc-i) > 0){
                    if(argv[i].a_type == A_FLOAT){
                        int suppressinlet = (int)argv[i].a_w.w_float;
                        x->x_suppressinlet = suppressinlet == 0 ? 0 : 1;
                    }
                    else
                        i--;
                };
            } */
            else{ // treat it as a part of comlist
                SETSYMBOL(&comlist[comlen], cursym);
                comlen++;
            };
        };
    };
    if(comlen) //set the comment with comlist
        binbuf_restore(x->x_binbuf, comlen, comlist);
    else{
        SETSYMBOL(&comlist[0], gensym("comment"));
        binbuf_restore(x->x_binbuf, 1, comlist);
    };
    t_freebytes(comlist, argc * sizeof(*comlist));
}

static void *comment_new(t_symbol *s, int ac, t_atom *av){
    t_comment *x = (t_comment *)pd_new(comment_class);
    t_text *t = (t_text *)x;
    t->te_type = T_TEXT;
    x->x_glist = canvas_getcurrent();
    sprintf(x->x_tag, "all%lx", (unsigned long)x);
    sprintf(x->x_texttag, "t%lx", (unsigned long)x);
    sprintf(x->x_outlinetag, "h%lx", (unsigned long)x);
    x->x_encoding = x->x_fontfamily = 0;
    x->x_canvas = 0;
    x->x_textbuf = 0;
    x->x_pixwidth = x->x_fontsize = x->x_fontprops = x->x_bbpending = 0;
    x->x_red = x->x_green = x->x_blue = x->x_textbufsize = 0;
    x->x_bbset = x->x_ready = x->x_dragon = 0;
    x->x_selector = s;
    x->x_receive_sym = &s_;
    x->x_transclock = clock_new(x, (t_method)comment_transtick);
    char buf[32];
    sprintf(buf, "comment%lx", (unsigned long)x);
    x->x_bindsym = gensym(buf);
    pd_bind((t_pd *)x, x->x_bindsym);
    if(!commentsink)
        commentsink = pd_new(commentsink_class);
    pd_bind(commentsink, x->x_bindsym);
////////////////////////////////// GET ARGS ///////////////////////////////////////////
    if(ac && av->a_type == A_FLOAT){ // 1ST Width
        x->x_pixwidth = (int)av->a_w.w_float;
        ac--; av++;
        if(ac && av->a_type == A_FLOAT){ // 2ND Size
            x->x_fontsize = (int)av->a_w.w_float;
            ac--; av++;
            if(ac && av->a_type == A_SYMBOL){ // 3RD type
                x->x_fontfamily = av->a_w.w_symbol;
                ac--; av++;
                if(ac && av->a_type == A_SYMBOL){ // 4TH RECEIVE
                    if (av->a_w.w_symbol != gensym("?")){ //  '?' sets empty receive symbol
                        comment_receive(x, av->a_w.w_symbol);
                        ac--; av++;
                    }
                    else
                        ac--; av++;
                    if (ac && av->a_type == A_FLOAT){
                        x->x_fontprops = (int)av->a_w.w_float;
                        ac--; av++;
                        if (ac && av->a_type == A_FLOAT){
                            x->x_red = (unsigned char)av->a_w.w_float;
                            ac--; av++;
                            if(ac && av->a_type == A_FLOAT){
                                x->x_green = (unsigned char)av->a_w.w_float;
                                ac--; av++;
                                if(ac && av->a_type == A_FLOAT){
                                    x->x_blue = (unsigned char)av->a_w.w_float;
                                    ac--; av++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(x->x_fontsize < 1) x->x_fontsize = glist_getfont(x->x_glist);
    if(!x->x_fontfamily) x->x_fontfamily = gensym("helvetica");
    sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
    x->x_binbuf = binbuf_new();
    if(ac) comment_attrparser(x, ac, av);
    else{
        t_atom at;
        SETSYMBOL(&at, gensym("comment"));
        binbuf_restore(x->x_binbuf, 1, &at);
    }
    return(x);
}

/*CYCLONE_OBJ_API*/ void comment_setup(void){
    comment_class = class_new(gensym("comment"), (t_newmethod)comment_new, (t_method)comment_free,
                              sizeof(t_comment), CLASS_DEFAULT, A_GIMME, 0);
    class_addfloat(comment_class, comment_float);
    class_addlist(comment_class, comment_list);
    class_addmethod(comment_class, (t_method)comment_textcolor,
                    gensym("textcolor"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_fontname,
                    gensym("fontname"), A_SYMBOL, 0);
    class_addmethod(comment_class, (t_method)comment_receive,
                    gensym("receive"), A_SYMBOL, 0);
    class_addmethod(comment_class, (t_method)comment_fontsize,
                    gensym("fontsize"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_set,
                    gensym("set"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment_append,
                    gensym("append"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment_prepend,
                    gensym("prepend"), A_GIMME, 0);
    /* new methods 2017: currently do nothing - DK
    class_addmethod(comment_class, (t_method)comment_bgcolor,
                    gensym("bgcolor"), A_FLOAT, A_FLOAT, A_FLOAT,0);
    class_addmethod(comment_class, (t_method)comment_fontface,
                    gensym("fontface"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_textjustification,
                    gensym("textjustification"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_underline,
                    gensym("underline"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_suppressinlet,
                    gensym("suppressinlet"), A_FLOAT, 0);
         // now back to pre-existing methods */
    class_addmethod(comment_class, (t_method)comment__bboxhook,
                    gensym("_bbox"),
                    A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment__clickhook,
                    gensym("_click"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment__releasehook,
                    gensym("_release"), A_SYMBOL, 0);
    class_addmethod(comment_class, (t_method)comment__motionhook,
                    gensym("_motion"), A_SYMBOL, A_FLOAT, A_FLOAT, 0);
    class_setwidget(comment_class, &comment_widgetbehavior);
    
//    class_setpropertiesfn(comment_class, comment_properties);
    class_setsavefn(comment_class, comment_save);
    
    makeshift_class = class_new(gensym("text"), 0, 0,
                                sizeof(t_text), CLASS_NOINLET | CLASS_PATCHABLE, 0);
    
    commentsink_class = class_new(gensym("_commentsink"), 0, 0,
                                  sizeof(t_pd), CLASS_PD, 0);
    class_addanything(commentsink_class, commentsink_anything);
    class_addmethod(commentsink_class, (t_method)commentsink__bboxhook,
                    gensym("_bbox"), A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);

    // if older than 0.43, create an 0.43-style pdsend (not needed anymore, right?)
/*    sys_gui("if {[llength [info procs ::pdsend]] == 0} {");
    sys_gui("proc ::pdsend {args} {::pd \"[join $args { }] ;\"}}\n");*/
    
    sys_gui("proc comment_bbox {target cvname tag} {\n\
            pdsend \"$target _bbox $target [$cvname bbox $tag]\"}\n");
    // LATER think about window vs canvas coords
    sys_gui("proc comment_click {target cvname x y tag} {\n\
            pdsend \"$target _click $target [$cvname canvasx $x] [$cvname canvasy $y]\
            [$cvname index $tag @$x,$y] [$cvname bbox $tag]\"}\n");
    /* LATER think how to conditionally (FORKY_VERSION >= 38)
     replace puts with pdtk_post */
    sys_gui("proc comment_entext {enc tt} {\n\
            if {$enc == \"\"} {concat $tt} else {\n\
            set rr [catch {encoding convertfrom $enc $tt} tt1]\n\
            if {$rr == 0} {concat $tt1} else {\n\
            puts stderr [concat tcl/tk error: $tt1]\n\
            concat $tt}}}\n");
    sys_gui("proc comment_draw {tgt cv tag1 tag2 x y fnm fsz clr enc tt wd} {\n\
            set tt1 [comment_entext $enc $tt]\n\
            if {$wd > 0} {\n\
            $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
            -font [list $fnm $fsz] -fill $clr -width $wd -anchor nw} else {\n\
            $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
            -font [list $fnm $fsz] -fill $clr -anchor nw}\n\
            comment_bbox $tgt $cv $tag1\n\
            $cv bind $tag1 <Button> [list comment_click $tgt %W %x %y $tag1]}\n");
    sys_gui("proc comment_update {cv tag enc tt wd} {\n\
            set tt1 [comment_entext $enc $tt]\n\
            if {$wd > 0} {$cv itemconfig $tag -text $tt1 -width $wd} else {\n\
            $cv itemconfig $tag -text $tt1}}\n");
    
//    #include "comment_dialog.c"
}

