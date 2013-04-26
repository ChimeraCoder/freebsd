
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "mblocal.h"
#include "xlocale_private.h"

/*
 * Non-MT-safe version.
 */
wint_t
__ungetwc(wint_t wc, FILE *fp, locale_t locale)
{
	char buf[MB_LEN_MAX];
	size_t len;
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	if (wc == WEOF)
		return (WEOF);
	if ((len = l->__wcrtomb(buf, wc, &fp->_mbstate)) == (size_t)-1) {
		fp->_flags |= __SERR;
		return (WEOF);
	}
	while (len-- != 0)
		if (__ungetc((unsigned char)buf[len], fp) == EOF)
			return (WEOF);

	return (wc);
}

/*
 * MT-safe version.
 */
wint_t
ungetwc_l(wint_t wc, FILE *fp, locale_t locale)
{
	wint_t r;
	FIX_LOCALE(locale);

	FLOCKFILE(fp);
	ORIENT(fp, 1);
	r = __ungetwc(wc, fp, locale);
	FUNLOCKFILE(fp);

	return (r);
}
wint_t
ungetwc(wint_t wc, FILE *fp)
{
	return ungetwc_l(wc, fp, __get_locale());
}