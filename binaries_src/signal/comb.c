//almost complete rewrite since the previous version of Cyclone didn't implement it correctly (used only one delay buffer)
//- Derek Kwan 2016

#include <math.h>
#include <stdlib.h>
#include "m_pd.h"

#define COMB_STACK 48000 //stack buf size, 1 sec at 48k for good measure
#define COMB_DELAY  10.0 //maximum delay
#define COMB_MAX 240000 //max allocated amount for delay bufs, 5 seconds at 48k

static t_class *comb_class;

typedef struct _comb
{
    t_object  x_obj;
    t_inlet  *x_dellet;
    t_inlet  *x_alet;
    t_inlet  *x_blet;
    t_inlet  *x_clet;
    t_outlet  *x_outlet;
    int     x_sr;
    //pointers to the delay bufs
    double  * x_ybuf; 
    double x_ffstack[COMB_STACK];
    double * x_xbuf;
    double x_fbstack[COMB_STACK];
    int     x_alloc; //if we are using allocated bufs
    int     x_sz; //actual size of each delay buffer

    t_float     x_maxdel;  //maximum delay in ms
    int       x_wh;     //writehead
} t_comb;

static void comb_clear(t_comb *x){
    int i;
    for(i=0; i<x->x_sz; i++){
        x->x_xbuf[i] = 0.;
        x->x_ybuf[i] = 0.;
    };
    x->x_wh = 0;
}

static void comb_sz(t_comb *x){
    //helper function to deal with allocation issues if needed
    //ie if wanted size x->x_maxdel is bigger than stack, allocate
    
    //convert ms to samps
    int newsz = (int)ceil((double)x->x_maxdel*0.001*(double)x->x_sr);
    newsz++; //add a sample for good measure since say bufsize is 2048 and 
    //you want a delay of 2048 samples,.. problem!

    int alloc = x->x_alloc;
    int cursz = x->x_sz; //current size

    if(newsz < 0){
        newsz = 0;
    }
    else if(newsz > COMB_MAX){
        newsz = COMB_MAX;
    };
    if(!alloc && newsz > COMB_STACK){
        x->x_xbuf = (double *)malloc(sizeof(double)*newsz);
        x->x_ybuf = (double *)malloc(sizeof(double)*newsz);
        x->x_alloc = 1;
        x->x_sz = newsz;
    }
    else if(alloc && newsz > cursz){
        x->x_xbuf = (double *)realloc(x->x_xbuf, sizeof(double)*newsz);
        x->x_ybuf = (double *)realloc(x->x_ybuf, sizeof(double)*newsz);
        x->x_sz = newsz;
    }
    else if(alloc && newsz < COMB_STACK){
        free(x->x_xbuf);
        free(x->x_ybuf);
        x->x_xbuf = x->x_ffstack;
        x->x_ybuf = x->x_fbstack;
        x->x_alloc = 0;
    };
    comb_clear(x);
}




static double comb_getlin(double tab[], int sz, double idx){
    //copying from my own library, linear interpolated reader - DK
        double output;
        int tabphase1 = (int)idx;
        int tabphase2 = tabphase1 + 1;
        double frac = idx - (double)tabphase1;
        if(tabphase1 >= (sz - 1)){
                tabphase1 = sz - 1; //checking to see if index falls within bounds
                output = tab[tabphase1];
        }
        else if(tabphase1 < 0){
                tabphase1 = 0;
                output = tab[tabphase1];
            }
        else{
            double yb = tab[tabphase2]; //linear interp
            double ya = tab[tabphase1];
            output = ya+((yb-ya)*frac);
        
        };
        return output;
}

static double comb_readmsdelay(t_comb *x, double arr[], t_float ms){
    //helper func, basically take desired ms delay, convert to samp, read from arr[]

    //eventual reading head
    double rh = (double)ms*((double)x->x_sr*0.001); //converting ms to samples
        rh = (double)x->x_wh+((double)x->x_sz-rh); //essentially subracting from writehead to find proper position in buffer
        //wrapping into length of delay buffer
        while(rh >= x->x_sz){
            rh -= (double)x->x_sz;
        };
        //now to read from the buffer!
        double output = comb_getlin(arr, x->x_sz, rh);
        return output;
        
}



static t_int *comb_perform(t_int *w)
{
    t_comb *x = (t_comb *)(w[1]);
    int n = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float *din = (t_float *)(w[4]);
    t_float *ain = (t_float *)(w[5]);
    t_float *bin = (t_float *)(w[6]);
    t_float *cin = (t_float *)(w[7]);
    t_float *out = (t_float *)(w[8]);
    
    int i;
    for(i=0; i<n;i++){
        int wh = x->x_wh;
        double input = (double)xin[i];
        //first off, write input to delay buf
        x->x_xbuf[wh] = input;
        //get delayed values of x and y
        t_float delms = din[i];
        //first bounds checking
        if(delms < 0){
            delms = 0;
        }
        else if(delms > x->x_maxdel){
            delms = x->x_maxdel;
        };
        //now get those delayed vals
        double delx = comb_readmsdelay(x, x->x_xbuf, delms);
        double dely = comb_readmsdelay(x, x->x_ybuf, delms);
        //figure out your current y term, y[n] = a*x[n]+b*x[n-d] + c*x[n-d]
        double output = (double)ain[i]*input + (double)bin[i]*delx + (double)cin[i]*dely;
        //stick this guy in the ybuffer and output
        x->x_ybuf[wh] = output;
        out[i] = output;

        //increment writehead
        x->x_wh = (wh + 1) % x->x_sz;
    };
    
    return (w + 9);
}

static void comb_dsp(t_comb *x, t_signal **sp)
{
    int sr = sp[0]->s_sr;
    if(sr != x->x_sr){
        //if new sample rate isn't old sample rate, need to realloc
        x->x_sr = sr;
        comb_sz(x);
    };
    dsp_add(comb_perform, 8, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
	    sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec);
}

static void *comb_new(t_floatarg maxdel, t_floatarg initdel,
		      t_floatarg gain, t_floatarg ffcoeff, t_floatarg fbcoeff)
{
    t_comb *x = (t_comb *)pd_new(comb_class);
    
    x->x_maxdel = maxdel > 0 ? maxdel : COMB_DELAY; 
    x->x_sr = sys_getsr();
   
    //defaults
    x->x_alloc = 0;
    x->x_sz = COMB_STACK;
    //clear out stack bufs, set pointer to stack
    x->x_ybuf = x->x_fbstack;
    x->x_xbuf = x->x_ffstack;
    comb_clear(x);

    //ship off to the helper method to deal with allocation if necessary
    comb_sz(x);
    //boundschecking
    if(initdel < 0){
        initdel = 0;
    }
    else if(initdel > x->x_maxdel){
        initdel = x->x_maxdel;
    };

    //inlets outlets
    x->x_dellet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_dellet, initdel);
    x->x_alet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_alet, gain);
    x->x_blet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_blet, ffcoeff);
    x->x_clet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_clet, fbcoeff);
    x->x_outlet = outlet_new((t_object *)x, &s_signal);
    return (x);
}

static void * comb_free(t_comb *x){
    if(x->x_alloc){
        free(x->x_xbuf);
        free(x->x_ybuf);
    };
    inlet_free(x->x_dellet);
    inlet_free(x->x_alet);
    inlet_free(x->x_blet);
    inlet_free(x->x_clet);
    outlet_free(x->x_outlet);
    return (void *)x;
}

void comb_tilde_setup(void)
{
    comb_class = class_new(gensym("comb~"),
			   (t_newmethod)comb_new,
			   (t_method)comb_free,
			   sizeof(t_comb), 0,
			   A_DEFFLOAT, A_DEFFLOAT,
			   A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(comb_class, nullfn, gensym("signal"), 0);
    class_addmethod(comb_class, (t_method)comb_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(comb_class, (t_method)comb_clear, gensym("clear"), 0);
}
