
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

#ifndef lint
static char rcsid[] = "$FreeBSD$";
#endif /* not lint */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>

int
ftime(struct timeb *tbp)
{
	struct timezone tz;
	struct timeval t;

	if (gettimeofday(&t, &tz) < 0)
		return (-1);
	tbp->millitm = t.tv_usec / 1000;
	tbp->time = t.tv_sec;
	tbp->timezone = tz.tz_minuteswest;
	tbp->dstflag = tz.tz_dsttime;

	return (0);
}