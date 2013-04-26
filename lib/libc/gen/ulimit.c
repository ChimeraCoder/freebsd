
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <ulimit.h>

long
ulimit(int cmd, ...)
{
	struct rlimit limit;
	va_list ap;
	long arg;

	if (cmd == UL_GETFSIZE) {
		if (getrlimit(RLIMIT_FSIZE, &limit) == -1)
			return (-1);
		limit.rlim_cur /= 512;
		if (limit.rlim_cur > LONG_MAX)
			return (LONG_MAX);
		return ((long)limit.rlim_cur);
	} else if (cmd == UL_SETFSIZE) {
		va_start(ap, cmd);
		arg = va_arg(ap, long);
		va_end(ap);
		limit.rlim_max = limit.rlim_cur = (rlim_t)arg * 512;

		/* The setrlimit() function sets errno to EPERM if needed. */
		if (setrlimit(RLIMIT_FSIZE, &limit) == -1)
			return (-1);
		if (arg * 512 > LONG_MAX)
			return (LONG_MAX);
		return (arg);
	} else {
		errno = EINVAL;
		return (-1);
	}
}