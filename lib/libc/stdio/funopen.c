
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
static char sccsid[] = "@(#)funopen.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>
#include <errno.h>

#include "local.h"

FILE *
funopen(const void *cookie,
	int (*readfn)(void *, char *, int),
	int (*writefn)(void *, const char *, int),
	fpos_t (*seekfn)(void *, fpos_t, int),
	int (*closefn)(void *))
{
	FILE *fp;
	int flags;

	if (readfn == NULL) {
		if (writefn == NULL) {		/* illegal */
			errno = EINVAL;
			return (NULL);
		} else
			flags = __SWR;		/* write only */
	} else {
		if (writefn == NULL)
			flags = __SRD;		/* read only */
		else
			flags = __SRW;		/* read-write */
	}
	if ((fp = __sfp()) == NULL)
		return (NULL);
	fp->_flags = flags;
	fp->_file = -1;
	fp->_cookie = (void *)cookie;
	fp->_read = readfn;
	fp->_write = writefn;
	fp->_seek = seekfn;
	fp->_close = closefn;
	return (fp);
}