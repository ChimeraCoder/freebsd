
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

/* return year day for Easter */

/*
 * This code is based on the Calendar FAQ's code for how to calculate
 * easter is. This is the Gregorian calendar version. They refer to
 * the Algorithm of Oudin in the "Explanatory Supplement to the
 * Astronomical Almanac".
 */

int
easter(int year) /* 0 ... abcd, NOT since 1900 */
{
	int G,	/* Golden number - 1 */
	    C,	/* Century */
	    H,	/* 23 - epact % 30 */
	    I,	/* days from 21 March to Paschal full moon */
	    J,	/* weekday of full moon */
	    L;	/* days from 21 March to Sunday on of before full moon */

	G = year % 19;
	C = year / 100;
	H = (C - C / 4 - (8 * C + 13) / 25 + 19 * G + 15) % 30;
	I = H - (H / 28) * (1 - (H / 28) * (29 / (H + 1)) * ((21 - G) / 11));
	J = (year + year / 4 + I + 2 - C + C / 4) % 7;

	L = I - J;

	if (isleap(year))
		return 31 + 29 + 21 + L + 7;
	else
		return 31 + 28 + 21 + L + 7;
}