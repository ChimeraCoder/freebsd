
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
/*
 * ymd2yd - compute the date in the year from y/m/d
 */
#include "ntp_fp.h"
#include "ntp_unixtime.h"
#include "ntp_stdlib.h"

/*
 * Tables to compute the day of year from yyyymmdd timecode.
 * Viva la leap.
 */
static int day1tab[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int day2tab[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int
ymd2yd(
	int y,
	int m,
	int d
	)
{
	int i, *t;

	if (m < 1 || m > 12 || d < 1)
		return (-1);

	if (((y%4 == 0) && (y%100 != 0)) || (y%400 == 0))
		t = day2tab;	/* leap year */
	else
		t = day1tab;	/* not a leap year */
	if (d > t[m - 1])
		return (-1);
	for (i = 0; i < m - 1; i++)
		d += t[i];
	return d;
}