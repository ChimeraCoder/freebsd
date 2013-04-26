
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

#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "libc_private.h"

__weak_reference(__fcntl_compat, fcntl);

int
__fcntl_compat(int fd, int cmd, ...)
{
	va_list args;
	long arg;
	struct __oflock ofl;
	struct flock *flp;
	int res;

	va_start(args, cmd);
	arg = va_arg(args, long);
	va_end(args);

	if (__getosreldate() >= 800028) {
		return (__sys_fcntl(fd, cmd, arg));
	} else {
		if (cmd == F_GETLK || cmd == F_SETLK || cmd == F_SETLKW) {
			/*
			 * Convert new-style struct flock (which
			 * includes l_sysid) to old-style.
			 */
			flp = (struct flock *) (uintptr_t) arg;
			ofl.l_start = flp->l_start;
			ofl.l_len = flp->l_len;
			ofl.l_pid = flp->l_pid;
			ofl.l_type = flp->l_type;
			ofl.l_whence = flp->l_whence;

			switch (cmd) {
			case F_GETLK:
				res = __sys_fcntl(fd, F_OGETLK, &ofl);
				if (res >= 0) {
					flp->l_start = ofl.l_start;
					flp->l_len = ofl.l_len;
					flp->l_pid = ofl.l_pid;
					flp->l_type = ofl.l_type;
					flp->l_whence = ofl.l_whence;
					flp->l_sysid = 0;
				}
				return (res);

			case F_SETLK:
				return (__sys_fcntl(fd, F_OSETLK, &ofl));

			case F_SETLKW:
				return (__sys_fcntl(fd, F_OSETLKW, &ofl));
			}
		}
		return (__sys_fcntl(fd, cmd, arg));
	}
}