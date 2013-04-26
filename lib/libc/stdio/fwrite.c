
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
static char sccsid[] = "@(#)fwrite.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include "un-namespace.h"
#include "local.h"
#include "fvwrite.h"
#include "libc_private.h"

/*
 * Write `count' objects (each size `size') from memory to the given file.
 * Return the number of whole objects written.
 */
size_t
fwrite(const void * __restrict buf, size_t size, size_t count, FILE * __restrict fp)
{
	size_t n;
	struct __suio uio;
	struct __siov iov;

	/*
	 * ANSI and SUSv2 require a return value of 0 if size or count are 0.
	 */
	if ((count == 0) || (size == 0))
		return (0);

	/*
	 * Check for integer overflow.  As an optimization, first check that
	 * at least one of {count, size} is at least 2^16, since if both
	 * values are less than that, their product can't possible overflow
	 * (size_t is always at least 32 bits on FreeBSD).
	 */
	if (((count | size) > 0xFFFF) &&
	    (count > SIZE_MAX / size)) {
		errno = EINVAL;
		fp->_flags |= __SERR;
		return (0);
	}

	n = count * size;

	iov.iov_base = (void *)buf;
	uio.uio_resid = iov.iov_len = n;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;

	FLOCKFILE(fp);
	ORIENT(fp, -1);
	/*
	 * The usual case is success (__sfvwrite returns 0);
	 * skip the divide if this happens, since divides are
	 * generally slow and since this occurs whenever size==0.
	 */
	if (__sfvwrite(fp, &uio) != 0)
	    count = (n - uio.uio_resid) / size;
	FUNLOCKFILE(fp);
	return (count);
}