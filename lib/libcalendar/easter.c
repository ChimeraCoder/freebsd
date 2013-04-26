
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

#include "calendar.h"

typedef struct date date;

static int	 easterodn(int y);

/* Compute Easter Sunday in Gregorian Calendar */
date *
easterg(int y, date *dt)
{
	int c, i, j, k, l, n;

	n = y % 19;
	c = y / 100;
	k = (c - 17) / 25;
	i = (c - c/4 -(c-k)/3 + 19 * n + 15) % 30;
	i = i -(i/28) * (1 - (i/28) * (29/(i + 1)) * ((21 - n)/11));
	j = (y + y/4 + i + 2 - c + c/4) % 7;
	l = i - j;
	dt->m = 3 + (l + 40) / 44;
	dt->d = l + 28 - 31*(dt->m / 4);
	dt->y = y;
	return (dt);
}

/* Compute the Gregorian date of Easter Sunday in Julian Calendar */
date *
easterog(int y, date *dt)
{

	return (gdate(easterodn(y), dt));
}

/* Compute the Julian date of Easter Sunday in Julian Calendar */
date   *
easteroj(int y, date * dt)
{

	return (jdate(easterodn(y), dt));
}

/* Compute the day number of Easter Sunday in Julian Calendar */
static int
easterodn(int y)
{
	/*
	 * Table for the easter limits in one metonic (19-year) cycle. 21
	 * to 31 is in March, 1 through 18 in April. Easter is the first
	 * sunday after the easter limit.
	 */
	int     mc[] = {5, 25, 13, 2, 22, 10, 30, 18, 7, 27, 15, 4,
		    24, 12, 1, 21, 9, 29, 17};

	/* Offset from a weekday to next sunday */
	int     ns[] = {6, 5, 4, 3, 2, 1, 7};
	date	dt;
	int     dn;

	/* Assign the easter limit of y to dt */
	dt.d = mc[y % 19];

	if (dt.d < 21)
		dt.m = 4;
	else
		dt.m = 3;

	dt.y = y;

	/* Return the next sunday after the easter limit */
	dn = ndaysj(&dt);
	return (dn + ns[weekday(dn)]);
}