/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifdef NT
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "fi.h"

FILE *firead_open(char *filename, t_canvas *cv, int textmode)
{
    int fd;
    char path[MAXPDSTRING+2], *nameptr;
    t_symbol *dirsym = (cv ? canvas_getdir(cv) : 0);
    /* path arg is returned unbashed (system-independent) */
    if ((fd = open_via_path((dirsym ? dirsym->s_name : ""), filename,
			    "", path, &nameptr, MAXPDSTRING, 1)) < 0)
    	return (0);
    /* Closing/reopening dance.  This is unnecessary under linux, and we
       could have tried to convert fd to fp, but under windows open_via_path()
       returns what seems to be an invalid fd.
       LATER try to understand what is going on here... */
    close(fd);
    if (path != nameptr)
    {
	char *slashpos = path + strlen(path);
	*slashpos++ = '/';
	/* try not to be dependent on current open_via_path() implementation */
	if (nameptr != slashpos)
	    strcpy(slashpos, nameptr);
    }
    sys_bashfilename(path, path);
    return (fopen(path, (textmode ? "r" : "rb")));
}

FILE *fiwrite_open(char *filename, t_canvas *cv, int textmode)
{
    char path[MAXPDSTRING+2];
    if (cv)
	/* path arg is returned unbashed (system-independent) */
	canvas_makefilename(cv, filename, path, MAXPDSTRING);
    else
    {
    	strncpy(path, filename, MAXPDSTRING);
    	path[MAXPDSTRING-1] = 0;
    }
    sys_bashfilename(path, path);
    return (fopen(path, (textmode ? "w" : "wb")));
}
