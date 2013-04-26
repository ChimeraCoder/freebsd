
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)util.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <fts.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "ls.h"
#include "extern.h"

int
prn_normal(const char *s)
{
	mbstate_t mbs;
	wchar_t wc;
	int i, n;
	size_t clen;

	memset(&mbs, 0, sizeof(mbs));
	n = 0;
	while ((clen = mbrtowc(&wc, s, MB_LEN_MAX, &mbs)) != 0) {
		if (clen == (size_t)-2) {
			n += printf("%s", s);
			break;
		}
		if (clen == (size_t)-1) {
			memset(&mbs, 0, sizeof(mbs));
			putchar((unsigned char)*s);
			s++;
			n++;
			continue;
		}
		for (i = 0; i < (int)clen; i++)
			putchar((unsigned char)s[i]);
		s += clen;
		if (iswprint(wc))
			n += wcwidth(wc);
	}
	return (n);
}

int
prn_printable(const char *s)
{
	mbstate_t mbs;
	wchar_t wc;
	int i, n;
	size_t clen;

	memset(&mbs, 0, sizeof(mbs));
	n = 0;
	while ((clen = mbrtowc(&wc, s, MB_LEN_MAX, &mbs)) != 0) {
		if (clen == (size_t)-1) {
			putchar('?');
			s++;
			n++;
			memset(&mbs, 0, sizeof(mbs));
			continue;
		}
		if (clen == (size_t)-2) {
			putchar('?');
			n++;
			break;
		}
		if (!iswprint(wc)) {
			putchar('?');
			s += clen;
			n++;
			continue;
		}
		for (i = 0; i < (int)clen; i++)
			putchar((unsigned char)s[i]);
		s += clen;
		n += wcwidth(wc);
	}
	return (n);
}

/*
 * The fts system makes it difficult to replace fts_name with a different-
 * sized string, so we just calculate the real length here and do the
 * conversion in prn_octal()
 *
 * XXX when using f_octal_escape (-b) rather than f_octal (-B), the
 * length computed by len_octal may be too big. I just can't be buggered
 * to fix this as an efficient fix would involve a lookup table. Same goes
 * for the rather inelegant code in prn_octal.
 *
 *						DES 1998/04/23
 */

size_t
len_octal(const char *s, int len)
{
	mbstate_t mbs;
	wchar_t wc;
	size_t clen, r;

	memset(&mbs, 0, sizeof(mbs));
	r = 0;
	while (len != 0 && (clen = mbrtowc(&wc, s, len, &mbs)) != 0) {
		if (clen == (size_t)-1) {
			r += 4;
			s++;
			len--;
			memset(&mbs, 0, sizeof(mbs));
			continue;
		}
		if (clen == (size_t)-2) {
			r += 4 * len;
			break;
		}
		if (iswprint(wc))
			r++;
		else
			r += 4 * clen;
		s += clen;
	}
	return (r);
}

int
prn_octal(const char *s)
{
	static const char esc[] = "\\\\\"\"\aa\bb\ff\nn\rr\tt\vv";
	const char *p;
	mbstate_t mbs;
	wchar_t wc;
	size_t clen;
	unsigned char ch;
	int goodchar, i, len, prtlen;

	memset(&mbs, 0, sizeof(mbs));
	len = 0;
	while ((clen = mbrtowc(&wc, s, MB_LEN_MAX, &mbs)) != 0) {
		goodchar = clen != (size_t)-1 && clen != (size_t)-2;
		if (goodchar && iswprint(wc) && wc != L'\"' && wc != L'\\') {
			for (i = 0; i < (int)clen; i++)
				putchar((unsigned char)s[i]);
			len += wcwidth(wc);
		} else if (goodchar && f_octal_escape &&
#if WCHAR_MIN < 0
                    wc >= 0 &&
#endif
		    wc <= (wchar_t)UCHAR_MAX &&
		    (p = strchr(esc, (char)wc)) != NULL) {
			putchar('\\');
			putchar(p[1]);
			len += 2;
		} else {
			if (goodchar)
				prtlen = clen;
			else if (clen == (size_t)-1)
				prtlen = 1;
			else
				prtlen = strlen(s);
			for (i = 0; i < prtlen; i++) {
				ch = (unsigned char)s[i];
				putchar('\\');
				putchar('0' + (ch >> 6));
				putchar('0' + ((ch >> 3) & 7));
				putchar('0' + (ch & 7));
				len += 4;
			}
		}
		if (clen == (size_t)-2)
			break;
		if (clen == (size_t)-1) {
			memset(&mbs, 0, sizeof(mbs));
			s++;
		} else
			s += clen;
	}
	return (len);
}

void
usage(void)
{
	(void)fprintf(stderr,
#ifdef COLORLS
	"usage: ls [-ABCFGHILPRSTUWZabcdfghiklmnopqrstuwxy1,] [-D format]"
#else
	"usage: ls [-ABCFHILPRSTUWZabcdfghiklmnopqrstuwxy1,] [-D format]"
#endif
		      " [file ...]\n");
	exit(1);
}