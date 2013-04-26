
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ftell.c	8.2 (Berkeley) 5/4/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include "un-namespace.h"
#include "local.h"
#include "libc_private.h"

/*
 * standard ftell function.
 */
long
ftell(FILE *fp)
{
	off_t rv;

	rv = ftello(fp);
	if (rv > LONG_MAX) {
		errno = EOVERFLOW;
		return (-1);
	}
	return (rv);
}

/*
 * ftello: return current offset.
 */
off_t
ftello(FILE *fp)
{
	fpos_t rv;
	int ret;

	FLOCKFILE(fp);
	ret = _ftello(fp, &rv);
	FUNLOCKFILE(fp);
	if (ret)
		return (-1);
	if (rv < 0) {   /* Unspecified value because of ungetc() at 0 */
		errno = ESPIPE;
		return (-1);
	}
	return (rv);
}

int
_ftello(FILE *fp, fpos_t *offset)
{
	fpos_t pos;
	size_t n;

	if (fp->_seek == NULL) {
		errno = ESPIPE;			/* historic practice */
		return (1);
	}

	/*
	 * Find offset of underlying I/O object, then
	 * adjust for buffered bytes.
	 */
	if (fp->_flags & __SOFF)
		pos = fp->_offset;
	else {
		pos = _sseek(fp, (fpos_t)0, SEEK_CUR);
		if (pos == -1)
			return (1);
	}
	if (fp->_flags & __SRD) {
		/*
		 * Reading.  Any unread characters (including
		 * those from ungetc) cause the position to be
		 * smaller than that in the underlying object.
		 */
		if ((pos -= (HASUB(fp) ? fp->_ur : fp->_r)) < 0) {
			fp->_flags |= __SERR;
			errno = EIO;
			return (1);
		}
		if (HASUB(fp))
			pos -= fp->_r;  /* Can be negative at this point. */
	} else if ((fp->_flags & __SWR) && fp->_p != NULL) {
		/*
		 * Writing.  Any buffered characters cause the
		 * position to be greater than that in the
		 * underlying object.
		 */
		n = fp->_p - fp->_bf._base;
		if (pos > OFF_MAX - n) {
			errno = EOVERFLOW;
			return (1);
		}
		pos += n;
	}
	*offset = pos;
	return (0);
}