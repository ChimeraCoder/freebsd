
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

#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/vdso.h>
#include <errno.h>
#include <time.h>
#include "libc_private.h"

int __clock_gettime(clockid_t, struct timespec *ts);

__weak_reference(__clock_gettime, clock_gettime);

int
__clock_gettime(clockid_t clock_id, struct timespec *ts)
{
	int error;

	if (__vdso_clock_gettime != NULL && __vdso_gettc != NULL)
		error = __vdso_clock_gettime(clock_id, ts);
	else
		error = ENOSYS;
	if (error == ENOSYS)
		error = __sys_clock_gettime(clock_id, ts);
	return (error);
}