/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include <common/api.h>

static t_class *cartopol_tilde_class;

typedef struct _cartopol_tilde{
    t_object x_obj;
}t_cartopol_tilde;

t_int *cartopol_tilde_perform(t_int *w){
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out1 = (t_sample *)(w[3]);
    t_sample *out2 = (t_sample *)(w[4]);
    int n = (int)(w[5]);
    while(n--){
        t_sample f1 = *in1++, f2 = *in2++;
        *out1++ = sqrt(f1*f1 + f2*f2);
        *out2++ = atan2(f2, f1);
    }
    return(w+6);
}

static void cartopol_tilde_dsp(t_cartopol_tilde *x, t_signal **sp){
    x = NULL;
    int n1 = sp[0]->s_length * sp[0]->s_nchans;
    int n2 = sp[1]->s_length * sp[1]->s_nchans;
    int outchans = sp[0]->s_nchans;
    signal_setmultiout(&sp[2], outchans);
    signal_setmultiout(&sp[3], outchans);
    if(sp[0]->s_nchans != sp[1]->s_nchans){
        pd_error(x, "[cartopol~]: number of channels mismatch");
        return;
    }
    t_sample *in1 = sp[0]->s_vec, *in2 = sp[1]->s_vec;
    t_sample *out1 = sp[2]->s_vec, *out2 = sp[3]->s_vec;
    for(int i = (n1+n2-1)/n1; i--; ){
        t_int blocksize = (n1 < n2 - i*n1 ? n1 : n2 - i*n1);
        dsp_add(cartopol_tilde_perform, 5, in1, in2 + i*n1,
                out1 + i*n1, out2 + i*n1, blocksize);
    }
}

static void *cartopol_tilde_new(void){
    t_cartopol_tilde *x = (t_cartopol_tilde *)pd_new(cartopol_tilde_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    return(x);
}

CYCLONE_OBJ_API void cartopol_tilde_setup(void){
    cartopol_tilde_class = class_new(gensym("cartopol~"), (t_newmethod)cartopol_tilde_new, 0,
        sizeof(t_cartopol_tilde), CLASS_MULTICHANNEL, 0);
    class_addmethod(cartopol_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(cartopol_tilde_class, (t_method)cartopol_tilde_dsp, gensym("dsp"), A_CANT, 0);
}
