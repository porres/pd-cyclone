/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* generic helpers for binary file reading and writing */

#ifdef NT
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "m_pd.h"
#include "shared.h"
#include "common/bifi.h"

#if 1
#define BIFI_VERBOSE
#if 0
#define BIFI_DEBUG
#endif
#endif

static int bifi_swapping = 1;  /* set in bifi_clear() */

/* one helper from g_array.c (the original is global, but since
   garray_ambigendian() lacks EXTERN specifier, .dll externs cannot see it;
   btw. it has a comment: ``this should be renamed and moved...'')
*/
static int ambigendian(void)
{
    unsigned short s = 1;
    unsigned char c = *(char *)(&s);
    return (c==0);
}

/* two helpers from d_soundfile.c */
uint32 bifi_swap4(uint32 n)
{
    if (bifi_swapping)
    	return (((n & 0xff) << 24) | ((n & 0xff00) << 8) |
		((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
    else return (n);
}

uint16 bifi_swap2(uint16 n)
{
    if (bifi_swapping)
    	return (((n & 0xff) << 8) | ((n & 0xff00) >> 8));
    else return (n);
}

static void bifi_error_clear(t_bifi *x)
{
    x->b_err = BIFI_ERR_OK;
    x->b_syserrno = 0;
    errno = 0;
}

static void bifi_error_set(t_bifi *x, int errcode)
{
    x->b_err = errcode;
    x->b_syserrno = errno;
    if (errcode != BIFI_ERR_OK && x->b_fp)
    {
	fclose(x->b_fp);
	x->b_fp = 0;
    }
#if 0  /* LATER use Pd's own error logging mechanism, maybe by calling this: */
    sys_unixerror((char *)x);  /* sys_logerror((char *)x, "...")? */
#endif
}

void bifi_clear(t_bifi *x)
{
    bifi_swapping = !ambigendian();
    x->b_fp = 0;
    x->b_filename[0] = '\0';
    bifi_error_clear(x);
}

t_bifi *bifi_new(t_bifi *x, char *hdr, size_t hdrsz)
{
    t_bifi *result = x;
    if (result) result->b_selfalloc = 0;
    else {
	if (!(result = getbytes(sizeof(*result)))) return (0);
	result->b_selfalloc = 1;
    }
    if (hdr || !hdrsz) result->b_hdralloc = 0;
    else {
	if (!(hdr = getbytes(hdrsz)))
	{
	    if (result->b_selfalloc) freebytes(result, sizeof(*result));
	    return (0);
	}
	result->b_hdralloc = 1;
    }
    result->b_header = hdr;
    result->b_headersize = hdrsz;
    bifi_clear(result);
    return (result);
}

void bifi_free(t_bifi *x)
{
    if (x->b_fp) fclose(x->b_fp);
    if (x->b_hdralloc) freebytes(x->b_header, x->b_headersize);
    if (x->b_selfalloc) freebytes(x, sizeof(*x));
}

void bifi_error_report(t_bifi *x)
{
    char *errmess = 0;
    switch (x->b_err)
    {
    case BIFI_ERR_OK:
	break;
    case BIFI_ERR_OPEN:
	errmess = "cannot open";
	break;
    case BIFI_ERR_READ:
	errmess = "error reading";
	break;
    case BIFI_ERR_WRITE:
	errmess = "error writing";
	break;
    case BIFI_ERR_BADHEADER:
	errmess = "missing header of";
	break;
    default:
	post("binary file i/o unknown error");
    }
    if (errmess)
	post("%s binary file `%s\' (errno %d: %s)", errmess,
	     x->b_filename, x->b_syserrno, strerror(x->b_syserrno));
    bifi_error_clear(x);
}

/* Open file and read in its header (x must be a valid t_bifi pointer,
   no checks are being made...)
*/
int bifi_read_start(t_bifi *x, const char *filename, const char *dirname)
{
    int fd;
    char dirbuf[MAXPDSTRING], *nameptr;

    bifi_clear(x);
    strcpy(x->b_filename, filename);
    if ((fd = open_via_path(dirname, filename,
			    "", dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)
    {
	bifi_error_set(x, BIFI_ERR_OPEN);
    	return (0);
    }

    /* Closing/reopening dance.  This is unnecessary under linux, and we
       could have tried to convert fd to fp (since we prefer using streams),
       but under windows open_via_path() returns what seems to be an invalid
       fd.  LATER try to understand what is going on here... */
    close(fd);
    if (dirbuf != nameptr)
    {
	char *slashpos = dirbuf + strlen(dirbuf);
	*slashpos++ = '/';
	/* try not to be dependent on current open_via_path() implementation */
	if (nameptr != slashpos)
	    strcpy(slashpos, nameptr);
    }
    sys_unbashfilename(dirbuf, dirbuf);
    if (!(x->b_fp = fopen(dirbuf, "rb")))
    {
	bifi_error_set(x, BIFI_ERR_OPEN);
    	return (0);
    }

    if (x->b_headersize &&
	fread(x->b_header, 1, x->b_headersize, x->b_fp) < x->b_headersize)
    {
	bifi_error_set(x, BIFI_ERR_BADHEADER);
	return (0);
    }
    return (1);
}

/* Open file and write the supplied header (x must be a valid t_bifi pointer
   with header data properly filled, no checks are being made...)
*/
int bifi_write_start(t_bifi *x, const char *filename, const char *dirname)
{
    char fnamebuf[MAXPDSTRING];

    bifi_clear(x);
    strcpy(x->b_filename, filename);

    fnamebuf[0] = 0;
    if (*dirname)
    	strcat(fnamebuf, dirname), strcat(fnamebuf, "/");
    strcat(fnamebuf, filename);
    sys_bashfilename(fnamebuf, fnamebuf);
    if (!(x->b_fp = fopen(fnamebuf, "wb")))
    {
	bifi_error_set(x, BIFI_ERR_OPEN);
    	return (0);
    }

    if (x->b_headersize &&
	fwrite(x->b_header, 1, x->b_headersize, x->b_fp) < x->b_headersize)
    {
	bifi_error_set(x, BIFI_ERR_WRITE);
	return (0);
    }
    return (1);
}
