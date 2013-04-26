
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

#include <namespace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>
#include <wchar.h>
#include "printf.h"

/*
 * Convert a wide character string argument for the %ls format to a multibyte
 * string representation. If not -1, prec specifies the maximum number of
 * bytes to output, and also means that we can't assume that the wide char.
 * string ends is null-terminated.
 */
static char *
__wcsconv(wchar_t *wcsarg, int prec)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	char buf[MB_LEN_MAX];
	wchar_t *p;
	char *convbuf;
	size_t clen, nbytes;

	/* Allocate space for the maximum number of bytes we could output. */
	if (prec < 0) {
		p = wcsarg;
		mbs = initial;
		nbytes = wcsrtombs(NULL, (const wchar_t **)&p, 0, &mbs);
		if (nbytes == (size_t)-1)
			return (NULL);
	} else {
		/*
		 * Optimisation: if the output precision is small enough,
		 * just allocate enough memory for the maximum instead of
		 * scanning the string.
		 */
		if (prec < 128)
			nbytes = prec;
		else {
			nbytes = 0;
			p = wcsarg;
			mbs = initial;
			for (;;) {
				clen = wcrtomb(buf, *p++, &mbs);
				if (clen == 0 || clen == (size_t)-1 ||
				    (int)(nbytes + clen) > prec)
					break;
				nbytes += clen;
			}
		}
	}
	if ((convbuf = malloc(nbytes + 1)) == NULL)
		return (NULL);

	/* Fill the output buffer. */
	p = wcsarg;
	mbs = initial;
	if ((nbytes = wcsrtombs(convbuf, (const wchar_t **)&p,
	    nbytes, &mbs)) == (size_t)-1) {
		free(convbuf);
		return (NULL);
	}
	convbuf[nbytes] = '\0';
	return (convbuf);
}


/* 's' ---------------------------------------------------------------*/

int
__printf_arginfo_str(const struct printf_info *pi, size_t n, int *argt)
{

	assert (n > 0);
	if (pi->is_long || pi->spec == 'C')
		argt[0] = PA_WSTRING;
	else
		argt[0] = PA_STRING;
	return (1);
}

int
__printf_render_str(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	const char *p;
	wchar_t *wcp;
	char *convbuf;
	int l;

	if (pi->is_long || pi->spec == 'S') {
		wcp = *((wint_t **)arg[0]);
		if (wcp == NULL)
			return (__printf_out(io, pi, "(null)", 6));
		convbuf = __wcsconv(wcp, pi->prec);
		if (convbuf == NULL) 
			return (-1);
		l = __printf_out(io, pi, convbuf, strlen(convbuf));
		free(convbuf);
		return (l);
	} 
	p = *((char **)arg[0]);
	if (p == NULL)
		return (__printf_out(io, pi, "(null)", 6));
	l = strlen(p);
	if (pi->prec >= 0 && pi->prec < l)
		l = pi->prec;
	return (__printf_out(io, pi, p, l));
}

/* 'c' ---------------------------------------------------------------*/

int
__printf_arginfo_chr(const struct printf_info *pi, size_t n, int *argt)
{

	assert (n > 0);
	if (pi->is_long || pi->spec == 'C')
		argt[0] = PA_WCHAR;
	else
		argt[0] = PA_INT;
	return (1);
}

int
__printf_render_chr(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	int i;
	wint_t ii;
	unsigned char c;
	static const mbstate_t initial;		/* XXX: this is bogus! */
	mbstate_t mbs;
	size_t mbseqlen;
	char buf[MB_CUR_MAX];

	if (pi->is_long || pi->spec == 'C') {
		ii = *((wint_t *)arg[0]);

		mbs = initial;
		mbseqlen = wcrtomb(buf, (wchar_t)ii, &mbs);
		if (mbseqlen == (size_t) -1)
			return (-1);
		return (__printf_out(io, pi, buf, mbseqlen));
	}
	i = *((int *)arg[0]);
	c = i;
	i = __printf_out(io, pi, &c, 1);
	__printf_flush(io);
	return (i);
}