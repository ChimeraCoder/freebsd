
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
static char sccsid[] = "@(#)fclose.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "un-namespace.h"
#include <spinlock.h>
#include "libc_private.h"
#include "local.h"

int
fclose(FILE *fp)
{
	int r;

	if (fp->_flags == 0) {	/* not open! */
		errno = EBADF;
		return (EOF);
	}
	FLOCKFILE(fp);
	r = fp->_flags & __SWR ? __sflush(fp) : 0;
	if (fp->_close != NULL && (*fp->_close)(fp->_cookie) < 0)
		r = EOF;
	if (fp->_flags & __SMBF)
		free((char *)fp->_bf._base);
	if (HASUB(fp))
		FREEUB(fp);
	if (HASLB(fp))
		FREELB(fp);
	fp->_file = -1;
	fp->_r = fp->_w = 0;	/* Mess up if reaccessed. */

	/*
	 * Lock the spinlock used to protect __sglue list walk in
	 * __sfp().  The __sfp() uses fp->_flags == 0 test as an
	 * indication of the unused FILE.
	 *
	 * Taking the lock prevents possible compiler or processor
	 * reordering of the writes performed before the final _flags
	 * cleanup, making sure that we are done with the FILE before
	 * it is considered available.
	 */
	STDIO_THREAD_LOCK();
	fp->_flags = 0;		/* Release this FILE for reuse. */
	STDIO_THREAD_UNLOCK();
	FUNLOCKFILE(fp);
	return (r);
}