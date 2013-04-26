
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
static char sccsid[] = "@(#)clock.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>

/*
 * Convert usec to clock ticks; could do (usec * CLOCKS_PER_SEC) / 1000000,
 * but this would overflow if we switch to nanosec.
 */
#define	CONVTCK(r)	((r).tv_sec * CLOCKS_PER_SEC \
			 + (r).tv_usec / (1000000 / CLOCKS_PER_SEC))

clock_t
clock()
{
	struct rusage ru;

	if (getrusage(RUSAGE_SELF, &ru))
		return ((clock_t) -1);
	return((clock_t)((CONVTCK(ru.ru_utime) + CONVTCK(ru.ru_stime))));
}