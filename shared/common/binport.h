/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __BINPORT_H__
#define __BINPORT_H__

enum { BINPORT_OK, BINPORT_MAXTEXT, BINPORT_PDFILE,
       BINPORT_INVALID, BINPORT_CORRUPT };

#ifndef BINPORT_STANDALONE
int binport_read(t_binbuf *bb, char *filename, char *dirname);
#endif

#endif
