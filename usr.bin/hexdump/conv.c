
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
static const char sccsid[] = "@(#)conv.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include "hexdump.h"

void
conv_c(PR *pr, u_char *p, size_t bufsize)
{
	char buf[10];
	char const *str;
	wchar_t wc;
	size_t clen, oclen;
	int converr, pad, width;
	u_char peekbuf[MB_LEN_MAX];

	if (pr->mbleft > 0) {
		str = "**";
		pr->mbleft--;
		goto strpr;
	}

	switch(*p) {
	case '\0':
		str = "\\0";
		goto strpr;
	/* case '\a': */
	case '\007':
		str = "\\a";
		goto strpr;
	case '\b':
		str = "\\b";
		goto strpr;
	case '\f':
		str = "\\f";
		goto strpr;
	case '\n':
		str = "\\n";
		goto strpr;
	case '\r':
		str = "\\r";
		goto strpr;
	case '\t':
		str = "\\t";
		goto strpr;
	case '\v':
		str = "\\v";
		goto strpr;
	default:
		break;
	}
	/*
	 * Multibyte characters are disabled for hexdump(1) for backwards
	 * compatibility and consistency (none of its other output formats
	 * recognize them correctly).
	 */
	converr = 0;
	if (odmode && MB_CUR_MAX > 1) {
		oclen = 0;
retry:
		clen = mbrtowc(&wc, p, bufsize, &pr->mbstate);
		if (clen == 0)
			clen = 1;
		else if (clen == (size_t)-1 || (clen == (size_t)-2 &&
		    p == peekbuf)) {
			memset(&pr->mbstate, 0, sizeof(pr->mbstate));
			wc = *p;
			clen = 1;
			converr = 1;
		} else if (clen == (size_t)-2) {
			/*
			 * Incomplete character; peek ahead and see if we
			 * can complete it.
			 */
			oclen = bufsize;
			bufsize = peek(p = peekbuf, MB_CUR_MAX);
			goto retry;
		}
		clen += oclen;
	} else {
		wc = *p;
		clen = 1;
	}
	if (!converr && iswprint(wc)) {
		if (!odmode) {
			*pr->cchar = 'c';
			(void)printf(pr->fmt, (int)wc);
		} else {	
			*pr->cchar = 'C';
			assert(strcmp(pr->fmt, "%3C") == 0);
			width = wcwidth(wc);
			assert(width >= 0);
			pad = 3 - width;
			if (pad < 0)
				pad = 0;
			(void)printf("%*s%C", pad, "", wc);
			pr->mbleft = clen - 1;
		}
	} else {
		(void)sprintf(buf, "%03o", (int)*p);
		str = buf;
strpr:		*pr->cchar = 's';
		(void)printf(pr->fmt, str);
	}
}

void
conv_u(PR *pr, u_char *p)
{
	static char const * list[] = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		 "bs",  "ht",  "lf",  "vt",  "ff",  "cr",  "so",  "si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can",  "em", "sub", "esc",  "fs",  "gs",  "rs",  "us",
	};

						/* od used nl, not lf */
	if (*p <= 0x1f) {
		*pr->cchar = 's';
		if (odmode && *p == 0x0a)
			(void)printf(pr->fmt, "nl");
		else
			(void)printf(pr->fmt, list[*p]);
	} else if (*p == 0x7f) {
		*pr->cchar = 's';
		(void)printf(pr->fmt, "del");
	} else if (odmode && *p == 0x20) {	/* od replaced space with sp */
		*pr->cchar = 's';
		(void)printf(pr->fmt, " sp");
	} else if (isprint(*p)) {
		*pr->cchar = 'c';
		(void)printf(pr->fmt, *p);
	} else {
		*pr->cchar = 'x';
		(void)printf(pr->fmt, (int)*p);
	}
}