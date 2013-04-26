
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
static char sccsid[] = "@(#)flags.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <errno.h>

#include "local.h"

/*
 * Return the (stdio) flags for a given mode.  Store the flags
 * to be passed to an _open() syscall through *optr.
 * Return 0 on error.
 */
int
__sflags(const char *mode, int *optr)
{
	int ret, m, o;

	switch (*mode++) {

	case 'r':	/* open for reading */
		ret = __SRD;
		m = O_RDONLY;
		o = 0;
		break;

	case 'w':	/* open for writing */
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_TRUNC;
		break;

	case 'a':	/* open for appending */
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_APPEND;
		break;

	default:	/* illegal mode */
		errno = EINVAL;
		return (0);
	}

	/* 'b' (binary) is ignored */
	if (*mode == 'b')
		mode++;

	/* [rwa][b]\+ means read and write */
	if (*mode == '+') {
		mode++;
		ret = __SRW;
		m = O_RDWR;
	}

	/* 'b' (binary) can appear here, too -- and is ignored again */
	if (*mode == 'b')
		mode++;

	/* 'x' means exclusive (fail if the file exists) */
	if (*mode == 'x') {
		mode++;
		if (m == O_RDONLY) {
			errno = EINVAL;
			return (0);
		}
		o |= O_EXCL;
	}

	/* set close-on-exec */
	if (*mode == 'e')
		o |= O_CLOEXEC;

	*optr = m | o;
	return (ret);
}