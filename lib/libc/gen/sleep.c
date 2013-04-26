
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
static char sccsid[] = "@(#)sleep.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include "un-namespace.h"

unsigned int
__sleep(unsigned int seconds)
{
	struct timespec time_to_sleep;
	struct timespec time_remaining;

	/*
	 * Avoid overflow when `seconds' is huge.  This assumes that
	 * the maximum value for a time_t is >= INT_MAX.
	 */
	if (seconds > INT_MAX)
		return (seconds - INT_MAX + __sleep(INT_MAX));

	time_to_sleep.tv_sec = seconds;
	time_to_sleep.tv_nsec = 0;
	if (_nanosleep(&time_to_sleep, &time_remaining) != -1)
		return (0);
	if (errno != EINTR)
		return (seconds);		/* best guess */
	return (time_remaining.tv_sec +
		(time_remaining.tv_nsec != 0)); /* round up */
}

__weak_reference(__sleep, sleep);
__weak_reference(__sleep, _sleep);