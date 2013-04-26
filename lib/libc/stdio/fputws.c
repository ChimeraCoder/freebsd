
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
#include <wchar.h>
#include "un-namespace.h"
#include "fvwrite.h"
#include "libc_private.h"
#include "local.h"
#include "mblocal.h"

int
fputws_l(const wchar_t * __restrict ws, FILE * __restrict fp, locale_t locale)
{
	size_t nbytes;
	char buf[BUFSIZ];
	struct __suio uio;
	struct __siov iov;
	const wchar_t *wsp;
	FIX_LOCALE(locale);
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	FLOCKFILE(fp);
	ORIENT(fp, 1);
	if (prepwrite(fp) != 0)
		goto error;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	iov.iov_base = buf;
	wsp = ws;
	do {
		nbytes = l->__wcsnrtombs(buf, &wsp, SIZE_T_MAX, sizeof(buf),
		    &fp->_mbstate);
		if (nbytes == (size_t)-1)
			goto error;
		iov.iov_len = uio.uio_resid = nbytes;
		if (__sfvwrite(fp, &uio) != 0)
			goto error;
	} while (wsp != NULL);
	FUNLOCKFILE(fp);
	return (0);

error:
	FUNLOCKFILE(fp);
	return (-1);
}

int
fputws(const wchar_t * __restrict ws, FILE * __restrict fp)
{
	return fputws_l(ws, fp, __get_locale());
}