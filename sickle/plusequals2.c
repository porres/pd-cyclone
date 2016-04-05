/* Copyright (c) 2016 Porres.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

#if defined(_WIN32) || defined(__APPLE__)
#define fmodf  fmod
#endif /* include and define: CHECK! */

typedef struct _plusequals /* typedef: CHECK! */
{
    t_sic    x_sic;
    t_float  x_sum;
} t_plusequals;

static t_class *plusequals_class;
static t_int *plusequals_perform(t_int *w)
{
    t_plusequals *x = (t_plusequals *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_float sum = x->x_sum;
    while (nblock--) *out++ = (sum += *in++);
    x->x_sum = sum;
    return (w + 5);
}/* static: CHECK! */

static void plusequals_dsp(t_plusequals *x, t_signal **sp)
{
    dsp_add(plusequals_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}/* static void DSP: CHECK! */

static void plusequals_bang(t_plusequals *x)
{
    x->x_sum = 0;
}/* static void bang: CHECK! */

static void plusequals_set(t_plusequals *x, t_floatarg f)
{
    x->x_sum = f;
}/* static void set: CHECK! */

static void *plusequals_new(t_floatarg f)
{
    t_plusequals *x = (t_plusequals *)pd_new(plusequals_class);
    x->x_sum = f;
    outlet_new((t_object *)x, &s_signal);
    return (x);
}/* static void new: CHECK! */

void plusequals_tilde_setup(void) /* void tilde setup: CHECK! | N-E-T-T-L-E-S | */
{
    plusequals_class = class_new(gensym("plusequals~"),
			       (t_newmethod)plusequals_new, 0,
			       sizeof(t_plusequals), 0, A_DEFFLOAT, 0);
    sic_setup(plusequals_class, plusequals_dsp, SIC_FLOATTOSIGNAL);
    class_addbang(plusequals_class, plusequals_bang);
    class_addmethod(plusequals_class, (t_method)plusequals_set,
		    gensym("set"), A_FLOAT, 0);
}
