
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

#include <stand.h>

#include "glue.h"

/*
 * Return the time in seconds since the beginning of the day.
 */
time_t
time(time_t *tloc)
{
	int secs;

	secs = ub_get_timer(0) / 1000;
	if (tloc)
		*tloc = secs;

	return (secs);
}

int
getsecs(void)
{

	return (time(NULL));
}

/*
 * Use U-Boot udelay() function to wait for a given microseconds period
 */
void
delay(int usecs)
{

	ub_udelay(usecs);
}