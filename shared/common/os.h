/* Copyright (c) 2004-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __OS_H__
#define __OS_H__

int ospath_length(char *path, char *cwd);
char *ospath_absolute(char *path, char *cwd, char *result);
FILE *fileread_open(char *filename, t_canvas *cv, int textmode);
FILE *filewrite_open(char *filename, t_canvas *cv, int textmode);

#endif
