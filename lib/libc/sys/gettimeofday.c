
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
#include "libc_private.h"

int __gettimeofday(struct timeval *tv, struct timezone *tz);

__weak_reference(__gettimeofday, gettimeofday);

int
__gettimeofday(struct timeval *tv, struct timezone *tz)
{
	int error;

	error = __vdso_gettimeofday(tv, tz);
	if (error == ENOSYS)
		error = __sys_gettimeofday(tv, tz);
	return (error);
}