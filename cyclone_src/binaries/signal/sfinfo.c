// recycloned from iemlib/soundfile_info as a starting point

#include "m_pd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define SFINFO_HEADER_CHUNK_SIZE_ESTIMATION 10000


static t_class *sfinfo_class;

typedef struct _sfinfo
{
  t_object  x_obj;
  long      *x_begmem;
  int       x_mem_size;
  t_canvas  *x_canvas;
    
    t_float  x_ch;
    t_float  x_sr;
    t_float  x_ms;
    t_float  x_depth;
    t_symbol *x_filename;
    t_symbol *x_sampletype;
    t_float  x_samptype; // temporary
    
    t_outlet *x_ch_outlet;
    t_outlet *x_bit_depth_outlet;
    t_outlet *x_sr_outlet;
    t_outlet *x_dur_outlet;
    t_outlet *x_sampletype_outlet;
    t_outlet *x_filename_outlet;
} t_sfinfo;


static void sfinfo_bang(t_sfinfo *x)
{
//    sfinfo_open(x->x_filename);
}


static short sfinfo_string_to_int16(char *cvec)
{
  short ss=0;
  unsigned char *uc=(unsigned char *)cvec;
  
  ss += (short)(*uc);
  ss += (short)(*(uc+1)*256);
  return(ss);
}

static unsigned long sfinfo_string_to_uint32(char *cvec)
{
  unsigned long ul=0;
  unsigned char *uc=(unsigned char *)cvec;
  
  ul += (unsigned long)(*uc);
  ul += (unsigned long)(*(uc+1)*256);
  ul += (unsigned long)(*(uc+2)*65536);
  ul += (unsigned long)(*(uc+3)*16777216);
  return(ul);
}

static void sfinfo_open(t_sfinfo *x, t_symbol *filename)
{
    
  x->x_filename = filename;
    
  char completefilename[400];
  int i, n, n2, n4, filesize, read_chars, header_size=0, bytesperframe, sr, n_frames;
  FILE *fh;
//  t_atom *at;
  char *cvec;
  unsigned long ul_chunk_size, ul_sr;
  short ss_format, ss_ch, ss_bytesperframe;
  
  if(filename->s_name[0] == '/')/*make complete path + filename*/
  {
    strcpy(completefilename, filename->s_name);
  }
  else if(((filename->s_name[0] >= 'A')&&(filename->s_name[0] <= 'Z')||
    (filename->s_name[0] >= 'a')&&(filename->s_name[0] <= 'z'))&&
    (filename->s_name[1] == ':')&&(filename->s_name[2] == '/'))
  {
    strcpy(completefilename, filename->s_name);
  }
  else
  {
    strcpy(completefilename, canvas_getdir(x->x_canvas)->s_name);
    strcat(completefilename, "/");
    strcat(completefilename, filename->s_name);
  }

  
  fh = fopen(completefilename,"rb");
  if(!fh)
  {
    post("sfinfo_read: cannot open %s !!\n", completefilename);
  }
  else
  {
    n = x->x_mem_size; // 10000 bytes
    n2 = sizeof(short) * x->x_mem_size;
    n4 = sizeof(long) * x->x_mem_size;
    fseek(fh, 0, SEEK_END);
    filesize = ftell(fh);
    fseek(fh,0,SEEK_SET);
    read_chars = (int)fread(x->x_begmem, sizeof(char), n4, fh) / 2;
    fclose(fh);
    //    post("read chars = %d", read_chars);
    cvec = (char *)x->x_begmem;
    if(read_chars > 4)
    {
      if(strncmp(cvec, "RIFF", 4))
      {
        post("sfinfo_read-error:  %s is no RIFF-WAVE-file", completefilename);
        goto sfinfo_end;
      }
      header_size += 8; // jump over RIFF chunk size
      cvec += 8;
      if(strncmp(cvec, "WAVE", 4))
      {
        post("sfinfo_read-error:  %s is no RIFF-WAVE-file", completefilename);
        goto sfinfo_end;
      }
      header_size += 4;
      cvec += 4;
      
      for(i=header_size/2; i<read_chars; i++)
      {
        if(!strncmp(cvec, "fmt ", 4))
        {
          header_size += 4;
          cvec += 4;
          goto sfinfo_fmt;
        }
        header_size += 2;
        cvec += 2;
      }
      post("sfinfo_read-error:  %s has at begin no format-chunk", completefilename);
      goto sfinfo_end;
      
sfinfo_fmt:
      ul_chunk_size = sfinfo_string_to_uint32(cvec);
      if(ul_chunk_size < 16)
      {
        post("sfinfo_read-error:  %s has a format-chunk less than 16", completefilename);
        goto sfinfo_end;
      }
      header_size += 4;
      cvec += 4;

      ss_format = sfinfo_string_to_int16(cvec);
      if((ss_format != 1) && (ss_format != 3) && (ss_format != 6) && (ss_format != 7) && (ss_format != -2))
 
      {
        post("sfinfo_read-error:  %s has unknown format code", completefilename);
        goto sfinfo_end;
      }
      header_size += 2;
      cvec += 2;
        
      x->x_samptype = (t_float)ss_format;
        /*
         PCM = 1 ;
         IEEE-FLOAT = 3 ;
         ALAW = 6 ;
         MULAW = 7 ;
         WAVE_EX = -2
         */
        

      ss_ch = sfinfo_string_to_int16(cvec); /* channels */
      if((ss_ch < 1) || (ss_ch > 32000))
      {
        post("sfinfo_read-error:  %s has no common channel-number", completefilename);
        goto sfinfo_end;
      }
      x->x_ch = (t_float)ss_ch;
      header_size += 2;
      cvec += 2;

      ul_sr = sfinfo_string_to_uint32(cvec); /* samplerate */
      if((ul_sr > 2000000000) || (ul_sr < 1))
      {
        post("sfinfo_read-error:  %s has no common samplerate", completefilename);
        goto sfinfo_end;
      }
        
        x->x_sr = (t_float)ul_sr;

      sr = (int)ul_sr;
      header_size += 4;
      cvec += 4;
      
      header_size += 4; /* jump over bytes_per_sec */
      cvec += 4;

      ss_bytesperframe = sfinfo_string_to_int16(cvec); /* bytes_per_frame */
      if((ss_bytesperframe < 1) || (ss_bytesperframe > 32000))
      {
        post("sfinfo_read-error:  %s has no common number of bytes per frame", completefilename);
        goto sfinfo_end;
      }

      bytesperframe = (int)ss_bytesperframe;
      header_size += 2;
      cvec += 2;
      
      x->x_depth = (t_float)(ss_bytesperframe / ss_ch) * 8;
        
      header_size += 2; /* jump over bits_per_sample */
      cvec += 2;
      
      for(i=header_size/2; i<read_chars; i++) // looking for data chunk
      {
        if(!strncmp(cvec, "data", 4))
          goto sfinfo_data;
        header_size += 2;
        cvec += 2;
      }
      post("sfinfo_read-error:  %s has at begin no data-chunk", completefilename);
      goto sfinfo_end;
      
sfinfo_data:
      header_size += 8; // ignore data chunk size
      cvec += 8;
      
      
      n_frames = (filesize - header_size) / bytesperframe;

        
        x->x_ms = ((t_float)n_frames / x->x_sr) * 1000;
        
      /*      post("ch = %d", ch);
      post("sr = %d", sr);
      post("bpf = %d", bytesperframe/ch);
      post("head = %d", header_size);
      post("len = %d", n_frames);*/
        
        outlet_symbol(x->x_filename_outlet, x->x_filename);
        //    outlet_symbol(x->x_sampletype_outlet, x->x_filename);
        outlet_float(x->x_sampletype_outlet, x->x_samptype);
        outlet_float(x->x_dur_outlet, x->x_ms);
        outlet_float(x->x_sr_outlet, x->x_sr);
        outlet_float(x->x_bit_depth_outlet, x->x_depth);
        outlet_float(x->x_ch_outlet, x->x_ch);
      
      
sfinfo_end:
      
      ;
    }
  }
}

static void sfinfo_free(t_sfinfo *x)
{
    outlet_free(x->x_ch_outlet);
    outlet_free(x->x_bit_depth_outlet);
    outlet_free(x->x_sr_outlet);
    outlet_free(x->x_dur_outlet);
    outlet_free(x->x_sampletype_outlet);
    outlet_free(x->x_filename_outlet);
  freebytes(x->x_begmem, x->x_mem_size * sizeof(long));
}

static void *sfinfo_new(t_symbol *s)
{
  t_sfinfo *x = (t_sfinfo *)pd_new(sfinfo_class);
  
  x->x_mem_size = SFINFO_HEADER_CHUNK_SIZE_ESTIMATION;
    // try to read the first 10000 bytes of the soundfile
  x->x_begmem = (long *)getbytes(x->x_mem_size * sizeof(long));
    
    x->x_filename = s;
    
    x->x_ch_outlet = outlet_new((t_object *)x, &s_float);
    x->x_bit_depth_outlet = outlet_new((t_object *)x, &s_float);
    x->x_sr_outlet = outlet_new((t_object *)x, &s_float);
    x->x_dur_outlet = outlet_new((t_object *)x, &s_float);
    x->x_sampletype_outlet = outlet_new((t_object *)x, &s_float);
    x->x_filename_outlet = outlet_new((t_object *)x, &s_symbol);
    
  x->x_canvas = canvas_getcurrent();
  return (x);
}

/* ---------------- global setup function -------------------- */

void sfinfo_tilde_setup(void)
{
  sfinfo_class = class_new(gensym("sfinfo~"), (t_newmethod)sfinfo_new,
    (t_method)sfinfo_free, sizeof(t_sfinfo), 0, A_DEFSYMBOL, 0);
  class_addmethod(sfinfo_class, (t_method)sfinfo_open, gensym("open"), A_SYMBOL, 0);
  class_addbang(sfinfo_class, sfinfo_bang);
}
