
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
static char sccsid[] = "@(#)stdio.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "un-namespace.h"
#include "local.h"

/*
 * Small standard I/O/seek/close functions.
 */
int
__sread(void *cookie, char *buf, int n)
{
	FILE *fp = cookie;

	return(_read(fp->_file, buf, (size_t)n));
}

int
__swrite(void *cookie, char const *buf, int n)
{
	FILE *fp = cookie;

	return (_write(fp->_file, buf, (size_t)n));
}

fpos_t
__sseek(void *cookie, fpos_t offset, int whence)
{
	FILE *fp = cookie;

	return (lseek(fp->_file, (off_t)offset, whence));
}

int
__sclose(void *cookie)
{

	return (_close(((FILE *)cookie)->_file));
}

/*
 * Higher level wrappers.
 */
int
_sread(FILE *fp, char *buf, int n)
{
	int ret;

	ret = (*fp->_read)(fp->_cookie, buf, n);
	if (ret > 0) {
		if (fp->_flags & __SOFF) {
			if (fp->_offset <= OFF_MAX - ret)
				fp->_offset += ret;
			else
				fp->_flags &= ~__SOFF;
		}
	} else if (ret < 0)
		fp->_flags &= ~__SOFF;
	return (ret);
}

int
_swrite(FILE *fp, char const *buf, int n)
{
	int ret;
	int serrno;

	if (fp->_flags & __SAPP) {
		serrno = errno;
		if (_sseek(fp, (fpos_t)0, SEEK_END) == -1 &&
		    (fp->_flags & __SOPT))
			return (-1);
		errno = serrno;
	}
	ret = (*fp->_write)(fp->_cookie, buf, n);
	/* __SOFF removed even on success in case O_APPEND mode is set. */
	if (ret >= 0) {
		if ((fp->_flags & (__SAPP|__SOFF)) == (__SAPP|__SOFF) &&
		    fp->_offset <= OFF_MAX - ret)
			fp->_offset += ret;
		else
			fp->_flags &= ~__SOFF;

	} else if (ret < 0)
		fp->_flags &= ~__SOFF;
	return (ret);
}

fpos_t
_sseek(FILE *fp, fpos_t offset, int whence)
{
	fpos_t ret;
	int serrno, errret;

	serrno = errno;
	errno = 0;
	ret = (*fp->_seek)(fp->_cookie, offset, whence);
	errret = errno;
	if (errno == 0)
		errno = serrno;
	/*
	 * Disallow negative seeks per POSIX.
	 * It is needed here to help upper level caller
	 * in the cases it can't detect.
	 */
	if (ret < 0) {
		if (errret == 0) {
			if (offset != 0 || whence != SEEK_CUR) {
				if (HASUB(fp))
					FREEUB(fp);
				fp->_p = fp->_bf._base;
				fp->_r = 0;
				fp->_flags &= ~__SEOF;
			}
			fp->_flags |= __SERR;
			errno = EINVAL;
		} else if (errret == ESPIPE)
			fp->_flags &= ~__SAPP;
		fp->_flags &= ~__SOFF;
		ret = -1;
	} else if (fp->_flags & __SOPT) {
		fp->_flags |= __SOFF;
		fp->_offset = ret;
	}
	return (ret);
}