
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "sh.h"

RCSID("$tcsh: tc.printf.c,v 3.35 2006/03/02 18:46:45 christos Exp $")

#ifdef lint
#undef va_arg
#define va_arg(a, b) (a ? (b) 0 : (b) 0)
#endif

#define INF	INT_MAX		/* should be bigger than any field to print */

static char snil[] = "(nil)";

static	void	xaddchar	(int);
static	void	doprnt		(void (*) (int), const char *, va_list);

static void
doprnt(void (*addchar) (int), const char *sfmt, va_list ap)
{
    char *bp;
    const char *f;
#ifdef SHORT_STRINGS
    const Char *Bp;
#endif /* SHORT_STRINGS */
#ifdef HAVE_LONG_LONG
    long long l;
    unsigned long long u;
#else
    long l;
    unsigned long u;
#endif
    char buf[(CHAR_BIT * sizeof (l) + 2) / 3 + 1]; /* Octal: 3 bits per char */
    int i;
    int fmt;
    unsigned char pad = ' ';
    int     flush_left = 0, f_width = 0, prec = INF, hash = 0;
    int	    do_long = 0, do_size_t = 0;
    int     sign = 0;
    int     attributes = 0;


    f = sfmt;
    for (; *f; f++) {
	if (*f != '%') {	/* then just out the char */
	    (*addchar) (((unsigned char)*f) | attributes);
	}
	else {
	    f++;		/* skip the % */

	    if (*f == '-') {	/* minus: flush left */
		flush_left = 1;
		f++;
	    }

	    if (*f == '0' || *f == '.') {
		/* padding with 0 rather than blank */
		pad = '0';
		f++;
	    }
	    if (*f == '*') {	/* field width */
		f_width = va_arg(ap, int);
		f++;
	    }
	    else if (isdigit((unsigned char) *f)) {
		f_width = atoi(f);
		while (isdigit((unsigned char) *f))
		    f++;	/* skip the digits */
	    }

	    if (*f == '.') {	/* precision */
		f++;
		if (*f == '*') {
		    prec = va_arg(ap, int);
		    f++;
		}
		else if (isdigit((unsigned char) *f)) {
		    prec = atoi(f);
		    while (isdigit((unsigned char) *f))
			f++;	/* skip the digits */
		}
	    }

	    if (*f == '#') {	/* alternate form */
		hash = 1;
		f++;
	    }

	    if (*f == 'l') {	/* long format */
		do_long++;
		f++;
		if (*f == 'l') {
		    do_long++;
		    f++;
		}
	    }
	    if (*f == 'z') {	/* size_t format */
		do_size_t++;
		f++;
	    }

	    fmt = (unsigned char) *f;
	    if (fmt != 'S' && fmt != 'Q' && isupper(fmt)) {
		do_long = 1;
		fmt = tolower(fmt);
	    }
	    bp = buf;
	    switch (fmt) {	/* do the format */
	    case 'd':
		switch (do_long) {
		case 0:
		    if (do_size_t)
			l = (long) (va_arg(ap, size_t));
		    else
			l = (long) (va_arg(ap, int));
		    break;
		case 1:
#ifndef HAVE_LONG_LONG
		default:
#endif
		    l = va_arg(ap, long);
		    break;
#ifdef HAVE_LONG_LONG
		default:
		    l = va_arg(ap, long long);
		    break;
#endif
		}

		if (l < 0) {
		    sign = 1;
		    l = -l;
		}
		do {
		    *bp++ = (char) (l % 10) + '0';
		} while ((l /= 10) > 0);
		if (sign)
		    *bp++ = '-';
		f_width = f_width - (int) (bp - buf);
		if (!flush_left)
		    while (f_width-- > 0) 
			(*addchar) (pad | attributes);
		for (bp--; bp >= buf; bp--) 
		    (*addchar) (((unsigned char) *bp) | attributes);
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) (' ' | attributes);
		break;

	    case 'p':
		do_long = 1;
		hash = 1;
		fmt = 'x';
		/*FALLTHROUGH*/
	    case 'o':
	    case 'x':
	    case 'u':
		switch (do_long) {
		case 0:
		    if (do_size_t)
			u = va_arg(ap, size_t);
		    else
			u = va_arg(ap, unsigned int);
		    break;
		case 1:
#ifndef HAVE_LONG_LONG
		default:
#endif
		    u = va_arg(ap, unsigned long);
		    break;
#ifdef HAVE_LONG_LONG
		default:
		    u = va_arg(ap, unsigned long long);
		    break;
#endif
		}
		if (fmt == 'u') {	/* unsigned decimal */
		    do {
			*bp++ = (char) (u % 10) + '0';
		    } while ((u /= 10) > 0);
		}
		else if (fmt == 'o') {	/* octal */
		    do {
			*bp++ = (char) (u % 8) + '0';
		    } while ((u /= 8) > 0);
		    if (hash)
			*bp++ = '0';
		}
		else if (fmt == 'x') {	/* hex */
		    do {
			i = (int) (u % 16);
			if (i < 10)
			    *bp++ = i + '0';
			else
			    *bp++ = i - 10 + 'a';
		    } while ((u /= 16) > 0);
		    if (hash) {
			*bp++ = 'x';
			*bp++ = '0';
		    }
		}
		i = f_width - (int) (bp - buf);
		if (!flush_left)
		    while (i-- > 0)
			(*addchar) (pad | attributes);
		for (bp--; bp >= buf; bp--)
		    (*addchar) (((unsigned char) *bp) | attributes);
		if (flush_left)
		    while (i-- > 0)
			(*addchar) (' ' | attributes);
		break;


	    case 'c':
		i = va_arg(ap, int);
		(*addchar) (i | attributes);
		break;

	    case 'S':
	    case 'Q':
#ifdef SHORT_STRINGS
		Bp = va_arg(ap, Char *);
		if (!Bp) {
		    bp = NULL;
		    goto lcase_s;
	        }
		f_width = f_width - Strlen(Bp);
		if (!flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (pad | attributes));
		for (i = 0; *Bp && i < prec; i++) {
		    char cbuf[MB_LEN_MAX];
		    size_t pos, len;

		    if (fmt == 'Q' && *Bp & QUOTE)
			(*addchar) ('\\' | attributes);
		    len = one_wctomb(cbuf, *Bp & CHAR);
		    for (pos = 0; pos < len; pos++)
			(*addchar) ((unsigned char)cbuf[pos] | attributes
				    | (*Bp & ATTRIBUTES));
		    Bp++;
		}
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) (' ' | attributes);
		break;
#endif /* SHORT_STRINGS */

	    case 's':
	    case 'q':
		bp = va_arg(ap, char *);
lcase_s:
		if (!bp)
		    bp = snil;
		f_width = f_width - strlen(bp);
		if (!flush_left)
		    while (f_width-- > 0)
			(*addchar) (pad | attributes);
		for (i = 0; *bp && i < prec; i++) {
		    if (fmt == 'q' && *bp & QUOTE)
			(*addchar) ('\\' | attributes);
		    (*addchar) (((unsigned char) *bp & TRIM) | attributes);
		    bp++;
		}
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) (' ' | attributes);
		break;

	    case 'a':
		attributes = va_arg(ap, int);
		break;

	    case '%':
		(*addchar) ('%' | attributes);
		break;

	    default:
		break;
	    }
	    flush_left = 0, f_width = 0, prec = INF, hash = 0;
	    do_size_t = 0, do_long = 0;
	    sign = 0;
	    pad = ' ';
	}
    }
}


static char *xstring, *xestring;
static void
xaddchar(int c)
{
    if (xestring == xstring)
	*xstring = '\0';
    else
	*xstring++ = (char) c;
}


pret_t
/*VARARGS*/
xsnprintf(char *str, size_t size, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    xstring = str;
    xestring = str + size - 1;
    doprnt(xaddchar, fmt, va);
    va_end(va);
    *xstring++ = '\0';
#ifdef PURIFY
    return 1;
#endif
}

pret_t
/*VARARGS*/
xprintf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    doprnt(xputchar, fmt, va);
    va_end(va);
#ifdef PURIFY
    return 1;
#endif
}


pret_t
xvprintf(const char *fmt, va_list va)
{
    doprnt(xputchar, fmt, va);
#ifdef PURIFY
    return 1;
#endif
}

pret_t
xvsnprintf(char *str, size_t size, const char *fmt, va_list va)
{
    xstring = str;
    xestring = str + size - 1;
    doprnt(xaddchar, fmt, va);
    *xstring++ = '\0';
#ifdef PURIFY
    return 1;
#endif
}

char *
xvasprintf(const char *fmt, va_list va)
{
    size_t size;
    char *buf;

    buf = NULL;
    size = 2048; /* Arbitrary */
    for (;;) {
	va_list copy;

	buf = xrealloc(buf, size);
	xstring = buf;
	xestring = buf + size - 1;
	va_copy(copy, va);
	doprnt(xaddchar, fmt, copy);
	va_end(copy);
	if (xstring < xestring)
	    break;
	size *= 2;
    }
    *xstring++ = '\0';
    return xrealloc(buf, xstring - buf);
}

char *
xasprintf(const char *fmt, ...)
{
    va_list va;
    char *ret;

    va_start (va, fmt);
    ret = xvasprintf(fmt, va);
    va_end(va);
    return ret;
}


#ifdef PURIFY
/* Purify uses (some of..) the following functions to output memory-use
 * debugging info.  Given all the messing with file descriptors that
 * tcsh does, the easiest way I could think of to get it (Purify) to
 * print anything was by replacing some standard functions with
 * ones that do tcsh output directly - see dumb hook in doreaddirs()
 * (sh.dir.c) -sg
 */
#ifndef FILE
#define FILE int
#endif
int 
fprintf(FILE *fp, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    doprnt(xputchar, fmt, va);
    va_end(va);
    return 1;
}

int 
vfprintf(FILE *fp, const char *fmt, va_list va)
{
    doprnt(xputchar, fmt, va);
    return 1;
}

#endif	/* PURIFY */