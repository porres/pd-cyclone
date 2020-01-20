/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* FIXME creation lag (X-specific) */
/* LATER think about pushing text to the text editor (ctrl-t)
 -- not easy, because we are not 'textedfor' */
/* LATER think about making the <Button> binding for the entire bbox,
 instead of the text item, to ease the pain of resizing, somewhat. */

// Porres cleaned up and updated comment 2018-2020

#include <common/api.h>
#include "m_pd.h"
#include "g_canvas.h"
#include <string.h>
#include <ctype.h>

// #define COMMENT_DEBUG

#define COMMENT_HANDLEWIDTH 8
#define COMMENT_OUTBUFSIZE  1000

typedef struct _comment{
    t_object        x_obj;
    t_glist        *x_glist;
    t_canvas       *x_cv;
    t_clock        *x_transclock;
    t_binbuf       *x_binbuf;
    char           *x_textbuf;
    int             x_textbufsize;
    int             x_pixwidth;
    int             x_bbset;
    int             x_bbpending;
    int             x_x1;
    int             x_y1;
    int             x_x2;
    int             x_y2;
    int             x_newx2;
    int             x_dragon;
    int             x_fontsize;
    unsigned char   x_red;
    unsigned char   x_green;
    unsigned char   x_blue;
    char            x_color[8];
    char            x_bgcolor[8];
    int             x_selstart;
    int             x_selend;
    int             x_active;
    int             x_ready;
    t_symbol       *x_bindsym;
    t_symbol       *x_fontname;
    t_symbol       *x_receive_sym;
    t_symbol       *x_rcv_unexpanded;
    int             x_rcv_set;
    int             x_flag;
    int             x_zoom;
    int             x_fontface;
    int             x_bold;
    int             x_italic;
    int             x_underline;
    int             x_bg_flag;
    int             x_textjust; // 0: left, 1: center, 2: right
    t_symbol       *x_encoding;   // (unused)
    unsigned int    x_bg[3]; // background color
}t_comment;

static t_class *comment_class, *commentsink_class;

static t_pd *commentsink = 0;

static void comment_receive(t_comment *x, t_symbol *s){
    t_symbol *rcv = canvas_realizedollar(x->x_glist, x->x_rcv_unexpanded = s);
    if(x->x_receive_sym != rcv){
        if(rcv == gensym("empty"))
            rcv = &s_;
        if(rcv != &s_){
            if(x->x_receive_sym != &s_)
                pd_unbind(&x->x_obj.ob_pd, x->x_receive_sym);
            pd_bind(&x->x_obj.ob_pd, x->x_receive_sym = rcv);
        }
        else{
            if(x->x_receive_sym != &s_)
                pd_unbind(&x->x_obj.ob_pd, x->x_receive_sym);
            x->x_receive_sym = rcv;
        }
    }
}

static void comment_set_receive(t_comment *x, t_symbol *s){
    x->x_rcv_set = 1;
    canvas_dirty(x->x_glist, 1);
    comment_receive(x, s);
}

static void comment_draw_bg(t_comment *x){
//    post("comment_draw_bg");
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags bg%lx -outline %s -fill %s\n", (unsigned long)x->x_cv,
        x->x_x1, x->x_y1, x->x_x2 + x->x_zoom, x->x_y2 + x->x_zoom, (unsigned long)x, x->x_bgcolor, x->x_bgcolor);
}

static void comment_draw(t_comment *x){
//    post("comment_draw");
//  comment_draw_bg(x);
    char buf[COMMENT_OUTBUFSIZE], *outbuf, *outp;
    int reqsize = x->x_textbufsize + 250;  // FIXME estimation
    if(reqsize > COMMENT_OUTBUFSIZE){ // <= seems unnecessary (porres)
        post("bug? allocating %d outbuf bytes", reqsize);
        if(!(outbuf = getbytes(reqsize))){
            post("bug? return");
            return;
        }
    }
    outp = outbuf = buf;
    sprintf(outp, "%s %s .x%lx.c txt%lx all%lx %d %d %s -%d %s %s {%.*s} %d %s %s %s\n",
        x->x_underline ? "comment_draw_ul" : "comment_draw",
        x->x_bindsym->s_name, // %s
        (unsigned long)x->x_cv, // .x%lx.c
        (unsigned long)x, // txt%lx
        (unsigned long)x, // all%lx
        text_xpix((t_text *)x, x->x_glist) + x->x_zoom, // %d
        text_ypix((t_text *)x, x->x_glist) + x->x_zoom, // %d
        x->x_fontname->s_name, // %s
        x->x_fontsize, // -%d
        glist_isselected(x->x_glist, &x->x_glist->gl_gobj) ? "blue" : x->x_color, // %s
        "\"\"", // %s (encoding)
        x->x_textbufsize, // %.
        x->x_textbuf, // *s
        x->x_pixwidth, // %d
        x->x_bold ? "bold" : "normal",
        x->x_italic ? "italic" : "roman", //
        x->x_textjust == 0 ? "left" : x->x_textjust == 1 ? "center" : "right");
    x->x_bbpending = 1;
    sys_gui(outbuf);
    if(outbuf != buf)
        freebytes(outbuf, reqsize);
}

static void comment_redraw(t_comment *x, int bg){
    sys_vgui(".x%lx.c delete all%lx\n", x->x_cv, (unsigned long)x);
    sys_vgui(".x%lx.c delete bg%lx\n", x->x_cv, (unsigned long)x);
    if(bg)
        comment_draw_bg(x);
    comment_draw(x);
}

static void comment_update(t_comment *x){
    #ifdef COMMENT_DEBUG
    post("update");
    #endif
    char buf[COMMENT_OUTBUFSIZE], *outbuf, *outp;
    unsigned long cv = (unsigned long)x->x_cv;
    int reqsize = x->x_textbufsize + 250;  // FIXME estimation
    if(reqsize > COMMENT_OUTBUFSIZE){ // <= seems unnecessary (porres)
        #ifdef COMMENT_DEBUG
         post("allocating %d outbuf bytes", reqsize);
         #endif
        if(!(outbuf = getbytes(reqsize)))
            return;
    }
    outp = outbuf = buf;
    sprintf(outp, "comment_update .x%lx.c txt%lx %s {%.*s} %d\n",
            cv, (unsigned long)x, (x->x_encoding ? x->x_encoding->s_name : "\"\""),
            x->x_textbufsize, x->x_textbuf, x->x_pixwidth);
    outp += strlen(outp);
    if(x->x_active){
        if(x->x_selend > x->x_selstart){
            sprintf(outp, ".x%lx.c select from txt%lx %d\n", cv, (unsigned long)x, x->x_selstart);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c select to txt%lx %d\n", cv, (unsigned long)x, x->x_selend);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c focus {}\n", cv);
        }
        else{
            sprintf(outp, ".x%lx.c select clear\n", cv);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c icursor txt%lx %d\n", cv, (unsigned long)x, x->x_selstart);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c focus txt%lx\n", cv, (unsigned long)x);
        }
        outp += strlen(outp);
    }
    sprintf(outp, "comment_bbox %s .x%lx.c txt%lx\n",
        x->x_bindsym->s_name, cv, (unsigned long)x);
    x->x_bbpending = 1;
    #ifdef COMMENT_DEBUG
    post("send comment_bbox");
    #endif
    sys_gui(outbuf);
    if(outbuf != buf)
        freebytes(outbuf, reqsize);
}

static void comment_validate(t_comment *x, t_glist *glist){
    if(!x->x_ready){
        if(x->x_textbuf)
            freebytes(x->x_textbuf, x->x_textbufsize);
        binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
        x->x_ready = 1;
        #ifdef COMMENT_DEBUG
         post("validation done");
         #endif
    }
    if(glist){
        if(glist != x->x_glist){
            post("bug [comment]: comment_getcanvas");
            x->x_glist = glist;
        }
        x->x_cv = glist_getcanvas(glist);
    }
}

static void comment_grabbedkey(void *z, t_floatarg f){
    z = NULL;
    f = 0;  /* LATER think about replacing #key binding/comment_float() with grabbing */
    #ifdef COMMENT_DEBUG
     post("comment_grabbedkey %g", f);
     #endif
}

static void comment_dograb(t_comment *x){
// LATER investigate grab feature. Used here to prevent backspace erasing text.
// Done also when already active, because after clicked we lost our previous grab.
    glist_grab(x->x_glist, (t_gobj *)x, 0, comment_grabbedkey, 0, 0);
}

static void comment__bboxhook(t_comment *x, t_symbol *bindsym, t_floatarg x1, t_floatarg y1, t_floatarg x2, t_floatarg y2){
    #ifdef COMMENT_DEBUG
     post("bbox %g %g %g %g", x1, y1, x2, y2);
     #endif
    bindsym = NULL;
    if(x->x_x1 != x1 || x->x_y1 != y1 || x->x_x2 != x2 || x->x_y2 != y2){
        x->x_x1 = x1;
        x->x_y1 = y1;
        x->x_x2 = x2;
        x->x_y2 = y2;
        if(x->x_bg_flag)
            comment_redraw(x, x->x_bg_flag);
    }
    x->x_bbset = 1;
    x->x_bbpending = 0;
}

static void comment__clickhook(t_comment *x, t_symbol *s, int ac, t_atom *av){
    #ifdef COMMENT_DEBUG
    post("click hook");
    #endif
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
            unsigned long cv = (unsigned long)x->x_cv;
            sprintf(outp, ".x%lx.c bind txt%lx <ButtonRelease> {pdsend {%s _release %s}}\n",
                cv, (unsigned long)x, x->x_bindsym->s_name, x->x_bindsym->s_name);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c bind txt%lx <Motion> {pdsend {%s _motion %s %%x %%y}}\n",
                cv, (unsigned long)x, x->x_bindsym->s_name, x->x_bindsym->s_name);
            outp += strlen(outp);
            sprintf(outp, ".x%lx.c create rectangle %d %d %d %d -outline blue -tags {outline%lx all%lx}\n",
                cv, x->x_x1, x->x_y1, x->x_x2, x->x_y2, (unsigned long)x, (unsigned long)x);
            sys_gui(buf);
            x->x_newx2 = x->x_x2;
            x->x_dragon = 1;
        }
    }
}

static void comment__releasehook(t_comment *x, t_symbol *bindsym){
    #ifdef COMMENT_DEBUG
    post("release hook");
    #endif
    bindsym = NULL;
    unsigned long cv = (unsigned long)x->x_cv;
    sys_vgui(".x%lx.c bind txt%lx <ButtonRelease> {}\n", cv, (unsigned long)x);
    sys_vgui(".x%lx.c bind txt%lx <Motion> {}\n", cv, (unsigned long)x);
    sys_vgui(".x%lx.c delete outline%lx\n", cv, (unsigned long)x);
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
    int min_width = 8;
    if(xx > x->x_x1 + min_width)
        sys_vgui(".x%lx.c coords outline%lx %d %d %d %d\n", (unsigned long)x->x_cv,
            (unsigned long)x, x->x_x1, x->x_y1, x->x_newx2 = xx, x->x_y2);
}

static void commentsink__bboxhook(t_pd *x, t_symbol *bindsym){
    if(bindsym->s_thing == x){
        pd_unbind(x, bindsym);  // if comment gone, unbind
        #ifdef COMMENT_DEBUG
            post("sink: %s unbound", bindsym->s_name);
        #endif
    }
}

static void commentsink_anything(t_pd *x, t_symbol *s, int ac, t_atom *av){ // nop: avoid warnings
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
    width += (x->x_zoom * 2);
// FIXME estimation
    height = x->x_fontsize + (x->x_zoom * 2);
    x1 = text_xpix((t_text *)x, glist);
    y1 = text_ypix((t_text *)x, glist) + 1;  // LATER revisit
    x2 = x1 + width;
    y2 = y1 + height - 2;  // LATER revisit
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
        sys_vgui(".x%lx.c move all%lx %d %d\n", x->x_cv, (unsigned long)x,
            dx*x->x_zoom, dy*x->x_zoom);
        sys_vgui(".x%lx.c move bg%lx %d %d\n", x->x_cv, (unsigned long)x,
            dx*x->x_zoom, dy*x->x_zoom);
        canvas_fixlinesfor(x->x_cv, t);
    }
}

static void comment_activate(t_gobj *z, t_glist *glist, int state){
    #ifdef COMMENT_DEBUG
    post("activate");
    #endif
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if(state){
        comment_dograb(x);
        if(x->x_active)
            return;
        sys_vgui(".x%lx.c focus txt%lx\n", x->x_cv, (unsigned long)x);
        x->x_selstart = 0;
        x->x_selend = x->x_textbufsize;
        x->x_active = 1;
        pd_bind((t_pd *)x, gensym("#key"));
        pd_bind((t_pd *)x, gensym("#keyname"));
    }
    else{
        if(!x->x_active)
            return;
        pd_unbind((t_pd *)x, gensym("#key"));
        pd_unbind((t_pd *)x, gensym("#keyname"));
        sys_vgui("selection clear .x%lx.c\n", x->x_cv);
        sys_vgui(".x%lx.c focus {}\n", x->x_cv);
        x->x_active = 0;
    }
    comment_update(x);
}

static void comment_select(t_gobj *z, t_glist *glist, int state){
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if(!state && x->x_active) comment_activate(z, glist, 0);
    sys_vgui(".x%lx.c itemconfigure txt%lx -fill %s\n", x->x_cv,
       (unsigned long)x, (state ? "blue" : x->x_color));
/* A regular rtext should set 'canvas_editing' variable to its canvas, we don't do it coz
 we get keys via global binding to "#key" (and coz 'canvas_editing' isn't exported). */
}

static void comment_delete(t_gobj *z, t_glist *glist){
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void comment_vis(t_gobj *z, t_glist *glist, int vis){
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if(vis)
        comment_draw(x);
    else{
        sys_vgui(".x%lx.c delete all%lx\n", x->x_cv, (unsigned long)x);
        sys_vgui(".x%lx.c delete bg%lx\n", x->x_cv, (unsigned long)x);
    }
}

static void comment_save(t_gobj *z, t_binbuf *b){
    t_comment *x = (t_comment *)z;
    comment_validate(x, 0); // this is not needed, must be removed!!!
    t_binbuf *bb = x->x_obj.te_binbuf;
    if(!x->x_rcv_set){ // no receive set, search arguments
        int n_args = binbuf_getnatom(bb); // number of arguments
        if(n_args > 0){ // we have arguments, let's search them
            char buf[80];
            if(x->x_flag){ // search for receive name in attributes
                for(int i = 0;  i < n_args; i++){
                    atom_string(binbuf_getvec(bb) + i, buf, 80);
                    if(gensym(buf) == gensym("@receive")){
                        atom_string(binbuf_getvec(bb) + i + 1, buf, 80);
                        x->x_rcv_unexpanded = gensym(buf);
                        break;
                    }
                }
            }
            else{ // search receive argument
                int arg_n = 4; // receive argument number
                if(n_args >= arg_n){ // we have it, get it
                    atom_string(binbuf_getvec(bb) + arg_n, buf, 80);
                    x->x_rcv_unexpanded = gensym(buf);
                }
            }
        }
        if(x->x_rcv_unexpanded == &s_) // if nothing found, set to "empty"
            x->x_rcv_unexpanded = gensym("?");
    }
    binbuf_addv(b, "ssiisiissiiiiiiiiii",
        gensym("#X"),
        gensym("obj"),
        (int)x->x_obj.te_xpix,
        (int)x->x_obj.te_ypix,
        atom_getsymbol(binbuf_getvec(bb)),
        x->x_pixwidth, // zoom???
        x->x_fontsize / x->x_zoom,
        x->x_fontname,
        x->x_rcv_unexpanded,
        x->x_fontface,
        (int)x->x_red,
        (int)x->x_green,
        (int)x->x_blue,
        x->x_underline,
        (int)x->x_bg[0],
        (int)x->x_bg[1],
        (int)x->x_bg[2],
        x->x_bg_flag,
        x->x_textjust);
    binbuf_addbinbuf(b, x->x_binbuf); // the actual comment
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

// this is basically the code of rtext_key()
static void comment_float(t_comment *x, t_float f){
    #ifdef COMMENT_DEBUG
    post("float = %d", (int)f);
    #endif
    if(x->x_active){
        int keynum = (int)f;
        if(keynum){
            int i, newsize, ndel;
            int n = keynum;
            if(n == '\r') n = '\n';
            if(n == '\b'){
                if((!x->x_selstart) && (x->x_selend == x->x_textbufsize)){
                    // LATER delete box (causes reentrancy problems
                    /* glist_delete(x->x_glist, &x->x_text->te_g); */
                    goto donefloat;
                }
                else if(x->x_selstart && (x->x_selstart == x->x_selend))
                    x->x_selstart--;
            }
            ndel = x->x_selend - x->x_selstart;
            for(i = x->x_selend; i < x->x_textbufsize; i++)
                x->x_textbuf[i- ndel] = x->x_textbuf[i];
            newsize = x->x_textbufsize - ndel;
            x->x_textbuf = resizebytes(x->x_textbuf, x->x_textbufsize, newsize);
            x->x_textbufsize = newsize;
            if(n == '\n' || !iscntrl(n)){
                #ifdef COMMENT_DEBUG
                 post("%d accepted", n);
                 #endif
                newsize = x->x_textbufsize+1;
                x->x_textbuf = resizebytes(x->x_textbuf, x->x_textbufsize, newsize);
                for(i = x->x_textbufsize; i > x->x_selstart; i--)
                    x->x_textbuf[i] = x->x_textbuf[i-1];
                x->x_textbuf[x->x_selstart] = n;
                x->x_textbufsize = newsize;
                x->x_selstart = x->x_selstart + 1;
            }
            #ifdef COMMENT_DEBUG
             else post("%d rejected", n);
             #endif
            x->x_selend = x->x_selstart;
            x->x_glist->gl_editor->e_textdirty = 1;
            binbuf_text(x->x_binbuf, x->x_textbuf, x->x_textbufsize);
            comment_update(x);
        }
    }
    else post("bug [comment]: comment_float");
donefloat:;
     #ifdef COMMENT_DEBUG
     post("donefloat");
     #endif
}

static void comment_list(t_comment *x, t_symbol *s, int ac, t_atom *av){
    #ifdef COMMENT_DEBUG
    post("list");
    #endif
    t_symbol *dummy = s;
    dummy = NULL;
    if(!x->x_active)
        post("bug [comment]: comment_list");
    else if(ac > 1 && av->a_type == A_FLOAT && (int)av->a_w.w_float && av[1].a_type == A_SYMBOL){
        t_symbol *keysym = av[1].a_w.w_symbol;
        if(keysym == gensym("Right")){
            if(x->x_selend == x->x_selstart &&
                x->x_selstart < x->x_textbufsize)
                x->x_selend = x->x_selstart = x->x_selstart + 1;
            else
                x->x_selstart = x->x_selend;
        }
        else if(keysym == gensym("Left")){
            if(x->x_selend == x->x_selstart && x->x_selstart > 0)
                x->x_selend = x->x_selstart = x->x_selstart - 1;
            else
                x->x_selend = x->x_selstart;
        }
        // this should be improved...  life's too short
        else if(keysym == gensym("Up")){
            if(x->x_selstart)
                x->x_selstart--;
            while(x->x_selstart > 0 && x->x_textbuf[x->x_selstart] != '\n')
                x->x_selstart--;
            x->x_selend = x->x_selstart;
        }
        else if(keysym == gensym("Down")){
            while(x->x_selend < x->x_textbufsize &&
                   x->x_textbuf[x->x_selend] != '\n')
                x->x_selend++;
            if(x->x_selend < x->x_textbufsize)
                x->x_selend++;
            x->x_selstart = x->x_selend;
        }
        else if(keysym == gensym("F5")){
            t_text *t = (t_text *)x;
            t_binbuf *bb = binbuf_new();
            int argc = binbuf_getnatom(x->x_binbuf);
            binbuf_addv(bb, "ii", (int)t->te_xpix + 5, (int)t->te_ypix + 5);
            binbuf_add(bb, argc, binbuf_getvec(x->x_binbuf));
            canvas_setcurrent(x->x_glist);
            typedmess((t_pd *)x->x_glist, gensym("text"),
                      argc + 2, binbuf_getvec(bb));
            canvas_unsetcurrent(x->x_glist);
            binbuf_free(bb);
            goto donelist;
        }
        else goto donelist;
        comment_update(x);
    }
donelist:;
    #ifdef COMMENT_DEBUG
     post("donelist");
     #endif
}

static void comment_free(t_comment *x){
    if(x->x_active){
        pd_unbind((t_pd *)x, gensym("#key"));
        pd_unbind((t_pd *)x, gensym("#keyname"));
    }
    if(x->x_receive_sym != &s_)
        pd_unbind(&x->x_obj.ob_pd, x->x_receive_sym);
    if(x->x_transclock)
        clock_free(x->x_transclock);
    if(x->x_bindsym){
        pd_unbind((t_pd *)x, x->x_bindsym);
        if(!x->x_bbpending)
            pd_unbind(commentsink, x->x_bindsym);
    }
    if(x->x_binbuf && !x->x_ready)
        binbuf_free(x->x_binbuf);
    if(x->x_textbuf)
        freebytes(x->x_textbuf, x->x_textbufsize);
}

static void comment_append(t_comment *x, t_symbol *s, int ac, t_atom * av){
    s = NULL;
    canvas_dirty(x->x_glist, 1);
    t_binbuf *bb = binbuf_new();
    binbuf_restore(bb, ac, av);
    binbuf_addbinbuf(x->x_binbuf, bb);
    binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
    comment_redraw(x, x->x_bg_flag);
}

static void comment_prepend(t_comment *x, t_symbol *s, int ac, t_atom * av){
    s = NULL;
    canvas_dirty(x->x_glist, 1);
    t_binbuf *bb = binbuf_new();
    binbuf_restore(bb, ac, av);
    binbuf_addbinbuf(bb, x->x_binbuf);
    binbuf_clear(x->x_binbuf);
    binbuf_addbinbuf(x->x_binbuf, bb);
    binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
    comment_redraw(x, x->x_bg_flag);
}

static void comment_set(t_comment *x, t_symbol *s, int ac, t_atom * av){
    s = NULL;
    canvas_dirty(x->x_glist, 1);
    binbuf_clear(x->x_binbuf);
    binbuf_restore(x->x_binbuf, ac, av);
    binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
    comment_redraw(x, x->x_bg_flag);
}

static void comment_textcolor(t_comment *x, t_floatarg r, t_floatarg g, t_floatarg b){
    unsigned char red = r < 0 ? 0 : r > 255 ? 255 : (unsigned char)r;
    unsigned char green = g < 0 ? 0 : g > 255 ? 255 : (unsigned char)g;
    unsigned char blue = b < 0 ? 0 : b > 255 ? 255 : (unsigned char)b;
    if(x->x_red != red || x->x_green != green || x->x_blue != blue){
        canvas_dirty(x->x_glist, 1);
        x->x_red = red, x->x_green = green, x->x_blue = blue;
        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
        sys_vgui(".x%lx.c itemconfigure txt%lx -fill %s\n", x->x_cv, (unsigned long)x, x->x_color);
    }
}

static void comment_bgcolor(t_comment *x, t_float r, t_float g, t_float b){
    unsigned int red = r < 0 ? 0 : r > 255 ? 255 : (unsigned int)r;
    unsigned int green = g < 0 ? 0 : g > 255 ? 255 : (unsigned int)g;
    unsigned int blue = b < 0 ? 0 : b > 255 ? 255 : (unsigned int)b;
    if(x->x_bg[0] != red || x->x_bg[1] != green || x->x_bg[2] != blue || !x->x_bg_flag){
        canvas_dirty(x->x_glist, 1);
        x->x_bg_flag = 1;
        x->x_bg[0] = red, x->x_bg[1] = green, x->x_bg[2] = blue;
        sprintf(x->x_bgcolor, "#%2.2x%2.2x%2.2x", x->x_bg[0], x->x_bg[1], x->x_bg[2]);
        comment_redraw(x, x->x_bg_flag);
    }
}

static void comment_fontname(t_comment *x, t_symbol *name){
    if(name != x->x_fontname){
        canvas_dirty(x->x_glist, 1);
        x->x_fontname = name;
        comment_redraw(x, x->x_bg_flag);
    }
}

static void comment_fontsize(t_comment *x, t_floatarg f){
    int size = (int)f < 5 ? 5 : (int)f;
    if(x->x_fontsize != size){
        canvas_dirty(x->x_glist, 1);
        x->x_fontsize = size;
        comment_redraw(x, x->x_bg_flag);
    }
}

static void comment_fontface(t_comment *x, t_float f){
    int face = f < 0 ? 0 : f > 3 ? 3  : (int)f;
    if(face != x->x_fontface){
        canvas_dirty(x->x_glist, 1);
        x->x_fontface = face;
        x->x_bold = x->x_fontface == 1 || x->x_fontface == 3;
        x->x_italic = x->x_fontface > 1;
        comment_redraw(x, x->x_bg_flag);
    }
}

static void comment_bg_flag(t_comment *x, t_float f){
    int bgflag = (int)(f != 0);
    if(x->x_bg_flag != bgflag){
        canvas_dirty(x->x_glist, 1);
        comment_redraw(x, x->x_bg_flag = bgflag);
    }
}

static void comment_underline(t_comment *x, t_float f){
    int underline = (int)(f != 0);
    if(x->x_underline != underline){
        canvas_dirty(x->x_glist, 1);
        x->x_underline = underline;
        comment_redraw(x, x->x_bg_flag);
    }
}

static void comment_textjustification(t_comment *x, t_float f){
    int just = f < 0 ? 0 : (f > 2 ? 2 : (int)f);
    if(just != x->x_textjust){
        canvas_dirty(x->x_glist, 1);
        x->x_textjust = just;
        comment_redraw(x, x->x_bg_flag);
    }
}

static void comment_zoom(t_comment *x, t_floatarg zoom){
    comment_fontsize(x, (float)x->x_fontsize * ((x->x_zoom = (int)zoom) == 1. ? 0.5 : 2.));
}

///////////// ==========------------------------------------===>  later rethink/REWRITE
static void comment_attrparser(t_comment *x, int ac, t_atom * av){
    t_atom* comlist = t_getbytes(ac * sizeof(*comlist));
    int i, comlen = 0; // eventual length of comment list comlist
    for(i = 0; i < ac; i++){
        if(av[i].a_type == A_FLOAT){
            SETFLOAT(&comlist[comlen], av[i].a_w.w_float);
            comlen++;
        }
        else if(av[i].a_type == A_SYMBOL){
            t_symbol * sym = av[i].a_w.w_symbol;
            if(sym == gensym("@fontsize")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        int fontsize = (int)av[i].a_w.w_float;
                        x->x_fontsize = fontsize;
                    }
                    else i--;
                };
            }
            else if(sym == gensym("@fontname")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_SYMBOL)
                        x->x_fontname = av[i].a_w.w_symbol;
                    else i--;
                };
            }
            else if(sym == gensym("@receive")){
                x->x_flag = 1;
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_SYMBOL)
                        comment_receive(x, atom_getsymbolarg(i, ac, av));
                    else i--;
                };
            }
            else if(sym == gensym("@textcolor")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        int rgb = (unsigned char)av[i].a_w.w_float;
                        x->x_red = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
                    }
                    else i--;
                };
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        i++;
                        int rgb = (unsigned char)av[i].a_w.w_float;
                        x->x_green = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
                    }
                    else i--;
                };
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        i++;
                        int rgb = (unsigned char)av[i].a_w.w_float;
                        x->x_blue = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
                    }
                    else i--;
                };
            }
            else if(sym == gensym("@bgcolor")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        int rgb = (unsigned int)av[i].a_w.w_float;
                        x->x_bg[0] = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_bg[0], x->x_bg[1], x->x_bg[2]);
                    }
                    else i--;
                };
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        i++;
                        int rgb = (unsigned int)av[i].a_w.w_float;
                        x->x_bg[1] = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_bg[0], x->x_bg[1], x->x_bg[2]);
                    }
                    else i--;
                };
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        i++;
                        int rgb = (unsigned char)av[i].a_w.w_float;
                        x->x_bg[2] = rgb;
                        sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_bg[0], x->x_bg[1], x->x_bg[2]);
                    }
                    else i--;
                };
            }
            else if(sym == gensym("@text")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_SYMBOL){
                        SETSYMBOL(&comlist[comlen], av[i].a_w.w_symbol);
                        comlen++;
                    }
                    else i--;
                };
            }
            else if(sym == gensym("@fontface")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT)
                        x->x_fontface  = (int)av[i].a_w.w_float;
                    else i--;
                };
            }
            else if(sym == gensym("@underline")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT)
                        x->x_underline = (int)(av[i].a_w.w_float != 0);
                    else
                        i--;
                };
            }
            else if(sym == gensym("@bg")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT)
                        x->x_bg_flag = (int)(av[i].a_w.w_float != 0);
                    else
                        i--;
                };
            }
            else if(sym == gensym("@textjustification")){
                i++;
                if((ac-i) > 0){
                    if(av[i].a_type == A_FLOAT){
                        int textjust = (int)av[i].a_w.w_float;
                        x->x_textjust = textjust < 0 ? 0 : textjust > 2 ? 2 : textjust;
                    }
                    else i--;
                };
            }
            else{ // treat it as a part of comlist
                SETSYMBOL(&comlist[comlen], sym);
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
    t_freebytes(comlist, ac * sizeof(*comlist));
}

static void *comment_new(t_symbol *s, int ac, t_atom *av){
    s = NULL;
    t_comment *x = (t_comment *)pd_new(comment_class);
    t_text *t = (t_text *)x;
    t->te_type = T_TEXT;
    x->x_glist = canvas_getcurrent();
    x->x_zoom = x->x_glist->gl_zoom;
    x->x_encoding = x->x_fontname = 0;
    x->x_cv = 0;
    x->x_textbuf = 0;
    x->x_rcv_set = x->x_flag = 0;
    x->x_pixwidth = x->x_fontsize = x->x_bbpending = x->x_fontface = x->x_bold = x->x_italic = 0;
    x->x_textjust = 0;
    x->x_red = x->x_green = x->x_blue = x->x_textbufsize = 0;
    x->x_bg_flag = 0;
    x->x_bg[0] = x->x_bg[1] = x->x_bg[2] = 255;
    sprintf(x->x_bgcolor, "#%2.2x%2.2x%2.2x", x->x_bg[0], x->x_bg[1], x->x_bg[2]);
    x->x_bbset = x->x_ready = x->x_dragon = 0;
    x->x_receive_sym = x->x_rcv_unexpanded = &s_;
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
        ac--, av++;
        if(ac && av->a_type == A_FLOAT){ // 2ND Size
            x->x_fontsize = (int)av->a_w.w_float * x->x_zoom;
            ac--, av++;
            if(ac && av->a_type == A_SYMBOL){ // 3RD type
                x->x_fontname = av->a_w.w_symbol;
                ac--, av++;
                if(ac && av->a_type == A_SYMBOL){ // 4TH RECEIVE
                    if(av->a_w.w_symbol != gensym("?")){ //  '?' sets empty receive symbol
                        comment_receive(x, av->a_w.w_symbol);
                        ac--, av++;
                    }
                    else
                        ac--, av++;
                    if(ac && av->a_type == A_FLOAT){ // face
                        x->x_fontface = (int)av->a_w.w_float;
                        ac--, av++;
                        if(ac && av->a_type == A_FLOAT){ // R
                            x->x_red = (unsigned char)av->a_w.w_float;
                            ac--, av++;
                            if(ac && av->a_type == A_FLOAT){ // G
                                x->x_green = (unsigned char)av->a_w.w_float;
                                ac--, av++;
                                if(ac && av->a_type == A_FLOAT){ // B
                                    x->x_blue = (unsigned char)av->a_w.w_float;
                                    ac--, av++;
                                    if(ac && av->a_type == A_FLOAT){ // Underline
                                        x->x_underline = (int)(av->a_w.w_float != 0);
                                        ac--, av++;
                                        if(ac && av->a_type == A_FLOAT){ // R
                                            x->x_bg[0] = (unsigned int)av->a_w.w_float;
                                            ac--, av++;
                                            if(ac && av->a_type == A_FLOAT){ // G
                                                x->x_bg[1] = (unsigned int)av->a_w.w_float;
                                                ac--, av++;
                                                if(ac && av->a_type == A_FLOAT){ // B
                                                    x->x_bg[2] = (unsigned int)av->a_w.w_float;
                                                    ac--, av++;
                                                    if(ac && av->a_type == A_FLOAT){ // bg flag
                                                        x->x_bg_flag = (int)(av->a_w.w_float != 0);
                                                        ac--, av++;
                                                        if(ac && av->a_type == A_FLOAT){ // Justification
                                                            int textjust = (int)(av->a_w.w_float);
                                                            x->x_textjust = textjust < 0 ? 0 : textjust > 2 ? 2 : textjust;
                                                            ac--, av++;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    x->x_binbuf = binbuf_new();
    if(ac)
        comment_attrparser(x, ac, av);
    else{
        t_atom at;
        SETSYMBOL(&at, gensym("comment"));
        binbuf_restore(x->x_binbuf, 1, &at);
    }
    if(x->x_fontsize < 1)
        x->x_fontsize = glist_getfont(x->x_glist) * x->x_zoom;
    if(!x->x_fontname)
        x->x_fontname = gensym("helvetica");
    x->x_fontface = x->x_fontface < 0 ? 0 : (x->x_fontface > 3 ? 3 : x->x_fontface);
    x->x_bold = x->x_fontface == 1 || x->x_fontface == 3;
    x->x_italic = x->x_fontface > 1;
    x->x_red = x->x_red > 255 ? 255 : x->x_red < 0 ? 0 : x->x_red;
    x->x_green = x->x_green > 255 ? 255 : x->x_green < 0 ? 0 : x->x_green;
    x->x_blue = x->x_blue > 255 ? 255 : x->x_blue < 0 ? 0 : x->x_blue;
    sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);
    x->x_bg[0] = x->x_bg[0] > 255 ? 255 : x->x_bg[0] < 0 ? 0 : x->x_bg[0];
    x->x_bg[1] = x->x_bg[1] > 255 ? 255 : x->x_bg[1] < 0 ? 0 : x->x_bg[1];
    x->x_bg[2] = x->x_bg[2] > 255 ? 255 : x->x_bg[2] < 0 ? 0 : x->x_bg[2];
    sprintf(x->x_bgcolor, "#%2.2x%2.2x%2.2x", x->x_bg[0], x->x_bg[1], x->x_bg[2]);
    return(x);
}

CYCLONE_OBJ_API void comment_setup(void){
    comment_class = class_new(gensym("comment"), (t_newmethod)comment_new, (t_method)comment_free,
        sizeof(t_comment), CLASS_DEFAULT, A_GIMME, 0);
    class_addfloat(comment_class, comment_float);
    class_addlist(comment_class, comment_list);
    class_addmethod(comment_class, (t_method)comment_fontname, gensym("fontname"), A_SYMBOL, 0);
    class_addmethod(comment_class, (t_method)comment_set_receive, gensym("receive"), A_SYMBOL, 0);
    class_addmethod(comment_class, (t_method)comment_fontsize, gensym("fontsize"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_set, gensym("set"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment_append, gensym("append"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment_prepend, gensym("prepend"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment_fontface, gensym("fontface"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_underline, gensym("underline"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_textjustification, gensym("textjustification"),
        A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_textcolor, gensym("textcolor"), A_FLOAT,
        A_FLOAT, A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_bg_flag, gensym("bg"), A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_bgcolor, gensym("bgcolor"), A_FLOAT, A_FLOAT,
        A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment_zoom, gensym("zoom"), A_CANT, 0);
    class_addmethod(comment_class, (t_method)comment__bboxhook, gensym("_bbox"),
        A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment__clickhook, gensym("_click"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment__releasehook, gensym("_release"), A_SYMBOL, 0);
    class_addmethod(comment_class, (t_method)comment__motionhook, gensym("_motion"), A_SYMBOL,
        A_FLOAT, A_FLOAT, 0);
    class_setwidget(comment_class, &comment_widgetbehavior);
    class_setsavefn(comment_class, comment_save);
//  class_setpropertiesfn(comment_class, comment_properties);
    commentsink_class = class_new(gensym("_commentsink"), 0, 0, sizeof(t_pd), CLASS_PD, 0);
    class_addanything(commentsink_class, commentsink_anything);
    class_addmethod(commentsink_class, (t_method)commentsink__bboxhook, gensym("_bbox"), A_SYMBOL, 0);
    sys_gui("proc comment_bbox {target cvname tag} {\n\
            pdsend \"$target _bbox $target [$cvname bbox $tag]\"}\n");
// LATER think about window vs canvas coords
    sys_gui("proc comment_click {target cvname x y tag} {\n\
            pdsend \"$target _click $target [$cvname canvasx $x] [$cvname canvasy $y]\
            [$cvname index $tag @$x,$y] [$cvname bbox $tag]\"}\n");
    sys_gui("proc comment_entext {enc tt} {\n\
            if {$enc == \"\"} {concat $tt} else {\n\
            set rr [catch {encoding convertfrom $enc $tt} tt1]\n\
            if {$rr == 0} {concat $tt1} else {\n\
            puts stderr [concat tcl/tk error: $tt1]\n\
            concat $tt}}}\n");
    sys_gui("proc comment_update {cv tag enc tt wd} {\n\
            set tt1 [comment_entext $enc $tt]\n\
            if {$wd > 0} {$cv itemconfig $tag -text $tt1 -width $wd} else {\n\
            $cv itemconfig $tag -text $tt1}}\n");
    sys_gui("proc comment_draw {tgt cv tag1 tag2 x y fnm fsz clr enc tt wd wt sl just} {\n\
            set tt1 [comment_entext $enc $tt]\n\
            if {$wd > 0} {\n\
            $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
            -font [list $fnm $fsz $wt $sl] -justify $just -fill $clr -width $wd -anchor nw} else {\n\
            $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
            -font [list $fnm $fsz $wt $sl] -justify $just -fill $clr -anchor nw}\n\
            comment_bbox $tgt $cv $tag1\n\
            $cv bind $tag1 <Button> [list comment_click $tgt %W %x %y $tag1]}\n");
// later rethink:
    sys_gui("proc comment_draw_ul {tgt cv tag1 tag2 x y fnm fsz clr enc tt wd wt sl just} {\n\
            set tt1 [comment_entext $enc $tt]\n\
            if {$wd > 0} {\n\
            $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
            -font [list $fnm $fsz $wt $sl underline] -justify $just -fill $clr -width $wd -anchor nw} else {\n\
            $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
            -font [list $fnm $fsz $wt $sl underline] -justify $just -fill $clr -anchor nw}\n\
            comment_bbox $tgt $cv $tag1\n\
            $cv bind $tag1 <Button> [list comment_click $tgt %W %x %y $tag1]}\n");
//    #include "comment_dialog.c"
}
