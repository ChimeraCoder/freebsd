
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)output.c	8.2 (Berkeley) 5/4/95";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Shell output routines.  We use our own output routines because:
 *	When a builtin command is interrupted we have to discard
 *		any pending output.
 *	When a builtin command appears in back quotes, we want to
 *		save the output of the command in a region obtained
 *		via malloc, rather than doing a fork and reading the
 *		output of the command via a pipe.
 */

#include <stdio.h>	/* defines BUFSIZ */
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "shell.h"
#include "syntax.h"
#include "output.h"
#include "memalloc.h"
#include "error.h"
#include "var.h"


#define OUTBUFSIZ BUFSIZ
#define MEM_OUT -2		/* output to dynamically allocated memory */
#define OUTPUT_ERR 01		/* error occurred on output */

static int doformat_wr(void *, const char *, int);

struct output output = {NULL, 0, NULL, OUTBUFSIZ, 1, 0};
struct output errout = {NULL, 0, NULL, 256, 2, 0};
struct output memout = {NULL, 0, NULL, 0, MEM_OUT, 0};
struct output *out1 = &output;
struct output *out2 = &errout;



#ifdef mkinit

INCLUDE "output.h"
INCLUDE "memalloc.h"

RESET {
	out1 = &output;
	out2 = &errout;
	if (memout.buf != NULL) {
		ckfree(memout.buf);
		memout.buf = NULL;
	}
}

#endif


void
outcslow(int c, struct output *file)
{
	outc(c, file);
}

void
out1str(const char *p)
{
	outstr(p, out1);
}

void
out1qstr(const char *p)
{
	outqstr(p, out1);
}

void
out2str(const char *p)
{
	outstr(p, out2);
}

void
out2qstr(const char *p)
{
	outqstr(p, out2);
}

void
outstr(const char *p, struct output *file)
{
	outbin(p, strlen(p), file);
}

/* Like outstr(), but quote for re-input into the shell. */
void
outqstr(const char *p, struct output *file)
{
	char ch;
	int inquotes;

	if (p[0] == '\0') {
		outstr("''", file);
		return;
	}
	/* Caller will handle '=' if necessary */
	if (p[strcspn(p, "|&;<>()$`\\\"' \t\n*?[~#")] == '\0' ||
			strcmp(p, "[") == 0) {
		outstr(p, file);
		return;
	}

	inquotes = 0;
	while ((ch = *p++) != '\0') {
		switch (ch) {
		case '\'':
			/* Can't quote single quotes inside single quotes. */
			if (inquotes)
				outcslow('\'', file);
			inquotes = 0;
			outstr("\\'", file);
			break;
		default:
			if (!inquotes)
				outcslow('\'', file);
			inquotes = 1;
			outc(ch, file);
		}
	}
	if (inquotes)
		outcslow('\'', file);
}

void
outbin(const void *data, size_t len, struct output *file)
{
	const char *p;

	p = data;
	while (len-- > 0)
		outc(*p++, file);
}

void
emptyoutbuf(struct output *dest)
{
	int offset;

	if (dest->buf == NULL) {
		INTOFF;
		dest->buf = ckmalloc(dest->bufsize);
		dest->nextc = dest->buf;
		dest->nleft = dest->bufsize;
		INTON;
	} else if (dest->fd == MEM_OUT) {
		offset = dest->bufsize;
		INTOFF;
		dest->bufsize <<= 1;
		dest->buf = ckrealloc(dest->buf, dest->bufsize);
		dest->nleft = dest->bufsize - offset;
		dest->nextc = dest->buf + offset;
		INTON;
	} else {
		flushout(dest);
	}
	dest->nleft--;
}


void
flushall(void)
{
	flushout(&output);
	flushout(&errout);
}


void
flushout(struct output *dest)
{

	if (dest->buf == NULL || dest->nextc == dest->buf || dest->fd < 0)
		return;
	if (xwrite(dest->fd, dest->buf, dest->nextc - dest->buf) < 0)
		dest->flags |= OUTPUT_ERR;
	dest->nextc = dest->buf;
	dest->nleft = dest->bufsize;
}


void
freestdout(void)
{
	INTOFF;
	if (output.buf) {
		ckfree(output.buf);
		output.buf = NULL;
		output.nleft = 0;
	}
	INTON;
}


int
outiserror(struct output *file)
{
	return (file->flags & OUTPUT_ERR);
}


void
outclearerror(struct output *file)
{
	file->flags &= ~OUTPUT_ERR;
}


void
outfmt(struct output *file, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	doformat(file, fmt, ap);
	va_end(ap);
}


void
out1fmt(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	doformat(out1, fmt, ap);
	va_end(ap);
}

void
out2fmt_flush(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	doformat(out2, fmt, ap);
	va_end(ap);
	flushout(out2);
}

void
fmtstr(char *outbuf, int length, const char *fmt, ...)
{
	va_list ap;

	INTOFF;
	va_start(ap, fmt);
	vsnprintf(outbuf, length, fmt, ap);
	va_end(ap);
	INTON;
}

static int
doformat_wr(void *cookie, const char *buf, int len)
{
	struct output *o;

	o = (struct output *)cookie;
	outbin(buf, len, o);

	return (len);
}

void
doformat(struct output *dest, const char *f, va_list ap)
{
	FILE *fp;

	if ((fp = fwopen(dest, doformat_wr)) != NULL) {
		vfprintf(fp, f, ap);
		fclose(fp);
	}
}

/*
 * Version of write which resumes after a signal is caught.
 */

int
xwrite(int fd, const char *buf, int nbytes)
{
	int ntry;
	int i;
	int n;

	n = nbytes;
	ntry = 0;
	for (;;) {
		i = write(fd, buf, n);
		if (i > 0) {
			if ((n -= i) <= 0)
				return nbytes;
			buf += i;
			ntry = 0;
		} else if (i == 0) {
			if (++ntry > 10)
				return nbytes - n;
		} else if (errno != EINTR) {
			return -1;
		}
	}
}