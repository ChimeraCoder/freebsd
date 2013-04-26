
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "mblocal.h"

wchar_t *
fgetws_l(wchar_t * __restrict ws, int n, FILE * __restrict fp, locale_t locale)
{
	wchar_t *wsp;
	size_t nconv;
	const char *src;
	unsigned char *nl;
	FIX_LOCALE(locale);
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	FLOCKFILE(fp);
	ORIENT(fp, 1);

	if (n <= 0) {
		errno = EINVAL;
		goto error;
	}

	if (fp->_r <= 0 && __srefill(fp))
		/* EOF */
		goto error;
	wsp = ws;
	do {
		src = fp->_p;
		nl = memchr(fp->_p, '\n', fp->_r);
		nconv = l->__mbsnrtowcs(wsp, &src,
		    nl != NULL ? (nl - fp->_p + 1) : fp->_r,
		    n - 1, &fp->_mbstate);
		if (nconv == (size_t)-1)
			/* Conversion error */
			goto error;
		if (src == NULL) {
			/*
			 * We hit a null byte. Increment the character count,
			 * since mbsnrtowcs()'s return value doesn't include
			 * the terminating null, then resume conversion
			 * after the null.
			 */
			nconv++;
			src = memchr(fp->_p, '\0', fp->_r);
			src++;
		}
		fp->_r -= (unsigned char *)src - fp->_p;
		fp->_p = (unsigned char *)src;
		n -= nconv;
		wsp += nconv;
	} while (wsp[-1] != L'\n' && n > 1 && (fp->_r > 0 ||
	    __srefill(fp) == 0));
	if (wsp == ws)
		/* EOF */
		goto error;
	if (!l->__mbsinit(&fp->_mbstate))
		/* Incomplete character */
		goto error;
	*wsp = L'\0';
	FUNLOCKFILE(fp);

	return (ws);

error:
	FUNLOCKFILE(fp);
	return (NULL);
}
wchar_t *
fgetws(wchar_t * __restrict ws, int n, FILE * __restrict fp)
{
	return fgetws_l(ws, n, fp, __get_locale());
}