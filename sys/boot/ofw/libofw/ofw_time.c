
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
#include "openfirm.h"

time_t
time(time_t *tloc)
{
	int secs;

	secs = OF_milliseconds() / 1000;
	if (tloc)
		*tloc = secs;
	return secs;
}

int
getsecs()
{
	time_t	n = 0;
	time(&n);
	return n;
}

void
delay(int usecs)
{
	int	msecs, start;

	msecs = usecs / 1000;
	start = OF_milliseconds();

	while (OF_milliseconds() - start < msecs);
}