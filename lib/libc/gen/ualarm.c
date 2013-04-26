
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
static char sccsid[] = "@(#)ualarm.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/time.h>
#include <unistd.h>

#define	USPS	1000000		/* # of microseconds in a second */

/*
 * Generate a SIGALRM signal in ``usecs'' microseconds.
 * If ``reload'' is non-zero, keep generating SIGALRM
 * every ``reload'' microseconds after the first signal.
 */
useconds_t
ualarm(usecs, reload)
	useconds_t usecs;
	useconds_t reload;
{
	struct itimerval new, old;

	new.it_interval.tv_usec = reload % USPS;
	new.it_interval.tv_sec = reload / USPS;

	new.it_value.tv_usec = usecs % USPS;
	new.it_value.tv_sec = usecs / USPS;

	if (setitimer(ITIMER_REAL, &new, &old) == 0)
		return (old.it_value.tv_sec * USPS + old.it_value.tv_usec);
	/* else */
		return (-1);
}