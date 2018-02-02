/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* adding zldata_realloc, making zl_zlmaxsize actually do something, editing zldata_init
    also adding arguments/attributes, getting rid of grow and loud - Derek Kwan 2016-2018 */

#include <string.h>
#include "m_pd.h"

// LATER test reentrancy, tune speedwise

#define ZL_DEF_SIZE    256     // default size
#define ZL_MINSIZE     1       // min size
#define ZL_MAXSIZE     32767   // max size
#define ZL_N_MODES     16      // number of modes

struct _zl;

typedef int (*t_zlintargfn)(struct _zl *, int);
typedef void (*t_zlanyargfn)(struct _zl *, t_symbol *, int, t_atom *);
typedef int (*t_zlnatomsfn)(struct _zl *);
typedef void (*t_zldoitfn)(struct _zl *, int, t_atom *, int);

static int            zl_nmodes = 0;
static t_symbol      *zl_modesym[ZL_N_MODES];
static int            zl_modeflags[ZL_N_MODES];
static t_zlintargfn   zl_intargfn[ZL_N_MODES];
static t_zlanyargfn   zl_anyargfn[ZL_N_MODES];
static t_zlnatomsfn   zl_natomsfn[ZL_N_MODES];
static t_zldoitfn     zl_doitfn[ZL_N_MODES];

typedef struct _zldata{
    int      d_size;    /* as allocated */
    int      d_max;     // max size allowed, must be <= d_size
    int      d_natoms;  /* as used */
    t_atom  *d_buf;
    t_atom   d_bufini[ZL_DEF_SIZE];
} t_zldata;

typedef struct _zl{
    t_object          x_ob;
    struct _zlproxy  *x_proxy;
    int               x_entered;
    int               x_locked;  /* locking inbuf1 in modes: iter, reg, slice */
    t_zldata          x_inbuf1;
    t_zldata          x_inbuf2;
    t_zldata          x_outbuf;
    int               x_mode;
    int               x_modearg;
    t_outlet         *x_out2;
} t_zl;

typedef struct _zlproxy{
    t_object  p_ob;
    t_zl     *p_master;
} t_zlproxy;

static t_class *zl_class;
static t_class *zlproxy_class;

// ********************************************************************
// ******************** ZL MODE FUNCTIONS *****************************
// ********************************************************************

static void zlhelp_copylist(t_atom *old, t_atom *new, int natoms){
  int i;
  for(i=0; i<natoms; i++){
      switch(old[i].a_type){
          case(A_FLOAT):
              SETFLOAT(&new[i], old[i].a_w.w_float);
              break;
          case(A_SYMBOL):
              SETSYMBOL(&new[i], old[i].a_w.w_symbol);
              break;
          case(A_POINTER):
              SETPOINTER(&new[i], old[i].a_w.w_gpointer);
              break;
          default:
              break;
      };
  };
}

static void zldata_realloc(t_zldata *d, int reqsz){
    int cursz = d->d_size;
	int curmax = d->d_max;
	int heaped = d->d_buf != d->d_bufini;
	if(reqsz > ZL_MAXSIZE)
        reqsz = ZL_MAXSIZE;
    else if(reqsz < ZL_MINSIZE)
        reqsz = ZL_MINSIZE;
    if(reqsz <= ZL_DEF_SIZE && heaped){
	    memcpy(d->d_bufini, d->d_buf, ZL_DEF_SIZE * sizeof(t_atom));
	    freebytes(d->d_buf, cursz * sizeof(t_atom));
	    d->d_buf = d->d_bufini;
    }
	else if(reqsz > ZL_DEF_SIZE && !heaped){
	    d->d_buf = getbytes(reqsz * sizeof(t_atom));
	    memcpy(d->d_buf, d->d_bufini, curmax * sizeof(t_atom));
    }
	else if(reqsz > ZL_DEF_SIZE && heaped)
	    d->d_buf = (t_atom *)resizebytes(d->d_buf, cursz*sizeof(t_atom), reqsz*sizeof(t_atom));
	d->d_max = reqsz;
	if(reqsz < ZL_DEF_SIZE)
        reqsz = ZL_DEF_SIZE;
	if(d->d_natoms > d->d_max)
        d->d_natoms = d->d_max;
    d->d_size = reqsz;
}

static void zldata_init(t_zldata *d, int sz){
    d->d_size = ZL_DEF_SIZE;
    d->d_natoms = 0;
    d->d_buf = d->d_bufini;
    if(sz > ZL_DEF_SIZE)
        zldata_realloc(d, sz);
}

static void zldata_free(t_zldata *d){
    if(d->d_buf != d->d_bufini)
        freebytes(d->d_buf, d->d_size * sizeof(*d->d_buf));
}

static void zldata_reset(t_zldata *d, int sz){
  zldata_free(d);
  zldata_init(d, sz);
}

static void zldata_setfloat(t_zldata *d, t_float f){
    SETFLOAT(d->d_buf, f);
    d->d_natoms = 1;
}

static void zldata_addfloat(t_zldata *d, t_float f){
    int natoms = d->d_natoms;
    int nrequested = natoms + 1;
    if (nrequested <= d->d_max)
        SETFLOAT(d->d_buf + natoms, f);
    d->d_natoms = natoms + 1;
}

static void zldata_setsymbol(t_zldata *d, t_symbol *s){
    SETSYMBOL(d->d_buf, s);
    d->d_natoms = 1;
}

static void zldata_addsymbol(t_zldata *d, t_symbol *s){
    int natoms = d->d_natoms;
    int nrequested = natoms + 1;
    if (nrequested <= d->d_max){
        SETSYMBOL(d->d_buf + natoms, s);
        d->d_natoms = natoms + 1;
    };
}

static void zldata_setlist(t_zldata *d, int ac, t_atom *av){
    int nrequested = ac;
    if (nrequested > d->d_max) ac = d->d_max;
    if(d->d_max >= ac)
      {
        memcpy(d->d_buf, av, nrequested * sizeof(*d->d_buf));
	d->d_natoms = ac;
      };
}

static void zldata_set(t_zldata *d, t_symbol *s, int ac, t_atom *av){
    if(s && s != &s_){
        int nrequested = ac + 1;
	if(nrequested > d->d_max) ac = d->d_max - 1;
        if(d->d_max >= nrequested){
            SETSYMBOL(d->d_buf, s);
            if(--nrequested)
                memcpy(d->d_buf + 1, av, nrequested * sizeof(*d->d_buf));
	    d->d_natoms = ac + 1;
        }
    }
    else
        zldata_setlist(d, ac, av);
}

static void zldata_addlist(t_zldata *d, int ac, t_atom *av){
    int natoms = d->d_natoms;
    int nrequested = natoms + ac;
    if(nrequested > d->d_max) ac = d->d_max - natoms; //truncate
    if(nrequested <= d->d_max){ //same reasoning as below, should be the case but mb okay to leave in check
        memcpy(d->d_buf + natoms, av, ac * sizeof(*d->d_buf));
		d->d_natoms = natoms + ac;
    };
}

static void zldata_add(t_zldata *d, t_symbol *s, int ac, t_atom *av){
    if(s && s != &s_){
        int natoms = d->d_natoms;
        int nrequested = natoms + 1 + ac;
	if(d->d_max < natoms + 1 + ac)
	  ac = d->d_max - 1 - natoms; //truncate
        if(d->d_max >= natoms + 1 + ac){ //should be the case but mb okay to leave this check in
            SETSYMBOL(d->d_buf + natoms, s);
            if (ac > 0)
                memcpy(d->d_buf + natoms + 1, av, ac * sizeof(*d->d_buf));
	    d->d_natoms = natoms + 1 + ac;
        };
    }
    else
        zldata_addlist(d, ac, av);
}

static void zl_dooutput(t_outlet *o, int ac, t_atom *av){
    if(ac > 1){
        if(av->a_type == A_FLOAT)
            outlet_list(o, &s_list, ac, av);
        else if (av->a_type == A_SYMBOL)
            outlet_anything(o, av->a_w.w_symbol, ac - 1, av + 1);
    }
    else if (ac){
        if (av->a_type == A_FLOAT)
            outlet_float(o, av->a_w.w_float);
        else if (av->a_type == A_SYMBOL)
            outlet_anything(o, av->a_w.w_symbol, 0, 0);
    }
}

static void zl_output(t_zl *x, int ac, t_atom *av){
    zl_dooutput(((t_object *)x)->ob_outlet, ac, av);
}

static void zl_output2(t_zl *x, int ac, t_atom *av){
    zl_dooutput(x->x_out2, ac, av);
}

static int zl_equal(t_atom *ap1, t_atom *ap2){
    return (ap1->a_type == ap2->a_type
	    &&
	    ((ap1->a_type == A_FLOAT
	      && ap1->a_w.w_float == ap2->a_w.w_float)
	     ||
	     (ap1->a_type == A_SYMBOL
	      && ap1->a_w.w_symbol == ap2->a_w.w_symbol)));
}

// ********************************************************************
// ************************* ZL MODES *********************************
// ********************************************************************

/* Mode handlers:
   If x_mode is positve, then the main routine uses an output 
   buffer 'buf' (outbuf, or a separately allocated one).
   If it's 0, then the main routine is passed a null 'buf' 
   (see below). And if negative, then the main routine isn't called.

   zl_<mode> (main routine) arguments:  if 'buf' is null, 'natoms'
   is always zero - in modes other than len (no buffer used), group,
   iter, reg, slice/ecils (inbuf1 used), there should be no output.
   If 'buf' is not null, then 'natoms' is guaranteed to be positive.
*/


// ************************* UNKNOWN *********************************

static int zl_nop_count(t_zl *x){
    return (0);
}

static void zl_nop(t_zl *x, int natoms, t_atom *buf, int banged){
    pd_error(x, "[zl]: unknown mode");
}

// ************************* ECILS *********************************

static int zl_ecils_intarg(t_zl *x, int i){
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_ecils_count(t_zl *x){
    return (x->x_entered ? -1 : 0);
}

static void zl_ecils(t_zl *x, int natoms, t_atom *buf, int banged){
    int cnt1, cnt2 = x->x_modearg;
    natoms = x->x_inbuf1.d_natoms;
    buf = x->x_inbuf1.d_buf;
    if (cnt2 > natoms)
        cnt2 = natoms, cnt1 = 0;  /* CHECKED */
    else
        cnt1 = natoms - cnt2;
    x->x_locked = 1;
    if (cnt2)
        zl_output2(x, cnt2, buf + cnt1);
    if (cnt1)
        zl_output(x, cnt1, buf);
}

// ************************* GROUP *********************************

static int zl_group_intarg(t_zl *x, int i){
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_group_count(t_zl *x){
    return (x->x_entered ? -1 : 0);
}

static void zl_group(t_zl *x, int natoms, t_atom *buf, int banged){
    int cnt = x->x_modearg;
    if(cnt > 0){
        natoms = x->x_inbuf1.d_natoms;
        buf = x->x_inbuf1.d_buf;
        if(cnt > x->x_inbuf1.d_max) cnt = x->x_inbuf1.d_max;
        if (natoms >= cnt){
            t_atom *from;

            x->x_locked = 1;
            for(from = buf; natoms >= cnt; natoms -= cnt, from += cnt)
                zl_output(x, cnt, from);
            x->x_inbuf1.d_natoms = natoms;
            while (natoms--)
                *buf++ = *from++;
        }
        if (banged && x->x_inbuf1.d_natoms){

            zl_output(x, x->x_inbuf1.d_natoms, buf);
            x->x_inbuf1.d_natoms = 0;
        }
    }
    else
        x->x_inbuf1.d_natoms = 0;
}

// ************************* ITER *********************************

static int zl_iter_intarg(t_zl *x, int i){
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_iter_count(t_zl *x){
    return (x->x_entered ?
	    (x->x_modearg < x->x_inbuf1.d_natoms ?
	     x->x_modearg : x->x_inbuf1.d_natoms)
	    : 0);
}

static void zl_iter(t_zl *x, int natoms, t_atom *buf, int banged){
    int nremaining = x->x_inbuf1.d_natoms;
    t_atom *ptr = x->x_inbuf1.d_buf;
    if(!buf){
        if(natoms = (x->x_modearg < nremaining ? x->x_modearg : nremaining))
            x->x_locked = 1;
        else
            return;
    }
    while(nremaining){
        if(natoms > nremaining)
            natoms = nremaining;
        if(buf){
            memcpy(buf, ptr, natoms * sizeof(*buf));
            zl_output(x, natoms, buf);
        }
        else
            zl_output(x, natoms, ptr);
	nremaining -= natoms;
	ptr += natoms;
    }
}

// ************************* JOIN *********************************

static void zl_join_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(!x->x_locked)
        zldata_set(&x->x_inbuf2, s, ac, av);
}

static int zl_join_count(t_zl *x){
    return (x->x_inbuf1.d_natoms + x->x_inbuf2.d_natoms);
}

static void zl_join(t_zl *x, int natoms, t_atom *buf, int banged){
    if(buf){
        int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms;
        if (ac1)
            memcpy(buf, x->x_inbuf1.d_buf, ac1 * sizeof(*buf));
        if (ac2)
            memcpy(buf + ac1, x->x_inbuf2.d_buf, ac2 * sizeof(*buf));
        zl_output(x, natoms, buf);
    }
}

// ************************* LEN *********************************

static int zl_len_count(t_zl *x){
  //return(x->x_inbuf1.d_natoms);
  return(0);
}

static void zl_len(t_zl *x, int natoms, t_atom *buf, int banged){
  //int inmax = x->x_inbuf1.d_max;
  //if(natoms > inmax) natoms = inmax;
  //x->x_inbuf1.d_natoms = natoms;
    outlet_float(((t_object *)x)->ob_outlet, x->x_inbuf1.d_natoms);
}

// ************************* NTH *********************************

static int zl_nth_intarg(t_zl *x, int i){
    return (i > 0 ? i : 0);
}

static void zl_nth_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(!s && ac && av->a_type == A_FLOAT)
        zldata_setlist(&x->x_inbuf2, ac - 1, av + 1);
}

static int zl_nth_count(t_zl *x){
    int ac1 = x->x_inbuf1.d_natoms;
    if(ac1){
        if(x->x_modearg > 0)
            return (ac1 - 1 + x->x_inbuf2.d_natoms);
        else
            return (x->x_entered ? ac1 : 0);
    }
    else return (-1);
}

static void zl_nth(t_zl *x, int natoms, t_atom *buf, int banged){
    int ac1 = x->x_inbuf1.d_natoms,
	ndx = x->x_modearg - 1;  // one-indexed
    if(ac1){
        t_atom *av1 = x->x_inbuf1.d_buf;
        if(ndx < 0 || ndx >= ac1){
            if(buf)
                memcpy(buf, av1, ac1 * sizeof(*buf));
            else{
                buf = av1;
                x->x_locked = 1;
            }
            zl_output2(x, ac1, buf);
        }
        else{
            t_atom at = av1[ndx];
            if(buf){
                int ac2 = x->x_inbuf2.d_natoms, ntail = ac1 - ndx - 1;
                t_atom *ptr = buf;
                if(ndx){
                    memcpy(ptr, av1, ndx * sizeof(*buf));
                    ptr += ndx;
                }
                if(ac2){  /* replacement */
                    memcpy(ptr, x->x_inbuf2.d_buf, ac2 * sizeof(*buf));
                    ptr += ac2;
                }
                if(ntail)
                    memcpy(ptr, av1 + ndx + 1, ntail * sizeof(*buf));
                zl_output2(x, natoms, buf);
            }
	    zl_output(x, 1, &at);
        }
    }
}

// ************************* MTH *********************************

static int zl_mth_intarg(t_zl *x, int i){
    return (i);
}

static void zl_mth_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(!s && ac && av->a_type == A_FLOAT)
        zldata_setlist(&x->x_inbuf2, ac - 1, av + 1);
}

static int zl_mth_count(t_zl *x){
    int ac1 = x->x_inbuf1.d_natoms;
    if(ac1){
        if(x->x_modearg >= 0)
            return (ac1 - 1 + x->x_inbuf2.d_natoms);
        else
            return (x->x_entered ? ac1 : 0);
    }
    else return(-1);
}

static void zl_mth(t_zl *x, int natoms, t_atom *buf, int banged){
    int ac1 = x->x_inbuf1.d_natoms,
    ndx = x->x_modearg;  // zero-indexed
    if(ac1){
        t_atom *av1 = x->x_inbuf1.d_buf;
        if(ndx < 0 || ndx >= ac1){
            if(buf)
                memcpy(buf, av1, ac1 * sizeof(*buf));
            else{
                buf = av1;
                x->x_locked = 1;
            }
            zl_output2(x, ac1, buf);
        }
        else{
            t_atom at = av1[ndx];
            if(buf){
                int ac2 = x->x_inbuf2.d_natoms, ntail = ac1 - ndx - 1;
                t_atom *ptr = buf;
                if(ndx){
                    memcpy(ptr, av1, ndx * sizeof(*buf));
                    ptr += ndx;
                }
                if(ac2){  /* replacement */
                    memcpy(ptr, x->x_inbuf2.d_buf, ac2 * sizeof(*buf));
                    ptr += ac2;
                }
                if(ntail)
                    memcpy(ptr, av1 + ndx + 1, ntail * sizeof(*buf));
                zl_output2(x, natoms, buf);
            }
            zl_output(x, 1, &at);
        }
    }
}

// ************************* REG *********************************

static void zl_reg_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if (!x->x_locked)
        zldata_set(&x->x_inbuf1, s, ac, av);
}

static int zl_reg_count(t_zl *x){
    return (x->x_entered ? x->x_inbuf1.d_natoms : 0);
}

static void zl_reg(t_zl *x, int natoms, t_atom *buf, int banged){
    if(buf)
        memcpy(buf, x->x_inbuf1.d_buf, natoms * sizeof(*buf));
    else{
        natoms = x->x_inbuf1.d_natoms;
        buf = x->x_inbuf1.d_buf;
        x->x_locked = 1;
    }
    if (natoms)
        zl_output(x, natoms, buf);
}

// ************************* REV *********************************

static int zl_rev_count(t_zl *x){
    return (x->x_inbuf1.d_natoms);
}

static void zl_rev(t_zl *x, int natoms, t_atom *buf, int banged){
    if (buf){
        t_atom *from = x->x_inbuf1.d_buf, *to = buf + natoms;
        while (to-- > buf)
            *to = *from++;
        zl_output(x, natoms, buf);
    }
}

// ************************* ROT *********************************

static int zl_rot_intarg(t_zl *x, int i){
    return (i);  // CHECKED anything goes (modulo)
}

static int zl_rot_count(t_zl *x){
    return (x->x_inbuf1.d_natoms);
}

static void zl_rot(t_zl *x, int natoms, t_atom *buf, int banged){
    if(buf){
        int cnt1 = x->x_modearg, cnt2;
        if(cnt1){
            if (cnt1 > 0){
                cnt1 %= natoms;
                cnt2 = natoms - cnt1;
            }
            else{
                cnt2 = -cnt1 % natoms;
                cnt1 = natoms - cnt2;
            }
            /* CHECKED right rotation for positive args */
            memcpy(buf, x->x_inbuf1.d_buf + cnt2, cnt1 * sizeof(*buf));
            memcpy(buf + cnt1, x->x_inbuf1.d_buf, cnt2 * sizeof(*buf));
        }
	else
        memcpy(buf, x->x_inbuf1.d_buf, natoms * sizeof(*buf));
	zl_output(x, natoms, buf);
    }
}

// ************************* SECT *********************************

static void zl_sect_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if (!x->x_locked)
        zldata_set(&x->x_inbuf2, s, ac, av);
}

/* LATER rethink */
static int zl_sect_count(t_zl *x){
    int result = 0;
    int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i1;
    t_atom *av1 = x->x_inbuf1.d_buf, *av2 = x->x_inbuf2.d_buf, *ap1;
    for(i1 = 0, ap1 = av1; i1 < ac1; i1++, ap1++){
        int i2;
        t_atom *testp;
        for(i2 = 0, testp = av1; i2 < i1; i2++, testp++)
            if(zl_equal(ap1, testp))
                goto
                skip;
        for (i2 = 0, testp = av2; i2 < ac2; i2++, testp++){
            if (zl_equal(ap1, testp)){
                result++;
                break;
            }
        }
    skip:;
    }
    return(result);
}

// CHECKED in-buffer duplicates are skipped
static void zl_sect(t_zl *x, int natoms, t_atom *buf, int banged){
    if(!natoms)
        outlet_bang(x->x_out2);
    if(buf){
        int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i1;
        t_atom *ap1 = x->x_inbuf1.d_buf, *av2 = x->x_inbuf2.d_buf, *to = buf;
        for(i1 = 0; i1 < ac1; i1++, ap1++){
            int i2;
            t_atom *testp;
            for(testp = buf; testp < to; testp++)
                if(zl_equal(ap1, testp))
                    goto
                        skip;
            for(i2 = 0, testp = av2; i2 < ac2; i2++, testp++){
                if(zl_equal(ap1, testp)){
                    *to++ = *ap1;
                    break;
                }
            }
        skip:;
        }
    zl_output(x, natoms, buf);
    }
}

// ************************* SLICE *********************************

static int zl_slice_intarg(t_zl *x, int i){
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_slice_count(t_zl *x){
    return (x->x_entered ? -1 : 0);
}

static void zl_slice(t_zl *x, int natoms, t_atom *buf, int banged){
    int cnt1 = x->x_modearg, cnt2;
    natoms = x->x_inbuf1.d_natoms;
    buf = x->x_inbuf1.d_buf;
    if(cnt1 > natoms)
        cnt1 = natoms, cnt2 = 0;  /* CHECKED */
    else
        cnt2 = natoms - cnt1;
    x->x_locked = 1;
    if(cnt2)
        zl_output2(x, cnt2, buf + cnt1);
    if(cnt1)
        zl_output(x, cnt1, buf);
}

// ************************* SORT *********************************

static void zl_sort_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    //
}

static int zl_sort_count(t_zl *x){
    //
}

static void zl_sort(t_zl *x, int natoms, t_atom *buf, int banged){
    //
}

// ************************* SUB *********************************

static void zl_sub_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(!x->x_locked)
        zldata_set(&x->x_inbuf2, s, ac, av);
}

static int zl_sub_count(t_zl *x){
    return(0);
}

static void zl_sub(t_zl *x, int natoms, t_atom *buf, int banged){
    int natoms2 = x->x_inbuf2.d_natoms;
    if(natoms2){
        int found = 0;
        int ndx1, natoms1 = x->x_inbuf1.d_natoms;
        t_atom *av1 = x->x_inbuf1.d_buf, *av2 = x->x_inbuf2.d_buf;
        for(ndx1 = 0; ndx1 < natoms1; ndx1++){
            int indx2;
            t_atom *ap1 = av1 + ndx1, *ap2 = av2;
            for(indx2 = 0; indx2 < natoms2; indx2++, ap1++, ap2++)
                if(!zl_equal(ap1, ap2))
                    break;
            if(indx2 == natoms2)
                found++;
        }
        outlet_float(x->x_out2, found);
        for(ndx1 = 0; ndx1 < natoms1; ndx1++, av1++){
            int ndx2;
            t_atom *ap3 = av1, *ap4 = av2;
            for(ndx2 = 0; ndx2 < natoms2; ndx2++, ap3++, ap4++)
                if(!zl_equal(ap3, ap4))
                    break;
                if(ndx2 == natoms2)
                    outlet_float(((t_object *)x)->ob_outlet, ndx1 + 1);
        }
    }
}

// ************************* UNION *********************************

static void zl_union_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(!x->x_locked)
        zldata_set(&x->x_inbuf2, s, ac, av);
}

/* LATER rethink */
static int zl_union_count(t_zl *x){
    int result, ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i2;
    t_atom *av1 = x->x_inbuf1.d_buf, *ap2 = x->x_inbuf2.d_buf;
    result = ac1 + ac2;
    for (i2 = 0; i2 < ac2; i2++, ap2++){
        int i1;
        t_atom *ap1;
        for (i1 = 0, ap1 = av1; i1 < ac1; i1++, ap1++){
            if (zl_equal(ap1, ap2)){
                result--;
                break;
            }
        }
    }
    return (result);
}

/* CHECKED in-buffer duplicates not skipped */
static void zl_union(t_zl *x, int natoms, t_atom *buf, int banged){
    if (buf){
        int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i2;
        t_atom *av1 = x->x_inbuf1.d_buf, *ap2 = x->x_inbuf2.d_buf;
        if (ac1){
            t_atom *to = buf + ac1;
            memcpy(buf, av1, ac1 * sizeof(*buf));
            for (i2 = 0; i2 < ac2; i2++, ap2++){
                int i1;
                t_atom *ap1;
                for (i1 = 0, ap1 = av1; i1 < ac1; i1++, ap1++)
                    if (zl_equal(ap1, ap2))
                        break;
                if (i1 == ac1)
                    *to++ = *ap2;
            }
        }
	else
        memcpy(buf, ap2, ac2 * sizeof(*buf));
	zl_output(x, natoms, buf);
    }
}

// ********************************************************************
// ************************* METHODS **********************************
// ********************************************************************

static void zl_doit(t_zl *x, int banged){
    int reentered = x->x_entered;
    int prealloc = !reentered;
    int natoms = (*zl_natomsfn[x->x_mode])(x);
    if(natoms < 0)
        return;
    x->x_entered = 1;
    if(natoms){
        t_zldata *d = &x->x_outbuf;
        if(natoms > d->d_max) // giving this a shot...
            natoms = d->d_max;
        // basically will limit output buffer to specified size instead of allowing it to go over...
        if(prealloc)
            (*zl_doitfn[x->x_mode])(x, natoms, d->d_buf, banged);
    }
    else
        (*zl_doitfn[x->x_mode])(x, 0, 0, banged);
    if(!reentered)
        x->x_entered = x->x_locked = 0;
}

static void zl_bang(t_zl *x){
    zl_doit(x, 1);
}

static void zl_float(t_zl *x, t_float f){
    if (!x->x_locked){
        if (zl_modeflags[x->x_mode])
            zldata_addfloat(&x->x_inbuf1, f);
        else
            zldata_setfloat(&x->x_inbuf1, f);
    }
    zl_doit(x, 0);
}

static void zl_symbol(t_zl *x, t_symbol *s){
    if (!x->x_locked){
        if (zl_modeflags[x->x_mode])
            zldata_addsymbol(&x->x_inbuf1, s);
        else
            zldata_setsymbol(&x->x_inbuf1, s);
    }
    zl_doit(x, 0);
}

static void zl_list(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(!x->x_locked){
        if(zl_modeflags[x->x_mode])
            zldata_addlist(&x->x_inbuf1, ac, av);
        else
            zldata_setlist(&x->x_inbuf1, ac, av);
    }
    zl_doit(x, 0);
}

static void zl_anything(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(!x->x_locked){
        if(zl_modeflags[x->x_mode])
            zldata_add(&x->x_inbuf1, s, ac, av);
        else
            zldata_set(&x->x_inbuf1, s, ac, av);
    }
    zl_doit(x, 0);
}


static int zl_modeargfn(t_zl *x){
    return (zl_intargfn[x->x_mode] || zl_anyargfn[x->x_mode]);
}

static void zl_setmodearg(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(zl_intargfn[x->x_mode]){
        int i = (!s && ac && av->a_type == A_FLOAT ?
                 (int)av->a_w.w_float :  /* CHECKED silent truncation */
                 0);  /* CHECKED current x->x_modearg not kept */
        x->x_modearg = (*zl_intargfn[x->x_mode])(x, i);
    }
    if (zl_anyargfn[x->x_mode])
        (*zl_anyargfn[x->x_mode])(x, s, ac, av);
}

static void zlproxy_bang(t_zlproxy *d){
    // nop
}

static void zlproxy_float(t_zlproxy *p, t_float f){
    t_zl *x = p->p_master;
    if (zl_modeargfn(x)){
        t_atom at;
        SETFLOAT(&at, f);
        zl_setmodearg(x, 0, 1, &at);
    }
    else  /* CHECKED inbuf2 filled only when used */
        zldata_setfloat(&x->x_inbuf2, f);
}

static void zlproxy_symbol(t_zlproxy *p, t_symbol *s){
    t_zl *x = p->p_master;
    if (zl_modeargfn(x)){
        t_atom at;
        SETSYMBOL(&at, s);
        zl_setmodearg(x, 0, 1, &at);
    }
    else  /* CHECKED inbuf2 filled only when used */
        zldata_setsymbol(&x->x_inbuf2, s);
}

/* LATER gpointer */

static void zlproxy_list(t_zlproxy *p, t_symbol *s, int ac, t_atom *av){
    if (ac){
        t_zl *x = p->p_master;
        if (zl_modeargfn(x))
            zl_setmodearg(x, 0, ac, av);
        else  /* CHECKED inbuf2 filled only when used */
            zldata_setlist(&x->x_inbuf2, ac, av);
    }
}

static void zlproxy_anything(t_zlproxy *p, t_symbol *s, int ac, t_atom *av){
    t_zl *x = p->p_master;
    if (zl_modeargfn(x))
        zl_setmodearg(x, s, ac, av);
    else  /* CHECKED inbuf2 filled only when used */
        zldata_set(&x->x_inbuf2, s, ac, av);
}

static void zl_free(t_zl *x){
    zldata_free(&x->x_inbuf1);
    zldata_free(&x->x_inbuf2);
    zldata_free(&x->x_outbuf);
    if(x->x_proxy)
        pd_free((t_pd *)x->x_proxy);
}

static void zl_mode(t_zl *x, t_symbol *s, int ac, t_atom *av){
    if(ac && av->a_type == A_SYMBOL){
        t_symbol *modesym = av->a_w.w_symbol;
        int i;
        for(i = 0; i < ZL_N_MODES; i++){
            if(modesym == zl_modesym[i]){
            x->x_mode = i;
            zl_setmodearg(x, 0, ac - 1, av + 1);
            break;
            }
        }
    }
}

static void zl_zlmaxsize(t_zl *x, t_floatarg f){
    int sz = (int)f;
    zldata_realloc(&x->x_inbuf1,sz);
    zldata_realloc(&x->x_inbuf2,sz);
    zldata_realloc(&x->x_outbuf,sz);
    
}

static void zl_zlclear(t_zl *x){
    int sz1 = x->x_inbuf1.d_size;
    int sz2 = x->x_inbuf2.d_size;
    int sz3 = x->x_outbuf.d_size;
    zldata_reset(&x->x_inbuf1, sz1);
    zldata_reset(&x->x_inbuf2, sz2);
    zldata_reset(&x->x_outbuf, sz3);
}

static void *zl_new(t_symbol *s, int argc, t_atom *argv){
    t_zl *x = (t_zl *)pd_new(zl_class);
    t_zlproxy *y = (t_zlproxy *)pd_new(zlproxy_class);
    x->x_proxy = y;
    y->p_master = x;
    x->x_entered = 0;
    x->x_locked = 0;
    x->x_mode = 0; // Unkown mode
    int sz = ZL_DEF_SIZE;
    int first_arg = 0;
    int size_arg = 0;
    int size_attr = 0;
    int i = argc;
    t_atom *a = argv;
    while(i){
        if(a->a_type == A_FLOAT){
            if(!first_arg){ // no first arg yet, get size
                sz = (int)atom_getfloatarg(0, i, a);
                first_arg = size_arg = 1;
            }
            i--; // iterate
            a++;
        };
        if(a->a_type == A_SYMBOL){ // is symbol
            if(!first_arg) // is first arg, so mark it
                first_arg = 1;
            t_symbol * cursym = atom_getsymbolarg(0, i, a);
            if(!strcmp(cursym->s_name, "@zlmaxsize")){ // is the attribute
                i--;
                a++;
                if(i == 1){
                    if(a->a_type == A_FLOAT){
                        sz  = (int)atom_getfloatarg(0, i, a);
                        size_attr = 2;
                    }
                    else
                        goto errstate;
                };
                if(i != 1)
                    goto errstate;
            };
            i--;
            a++;
        };
    };
    if(sz < ZL_MINSIZE)
        sz = ZL_MINSIZE;
    if(sz > ZL_MAXSIZE)
        sz = ZL_MAXSIZE;
    x->x_inbuf1.d_max = sz;
    x->x_inbuf2.d_max = sz;
    x->x_outbuf.d_max = sz;
    zldata_init(&x->x_inbuf1, sz);
    zldata_init(&x->x_inbuf2, sz);
    zldata_init(&x->x_outbuf, sz);
    zl_mode(x, s, argc - size_arg - size_attr, argv + size_arg);
    if(!x->x_mode)
        pd_error(x, "[zl]: unknown mode (needs a symbol argument)");
    inlet_new((t_object *)x, (t_pd *)y, 0, 0);
    outlet_new((t_object *)x, &s_anything);
    x->x_out2 = outlet_new((t_object *)x, &s_anything);
    return(x);
errstate:
    post("zl: improper args");
    return NULL;
}

static void zl_setupmode(char *id, int flags,
        t_zlintargfn ifn, t_zlanyargfn afn, t_zlnatomsfn nfn, t_zldoitfn dfn, int i){
    zl_modesym[i] = gensym(id);
    zl_modeflags[i] = flags;
    zl_intargfn[i] = ifn;
    zl_anyargfn[i] = afn;
    zl_natomsfn[i] = nfn;
    zl_doitfn[i] = dfn;
}

void zl_setup(void){
    zl_class = class_new(gensym("zl"), (t_newmethod)zl_new,
            (t_method)zl_free, sizeof(t_zl), 0, A_GIMME, 0);
    class_addbang(zl_class, zl_bang);
    class_addfloat(zl_class, zl_float);
    class_addsymbol(zl_class, zl_symbol);
    class_addlist(zl_class, zl_list);
    class_addanything(zl_class, zl_anything);
    class_addmethod(zl_class, (t_method)zl_mode, gensym("mode"), A_GIMME, 0);
    class_addmethod(zl_class, (t_method)zl_zlmaxsize, gensym("zlmaxsize"), A_FLOAT, 0);
    class_addmethod(zl_class, (t_method)zl_zlclear, gensym("zlclear"), 0);
    zlproxy_class = class_new(gensym("_zlproxy"), 0, 0,
        sizeof(t_zlproxy), CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(zlproxy_class, zlproxy_bang);
    class_addfloat(zlproxy_class, zlproxy_float);
    class_addsymbol(zlproxy_class, zlproxy_symbol);
    class_addlist(zlproxy_class, zlproxy_list);
    class_addanything(zlproxy_class, zlproxy_anything);
    zl_setupmode("unknown", 0, 0, 0, zl_nop_count, zl_nop, 0);
    zl_setupmode("ecils", 0, zl_ecils_intarg, 0, zl_ecils_count, zl_ecils, 1);
    zl_setupmode("group", 1, zl_group_intarg, 0, zl_group_count, zl_group, 2);
    zl_setupmode("iter", 0, zl_iter_intarg, 0, zl_iter_count, zl_iter, 3);
    zl_setupmode("join", 0, 0, zl_join_anyarg, zl_join_count, zl_join, 4);
    zl_setupmode("len", 0, 0, 0, zl_len_count, zl_len, 5);
    zl_setupmode("mth", 0, zl_mth_intarg, zl_mth_anyarg, zl_mth_count, zl_mth, 6);
    zl_setupmode("nth", 0, zl_nth_intarg, zl_nth_anyarg, zl_nth_count, zl_nth, 7);
    zl_setupmode("reg", 0, 0, zl_reg_anyarg, zl_reg_count, zl_reg, 8);
    zl_setupmode("rev", 0, 0, 0, zl_rev_count, zl_rev, 9);
    zl_setupmode("rot",	0, zl_rot_intarg, 0, zl_rot_count, zl_rot, 10);
    zl_setupmode("sect", 0, 0, zl_sect_anyarg, zl_sect_count, zl_sect, 11);
    zl_setupmode("slice", 0, zl_slice_intarg, 0, zl_slice_count, zl_slice, 12);
    zl_setupmode("sort", 0, 0, zl_sort_anyarg, zl_sort_count, zl_sort, 13);
    zl_setupmode("sub", 0, 0, zl_sub_anyarg, zl_sub_count, zl_sub, 14);
    zl_setupmode("union", 0, 0, zl_union_anyarg, zl_union_count, zl_union, 15);
}
