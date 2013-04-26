
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
#include <stdlib.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "mblocal.h"
#include "xlocale_private.h"

/*
 * MT-safe version.
 */
wint_t
fgetwc_l(FILE *fp, locale_t locale)
{
	wint_t r;
	FIX_LOCALE(locale);

	FLOCKFILE(fp);
	ORIENT(fp, 1);
	r = __fgetwc(fp, locale);
	FUNLOCKFILE(fp);

	return (r);
}

wint_t
fgetwc(FILE *fp)
{
	return fgetwc_l(fp, __get_locale());
}

/*
 * Internal (non-MPSAFE) version of fgetwc().  This version takes an
 * mbstate_t argument specifying the initial conversion state.  For
 * wide streams, this should always be fp->_mbstate.  On return, *nread
 * is set to the number of bytes read.
 */
wint_t 
__fgetwc_mbs(FILE *fp, mbstate_t *mbs, int *nread, locale_t locale)
{
	wchar_t wc;
	size_t nconv;
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	if (fp->_r <= 0 && __srefill(fp)) {
		*nread = 0;
		return (WEOF);
	}
	if (MB_CUR_MAX == 1) {
		/* Fast path for single-byte encodings. */
		wc = *fp->_p++;
		fp->_r--;
		*nread = 1;
		return (wc);
	}
	*nread = 0;
	do {
		nconv = l->__mbrtowc(&wc, fp->_p, fp->_r, mbs);
		if (nconv == (size_t)-1)
			break;
		else if (nconv == (size_t)-2)
			continue;
		else if (nconv == 0) {
			fp->_p++;
			fp->_r--;
			(*nread)++;
			return (L'\0');
		} else {
			fp->_p += nconv;
			fp->_r -= nconv;
			*nread += nconv;
			return (wc);
		}
	} while (__srefill(fp) == 0);
	fp->_flags |= __SERR;
	errno = EILSEQ;
	return (WEOF);
}