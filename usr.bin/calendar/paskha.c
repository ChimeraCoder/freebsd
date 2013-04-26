
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calendar.h"

#define	PASKHA		"paskha"
#define	PASKHALEN	(sizeof(PASKHA) - 1)

/* return difference in days between Julian and Gregorian calendars */
int
j2g(int year)
{
	return (year < 1500) ?
		0 :
		10 + (year/100 - 16) - ((year/100 - 16) / 4);
}

/* return year day for Orthodox Easter using Gauss formula */
/* (new style result) */

int
paskha(int R) /*year*/
{
	int a, b, c, d, e;
	static int x = 15;
	static int y = 6;
	int *cumday;

	a = R % 19;
	b = R % 4;
	c = R % 7;
	d = (19 * a + x) % 30;
	e = (2 * b + 4 * c + 6 * d + y) % 7;
	cumday = cumdaytab[isleap(R)];
	return (((cumday[3] + 1) + 22) + (d + e) + j2g(R));
}