/* Copyright (c) 1997-2003 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define BINPORT_MAXSTRING  256
#define BINPORT_SYMGROW     64

#ifndef BINPORT_STANDALONE
/* load max binary file to a binbuf */

#include "m_pd.h"

#else
/* make a max-textual listing from a max binary file */

/* This is a standalone version of a ``max binary to binbuf'' module.
   It uses certain Pd calls and structs, which are duplicated below.
   LATER should be linked to the Pd API library. */

#define BINPORT_VERBOSE
//#define BINPORT_DEBUG

#endif

#include "binport.h"

static void binport_error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "ERROR (binport): ");
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
}

static void binport_warning(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "warning (binport): ");
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
}

static void binport_bug(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "BUG (binport): ");
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
}

#ifdef BINPORT_STANDALONE

typedef int t_int;
typedef float t_float;

typedef struct _symbol
{
    char *s_name;
    void *s_thing;
    struct _symbol *s_next;
} t_symbol;

typedef union word
{
    t_float w_float;
    t_symbol *w_symbol;
    int w_index;
} t_word;

typedef enum
{
    A_NULL,
    A_FLOAT,
    A_SYMBOL,
    A_POINTER,
    A_SEMI,
    A_COMMA,
    A_DEFFLOAT,
    A_DEFSYM,
    A_DOLLAR, 
    A_DOLLSYM,
    A_GIMME,
    A_CANT
}  t_atomtype;

typedef struct _atom
{
    t_atomtype a_type;
    union word a_w;
} t_atom;

void *getbytes(size_t nbytes)
{
    void *ret;
    if (nbytes < 1) nbytes = 1;
    ret = (void *)calloc(nbytes, 1);
    if (!ret)
	binport_error("getbytes() failed -- out of memory");
    return (ret);
}

void *resizebytes(void *old, size_t oldsize, size_t newsize)
{
    void *ret;
    if (newsize < 1) newsize = 1;
    if (oldsize < 1) oldsize = 1;
    ret = (void *)realloc((char *)old, newsize);
    if (newsize > oldsize && ret)
    	memset(((char *)ret) + oldsize, 0, newsize - oldsize);
    if (!ret)
    	binport_error("resizebytes() failed -- out of memory");
    return (ret);
}

void freebytes(void *fatso, size_t nbytes)
{
    free(fatso);
}

#define HASHSIZE 1024

static t_symbol *symhash[HASHSIZE];

t_symbol *dogensym(char *s, t_symbol *oldsym)
{
    t_symbol **sym1, *sym2;
    unsigned int hash1 = 0,  hash2 = 0;
    int length = 0;
    char *s2 = s;
    while (*s2)
    {
	hash1 += *s2;
	hash2 += hash1;
	length++;
	s2++;
    }
    sym1 = symhash + (hash2 & (HASHSIZE-1));
    while (sym2 = *sym1)
    {
	if (!strcmp(sym2->s_name, s)) return(sym2);
	sym1 = &sym2->s_next;
    }
    if (oldsym) sym2 = oldsym;
    else
    {
    	sym2 = (t_symbol *)getbytes(sizeof(*sym2));
    	sym2->s_name = getbytes(length+1);
    	sym2->s_next = 0;
    	sym2->s_thing = 0;
    	strcpy(sym2->s_name, s);
    }
    *sym1 = sym2;
    return (sym2);
}

t_symbol *gensym(char *s)
{
    return(dogensym(s, 0));
}

#endif  /* end of Pd API */

/* clumsy... LATER find a better way */
#ifdef BINPORT_STANDALONE
#define A_INT  (A_CANT + 1)
#endif

static int binport_getint(t_atom *ap)
{
#ifdef A_INT
    return (*(int *)&ap->a_w);
#else
    return (ap->a_w.w_float);
#endif
}

static void binport_setint(t_atom *ap, int i)
{
#ifdef A_INT
    ap->a_type = A_INT;
    *(int *)&ap->a_w = i;
#else
    SETFLOAT(ap, (float)i);
#endif
}

static void binport_setfloat(t_atom *ap, float f)
{
    ap->a_type = A_FLOAT;
    ap->a_w.w_float = f;
}

typedef struct _binport
{
    FILE       *b_fp;
    int         b_nsymbols;
    int         b_symsize;
    t_symbol  **b_symtable;
} t_binport;

static int binport_getbuf(t_binport *bp, char *buf, size_t sz)
{
    return (fread(buf, 1, sz, bp->b_fp) == sz);
}

static int binport_getbyte(t_binport *bp, unsigned char *buf)
{
    int c;
    if ((c = fgetc(bp->b_fp)) == EOF)
	return (0);
    *buf = (unsigned char)c;
    return (1);
}

static int binport_getstring(t_binport *bp, char *buf)
{
    int c, i = 0;
    while (c = fgetc(bp->b_fp))
    {
	if (c == EOF)
	    return (0);
	if (++i < BINPORT_MAXSTRING)
	    *buf++ = (unsigned char)c;
    }
    *buf = '\0';
    if (i >= BINPORT_MAXSTRING)
	binport_warning("symbol string too long, skipped");
    return (1);
}

static t_symbol *binport_makesymbol(t_binport *bp, int id)
{
    char s[BINPORT_MAXSTRING];
    if (id < bp->b_nsymbols)
	binport_bug("symbol id mismatch");
    else if (id > bp->b_nsymbols)
	binport_error("unexpected symbol id");
    else if (binport_getstring(bp, s))
    {
	int reqsize = ++bp->b_nsymbols;
	if (reqsize > bp->b_symsize)
	{
	    reqsize += (BINPORT_SYMGROW - 1);
#ifdef BINPORT_DEBUG
	    binport_warning("resizing symbol table to %d elements", reqsize);
#endif
	    if (bp->b_symtable =
		resizebytes(bp->b_symtable,
			    bp->b_symsize * sizeof(*bp->b_symtable),
			    reqsize * sizeof(*bp->b_symtable)))
		bp->b_symsize = reqsize;
	    else
	    {
		bp->b_nsymbols = bp->b_symsize = 0;
		return (0);
	    }
	}
	return (bp->b_symtable[id] = gensym(s));
    }
    return (0);
}

static t_symbol *binport_getsymbol(t_binport *bp, int id)
{
    if (id < bp->b_nsymbols)
	return (bp->b_symtable[id]);
    else
	return (binport_makesymbol(bp, id));
}

static int binport_setsymbol(t_binport *bp, t_atom *ap, int id)
{
    t_symbol *s = binport_getsymbol(bp, id);
    if (s)
    {
	ap->a_type = A_SYMBOL;
	ap->a_w.w_symbol = s;
    }
    return (s != 0);
}

static int binport_nextatom(t_binport *bp, t_atom *ap)
{
    unsigned char opcode;
    int opval;
    char buf[64];
    if (!binport_getbyte(bp, &opcode))
	goto bad;
    opval = opcode & 0x0f;
    switch (opcode >> 4)
    {
    case 1:  /* variable length int,
		opval: length (number of bytes that follow) */
	if (!binport_getbuf(bp, buf, opval))
	    goto bad;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    if (opcode == 0x12)  /* FIXME */
		i = (short)i;
	    binport_setint(ap, i);
	}
	break;
    case 2:  /* variable length float,
		opval: length (number of bytes that follow) */
	if (!binport_getbuf(bp, buf, opval))
	    goto bad;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    binport_setfloat(ap, *(t_float *)&i);
	}
	break;
    case 3:  /* variable length symbol id,
		opval: length (number of bytes that follow) */
	if (!binport_getbuf(bp, buf, opval))
	    goto bad;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    if (!binport_setsymbol(bp, ap, i))
		goto bad;
	}
	break;
    case 5:  /* half-byte int */
	binport_setint(ap, opval);
	break;
    case 7:  /* half-byte symbol id */
	if (!binport_setsymbol(bp, ap, opval))
	    goto bad;
	break;
    case 12:  /* #number */
	sprintf(buf, "#%d", opval);
	ap->a_type = A_SYMBOL;
	ap->a_w.w_symbol = gensym(buf);
	break;
    case 13:  /* #symbol id,
		 opval: length (number of bytes that follow) */
	if (!binport_getbuf(bp, buf, opval))
	    goto bad;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    if (!binport_setsymbol(bp, ap, i))
		goto bad;
	}
	sprintf(buf, "#%s", ap->a_w.w_symbol->s_name);
#ifdef BINPORT_DEBUG
	binport_warning(buf);
#endif
	ap->a_w.w_symbol = gensym(buf);
	break;
    default:
	switch (opcode)
	{
	case 0xa0:
	    ap->a_type = A_SEMI;
	    break;
	default:
	    goto unknown;
	}
    }
    return (1);
unknown:
    binport_error("unknown opcode %x", (int)opcode);
bad:
    return (0);
}

static void binport_free(t_binport *bp)
{
    fclose(bp->b_fp);
    freebytes(bp->b_symtable, bp->b_symsize * sizeof(*bp->b_symtable));
    freebytes(bp, sizeof(*bp));
}

static t_binport *binport_new(FILE *fp, int *ftypep)
{
    static char binport_header[4] = { 2, 0, 0, 0 };
    char header[4];
    *ftypep = BINPORT_INVALID;
    if (fread(header, 1, 4, fp) == 4)
    {
	if (memcmp(header, binport_header, 4))
	{
	    if (memcmp(header, "max", 3))
	    {
		if (header[0] == '#')  /* LATER rethink */
		    *ftypep = BINPORT_PDFILE;
#ifdef BINPORT_VERBOSE
		else binport_warning("unknown header: %x %x %x %x",
				     (int)header[0], (int)header[1],
				     (int)header[2], (int)header[3]);
#endif
	    }
	    else *ftypep = BINPORT_MAXTEXT;
	}
	else
	{
	    t_binport *bp = getbytes(sizeof(*bp));
	    bp->b_fp = fp;
	    bp->b_nsymbols = 0;
	    bp->b_symsize = BINPORT_SYMGROW;
	    bp->b_symtable = getbytes(bp->b_symsize * sizeof(*bp->b_symtable));
	    *ftypep = BINPORT_OK;
	    return (bp);
	}
    }
#ifdef BINPORT_VERBOSE
    else binport_warning("file too short");
#endif
    fclose(fp);
    return (0);
}

#ifndef BINPORT_STANDALONE

/* LATER deal with corrupt binary files? */
int binport_read(t_binbuf *bb, char *filename, char *dirname)
{
    FILE *fp;
    char namebuf[MAXPDSTRING];
    namebuf[0] = 0;
    if (*dirname)
    	strcat(namebuf, dirname), strcat(namebuf, "/");
    strcat(namebuf, filename);
    sys_bashfilename(namebuf, namebuf);
    if (fp = fopen(namebuf, "rb"))
    {
	int ftype;
	t_binport *bp = binport_new(fp, &ftype);
	if (bp)
	{
	    t_atom at;
	    while (binport_nextatom(bp, &at))
		if (at.a_type != A_NULL)
		    binbuf_add(bb, 1, &at);
	    binport_free(bp);
	    return (BINPORT_OK);
	}
	else if (ftype == BINPORT_MAXTEXT || ftype == BINPORT_PDFILE)
	    return (ftype);
	else
	    binport_error("\"%s\" doesn't look like a patch file", filename);
    }
    else binport_bug("cannot open file");
    return (BINPORT_INVALID);
}

#else

static void binport_atomstring(t_atom *ap, char *buf, int bufsize)
{
    char *sp, *bp, *ep;
    switch(ap->a_type)
    {
    case A_SEMI:
	strcpy(buf, ";"); break;
    case A_COMMA:
	strcpy(buf, ","); break;
    case A_INT:
	sprintf(buf, "%d", binport_getint(ap)); break;
    case A_FLOAT:
	sprintf(buf, "%#f", ap->a_w.w_float);
	ep = buf + strlen(buf) - 1;
	while (ep > buf && *ep == '0') *ep-- = 0;
	break;
    case A_SYMBOL:
    	sp = ap->a_w.w_symbol->s_name;
	bp = buf;
	ep = buf + (bufsize-5);
	while (bp < ep && *sp)
	{
	    if (*sp == ';' || *sp == ',' || *sp == '\\' ||
		(*sp == '$' && bp == buf && sp[1] >= '0' && sp[1] <= '9'))
		*bp++ = '\\';
	    if ((unsigned char)*sp < 127)
		*bp++ = *sp++;
	    else
		/* FIXME this is temporary -- codepage horror */
		sprintf(bp, "\\%.3o", (unsigned char)*sp++), bp += 4;
	}
	if (*sp) *bp++ = '*';
	*bp = 0;
	break;
    case A_DOLLAR:
    	sprintf(buf, "$%d", ap->a_w.w_index);
    	break;
    case A_DOLLSYM:
    	sprintf(buf, "$%s", ap->a_w.w_symbol->s_name);
    	break;
    default:
    	binport_bug("bad atom type");
	strcpy(buf, "???");
    }
}

int main(int ac, char **av)
{
    if (ac > 1)
    {
	FILE *fp = fopen(av[1], "rb");
	if (fp)
	{
	    int ftype;
	    t_binport *bp = binport_new(fp, &ftype);
	    if (bp)
	    {
		char buf[BINPORT_MAXSTRING];
		t_atom at;
		int ac = 0;
		while (binport_nextatom(bp, &at))
		{
		    if (at.a_type == A_SEMI)
		    {
			fputs(";\n", stdout);
			ac = 0;
		    }
		    else if (at.a_type != A_NULL)
		    {
			if (ac++) fputc(' ', stdout);
			binport_atomstring(&at, buf, BINPORT_MAXSTRING);
			fputs(buf, stdout);
		    }
		}
		binport_free(bp);
	    }
	    else if (ftype == BINPORT_MAXTEXT)
		binport_warning("\"%s\" looks like a Max text file", av[1]);
	    else if (ftype == BINPORT_PDFILE)
		binport_warning("\"%s\" looks like a Pd patch file", av[1]);
	    else
		binport_error("\"%s\" doesn't look like a patch file", av[1]);
	}
	else binport_error("cannot open file \"%s\"", av[1]);
    }
    else binport_error("what file?");
    return (0);
}

#endif
