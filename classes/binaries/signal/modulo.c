// sames as [%~]

#include "m_pd.h"
#include "math.h"
#include "sickle/sic.h"

#define MODULO_DEFRHS  0.

typedef t_sic t_modulo;
static t_class *modulo_class;

static t_int *modulo_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
        *out++ = (f2 == 0. ? 0. : fmod(f1, f2));
    }
    return (w + 5);
}

static void modulo_dsp(t_modulo *x, t_signal **sp)
{
    dsp_add(modulo_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *modulo_new(t_symbol *s, int ac, t_atom *av)
{
    t_modulo *x = (t_modulo *)pd_new(modulo_class);
    sic_inlet((t_sic *)x, 1, MODULO_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void modulo_tilde_setup(void)
{
    modulo_class = class_new(gensym("modulo~"),
			      (t_newmethod)modulo_new, 0,
			      sizeof(t_modulo), 0, A_GIMME, 0);
    sic_setup(modulo_class, modulo_dsp, SIC_FLOATTOSIGNAL);
}
