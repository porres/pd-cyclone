// Porres 2016

#include "m_pd.h"
#include <math.h>

#define PI M_PI

typedef struct _filtercoeff {
    t_object    x_obj;
    t_int       x_n;
    t_inlet    *x_inlet_gain;
    t_inlet    *x_inlet_q;
    t_outlet   *x_out_a0;
    t_outlet   *x_out_a1;
    t_outlet   *x_out_a2;
    t_outlet   *x_out_b1;
    t_outlet   *x_out_b2;
    t_float     x_nyq;
    t_float     x_lastq;
    t_int       x_mode;
    } t_filtercoeff;

static t_class *filtercoeff_class;




void filtercoeff_list(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
// freq, gain, q
}

void filtercoeff_lowpass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 0;
}

void filtercoeff_highpass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 1;
}

void filtercoeff_bandpass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 2;
}

void filtercoeff_bandstop(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 3;
}

void filtercoeff_resonant(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 4;
}

void filtercoeff_peaknotch(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 5;
}

void filtercoeff_lowshelf(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 6;
}

void filtercoeff_highshelf(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 7;
}

void filtercoeff_allpass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 8;
}

void filtercoeff_gainlpass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 9;
}

void filtercoeff_gainhpass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 10;
}

void filtercoeff_gainbpass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 11;
}

void filtercoeff_gainbstop(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 12;
}

void filtercoeff_gainresonant(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 13;
}

void filtercoeff_gainapass(t_filtercoeff *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mode = 14;
}

void filtercoeff_off(t_filtercoeff *x)
{
    x->x_mode = 15;
}

static t_int *filtercoeff_perform(t_int *w)
{
    t_filtercoeff *x = (t_filtercoeff *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out1 = (t_float *)(w[6]);
    t_float *out2 = (t_float *)(w[7]);
    t_float *out3 = (t_float *)(w[8]);
    t_float *out4 = (t_float *)(w[9]);
    t_float *out5 = (t_float *)(w[10]);
    t_float nyq = x->x_nyq;
    t_float lastq = x->x_lastq;
    while (nblock--)
    {
        float omega, a0, a1, a2, b0, b1, b2, f = *in1++, g = *in2++, q = *in3++;
        if (f < 0) f = 0;
        if (f > nyq) f = nyq;
        omega = f * PI/nyq;
        switch (x->x_mode)
        {
            case 0:  // lowpass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = (1-cos_w) / (2*b0);
                a1 = (1-cos_w) / b0;
                a2 = a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 1: // highpass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = (1+cos_w) / (2*b0);
                a1 = -(1+cos_w) / b0;
                a2 = a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 2: // bandpass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = alphaQ / b0;
                a1 = 0;
                a2 = -a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 3: // bandstop
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = 1 / b0;
                a1 = -2*cos_w / b0;
                a2 = a0;
                b1 = a1;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 4: // resonant
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = alphaQ*q / b0;
                a1 = 0;
                a2 = -a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 5: // peaknotch
            {
                if (q <= 0)
                {
                    a0 = g;
                    a1 = 0;
                    a2 = -g;
                    b1 = 0;
                    b2 = -1;
                }
                else
                {
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                g = sqrtf(g);
                b0 = alphaQ/g + 1;
                a0 = (1 + alphaQ*g) / b0;
                a1 = -2*cos_w / b0;
                a2 = (1 - alphaQ*g) / b0;;
                b1 = a1;
                b2 = (1 - alphaQ/g) / b0;
                }
            }
                break;
            case 6: // lowshelf
            {
                if (q <= 0) q = 1; // ???????????????????????
                g = sqrtf(g);
                t_float alphaS = sinf(omega) * sqrtf((g*g + 1) * (1/q - 1) + 2*g);
                t_float cos_w = cosf(omega);
                b0 = g+1 + (g-1)*cos_w + alphaS;
                a0 = g*(g+1 - (g-1)*cos_w + alphaS) / b0;
                a1 = 2*g*(g-1 - (g+1)*cos_w) / b0;
                a2 = g*(g+1 - (g-1)*cos_w - alphaS) / b0;;
                b1 = -2*(g-1 + (g+1)*cos_w) / b0;
                b2 = (g+1 + (g-1)*cos_w - alphaS) / b0;
            }
                break;
            case 7:  // highshelf
            {
                if (q <= 0) q = 1; // ???????????????????????
                g = sqrtf(g);
                t_float alphaS = sinf(omega) * sqrtf((g*g + 1) * (1/q - 1) + 2*g);
                t_float cos_w = cosf(omega);
                b0 = g+1 - (g-1)*cos_w + alphaS;
                a0 = g*(g+1 + (g-1)*cos_w + alphaS) / b0;
                a1 = -2*g*(g-1 + (g+1)*cos_w) / b0;
                a2 = g*(g+1 + (g-1)*cos_w - alphaS) / b0;
                b1 = 2*(g-1 - (g+1)*cos_w) / b0;
                b2 = (g+1 - (g-1)*cos_w - alphaS) / b0;
            }
                break;
            case 8:  // allpass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = (1 - alphaQ) / b0;
                a1 = -2*cos_w / b0;
                a2 = 1;
                b1 = a1;
                b2 = a0;
            }
                break;
            case 9:  // gainlpass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = (1-cos_w)  * g/(2*b0);
                a1 = (1-cos_w)  * g/b0;
                a2 = a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 10: // gainhpass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = (1+cos_w)  * g/(2*b0);
                a1 = -(1+cos_w)  * g/b0;
                a2 = a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 11: // gainbpass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = alphaQ  * g/b0;
                a1 = 0;
                a2 = -a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 12: // gainbstop
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = g / b0;
                a1 = -2*cos_w  * g/b0;
                a2 = a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 13: // gainresonant
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = alphaQ*q  * g/b0;
                a1 = 0;
                a2 = -a0;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 14:  // gainapass
            {
                if (q <= 0) q = 1; // ???????????????????????
                t_float alphaQ = sinf(omega) / (2*q);
                t_float cos_w = cosf(omega);
                b0 = alphaQ + 1;
                a0 = (1 - alphaQ) * g/b0;
                a1 = -2*cos_w  * g/b0;
                a2 = g;
                b1 = -2*cos_w / b0;
                b2 = (1 - alphaQ) / b0;
            }
                break;
            case 15:  // off
            {
                a0 = 1;
                a1 = 0;
                a2 = 0;
                b1 = 0;
                b2 = 0;
            }
                break;
        }
        *out1++ = a0;
        *out2++ = a1;
        *out3++ = a2;
        *out4++ = b1;
        *out5++ = b2;
    }
    x->x_lastq = lastq;
    return (w + 11);
}

static void filtercoeff_dsp(t_filtercoeff *x, t_signal **sp)
{
    x->x_nyq = sp[0]->s_sr / 2;
    dsp_add(filtercoeff_perform, 10, x, sp[0]->s_n, sp[0]->s_vec,sp[1]->s_vec, sp[2]->s_vec,
            sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec);
}

static void *filtercoeff_new(t_symbol *s, int argc, t_atom *argv)
{
    t_filtercoeff *x = (t_filtercoeff *)pd_new(filtercoeff_class);
    t_symbol * type;
    x->x_mode = 15;
    int i;
    x->x_n = argc;
    for(i = 0; i < x->x_n; ++i)
    {
        if(argc > 0 && argv ->a_type == A_SYMBOL)
        {
            type = atom_getsymbolarg(0, argc, argv);
            if (type == gensym("lowpass")) x->x_mode = 0;
            else if(type == gensym("highpass")) x->x_mode = 1;
            else if(type == gensym("bandpass")) x->x_mode = 2;
            else if(type == gensym("bandstop")) x->x_mode = 3;
            else if(type == gensym("resonant")) x->x_mode = 4;
            else if(type == gensym("peaknotch")) x->x_mode = 5;
            else if(type == gensym("lowshelf")) x->x_mode = 6;
            else if(type == gensym("highshelf")) x->x_mode = 7;
            else if(type == gensym("allpass")) x->x_mode = 8;
            else if(type == gensym("gainlpass")) x->x_mode = 9;
            else if(type == gensym("gainhpass")) x->x_mode = 10;
            else if(type == gensym("gainbpass")) x->x_mode = 11;
            else if(type == gensym("gainbstop")) x->x_mode = 12;
            else if(type == gensym("gainresonant")) x->x_mode = 13;
            else if(type == gensym("gainapass")) x->x_mode = 14;
            else x->x_mode = 15;
        }
    }
    x->x_inlet_gain = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_gain, 1); // get arg value
    x->x_inlet_q = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_inlet_q, 1); // get arg value
    x->x_out_a0 = outlet_new((t_object *)x, &s_signal);
    x->x_out_a1 = outlet_new((t_object *)x, &s_signal);
    x->x_out_a2 = outlet_new((t_object *)x, &s_signal);
    x->x_out_b1 = outlet_new((t_object *)x, &s_signal);
    x->x_out_b2 = outlet_new((t_object *)x, &s_signal);
    x->x_lastq = 1; // ????????????????????????????????????????????????????????????
  //  pd_error(x, "[cyclone/filtercoeff~] is not ready yet");
    return (x);
}

void filtercoeff_tilde_setup(void)
{
    filtercoeff_class = class_new(gensym("filtercoeff~"), (t_newmethod)filtercoeff_new, 0,
        sizeof(t_filtercoeff), CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method)filtercoeff_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(filtercoeff_class, nullfn, gensym("signal"), 0);
    class_addlist(filtercoeff_class,(t_method)filtercoeff_list);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_off, gensym("off"), 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_lowpass, gensym("lowpass"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_highpass, gensym("highpass"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_bandpass, gensym("bandpass"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_bandstop, gensym("bandstop"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_resonant, gensym("resonant"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_peaknotch, gensym("peaknotch"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_lowshelf, gensym("lowshelf"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_highshelf, gensym("highshelf"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_allpass, gensym("allpass"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_gainlpass, gensym("gainlpass"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_gainhpass, gensym("gainhpass"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_gainbpass, gensym("gainbpass"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_gainbstop, gensym("gainbstop"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_gainresonant, gensym("gainresonant"), A_GIMME, 0);
    class_addmethod(filtercoeff_class, (t_method) filtercoeff_gainapass, gensym("gainapass"), A_GIMME, 0);
}
