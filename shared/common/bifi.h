/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* generic helpers for binary file reading and writing */

#ifndef __BIFI_H__
#define __BIFI_H__

#define BIFI_ERR_OK          0
#define BIFI_ERR_OPEN       -1
#define BIFI_ERR_READ       -2  /* generic read failure */
#define BIFI_ERR_WRITE      -3  /* generic write failure */
#define BIFI_ERR_BADHEADER  -4  /* header missing or short */

typedef struct _bifi
{
    int     b_selfalloc:1;
    int     b_hdralloc:1;
    char   *b_header;
    size_t  b_headersize;
    FILE   *b_fp;
    char    b_filename[MAXPDSTRING];
    int     b_err;       /* BIFI_ERR code */
    int     b_syserrno;  /* system error code */
} t_bifi;

uint32 bifi_swap4(uint32 n);
uint16 bifi_swap2(uint16 n);

t_bifi *bifi_new(t_bifi *x, char *hdr, size_t hdrsz);
void bifi_free(t_bifi *x);
void bifi_clear(t_bifi *x);

int bifi_read_start(t_bifi *x, const char *filename, const char *dirname);
int bifi_write_start(t_bifi *x, const char *filename, const char *dirname);

void bifi_error_report(t_bifi *x);

#endif
