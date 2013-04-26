
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
static char sccsid[] = "@(#)fflush.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <errno.h>
#include <stdio.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

static int	sflush_locked(FILE *);

/*
 * Flush a single file, or (if fp is NULL) all files.
 * MT-safe version
 */
int
fflush(FILE *fp)
{
	int retval;

	if (fp == NULL)
		return (_fwalk(sflush_locked));
	FLOCKFILE(fp);

	/*
	 * There is disagreement about the correct behaviour of fflush()
	 * when passed a file which is not open for reading.  According to
	 * the ISO C standard, the behaviour is undefined.
	 * Under linux, such an fflush returns success and has no effect;
	 * under Windows, such an fflush is documented as behaving instead
	 * as fpurge().
	 * Given that applications may be written with the expectation of
	 * either of these two behaviours, the only safe (non-astonishing)
	 * option is to return EBADF and ask that applications be fixed.
	 */
	if ((fp->_flags & (__SWR | __SRW)) == 0) {
		errno = EBADF;
		retval = EOF;
	} else
		retval = __sflush(fp);
	FUNLOCKFILE(fp);
	return (retval);
}

/*
 * Flush a single file, or (if fp is NULL) all files.
 * Non-MT-safe version
 */
int
__fflush(FILE *fp)
{
	int retval;

	if (fp == NULL)
		return (_fwalk(sflush_locked));
	if ((fp->_flags & (__SWR | __SRW)) == 0) {
		errno = EBADF;
		retval = EOF;
	} else
		retval = __sflush(fp);
	return (retval);
}

int
__sflush(FILE *fp)
{
	unsigned char *p;
	int n, t;

	t = fp->_flags;
	if ((t & __SWR) == 0)
		return (0);

	if ((p = fp->_bf._base) == NULL)
		return (0);

	n = fp->_p - p;		/* write this much */

	/*
	 * Set these immediately to avoid problems with longjmp and to allow
	 * exchange buffering (via setvbuf) in user write function.
	 */
	fp->_p = p;
	fp->_w = t & (__SLBF|__SNBF) ? 0 : fp->_bf._size;

	for (; n > 0; n -= t, p += t) {
		t = _swrite(fp, (char *)p, n);
		if (t <= 0) {
			fp->_flags |= __SERR;
			return (EOF);
		}
	}
	return (0);
}

static int
sflush_locked(FILE *fp)
{
	int	ret;

	FLOCKFILE(fp);
	ret = __sflush(fp);
	FUNLOCKFILE(fp);
	return (ret);
}