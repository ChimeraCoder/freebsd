
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
#include <stdio.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "xlocale_private.h"

wchar_t *
fgetwln_l(FILE * __restrict fp, size_t *lenp, locale_t locale)
{
	wint_t wc;
	size_t len;
	FIX_LOCALE(locale);

	FLOCKFILE(fp);
	ORIENT(fp, 1);

	len = 0;
	while ((wc = __fgetwc(fp, locale)) != WEOF) {
#define	GROW	512
		if (len * sizeof(wchar_t) >= fp->_lb._size &&
		    __slbexpand(fp, (len + GROW) * sizeof(wchar_t)))
			goto error;
		*((wchar_t *)fp->_lb._base + len++) = wc;
		if (wc == L'\n')
			break;
	}
	if (len == 0)
		goto error;

	FUNLOCKFILE(fp);
	*lenp = len;
	return ((wchar_t *)fp->_lb._base);

error:
	FUNLOCKFILE(fp);
	*lenp = 0;
	return (NULL);
}
wchar_t *
fgetwln(FILE * __restrict fp, size_t *lenp)
{
	return fgetwln_l(fp, lenp, __get_locale());
}