
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
#include <stdarg.h>
#include <stdio.h>
#include "un-namespace.h"

#include "local.h"
#include "xlocale_private.h"

int
vdprintf(int fd, const char * __restrict fmt, va_list ap)
{
	FILE f = FAKE_FILE;
	unsigned char buf[BUFSIZ];
	int ret;

	if (fd > SHRT_MAX) {
		errno = EMFILE;
		return (EOF);
	}

	f._p = buf;
	f._w = sizeof(buf);
	f._flags = __SWR;
	f._file = fd;
	f._cookie = &f;
	f._write = __swrite;
	f._bf._base = buf;
	f._bf._size = sizeof(buf);

	if ((ret = __vfprintf(&f, __get_locale(), fmt, ap)) < 0)
		return (ret);

	return (__fflush(&f) ? EOF : ret);
}