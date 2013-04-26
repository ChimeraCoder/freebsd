
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
 * atouint - convert an ascii string to an unsigned long, with error checking
 */#include <sys/types.h>
#include <ctype.h>

#include "ntp_types.h"
#include "ntp_stdlib.h"

int
atouint(
	const char *str,
	u_long *uval
	)
{
	register u_long u;
	register const char *cp;

	cp = str;
	if (*cp == '\0')
	    return 0;

	u = 0;
	while (*cp != '\0') {
		if (!isdigit((int)*cp))
		    return 0;
		if (u > 429496729 || (u == 429496729 && *cp >= '6'))
		    return 0;	/* overflow */
		u = (u << 3) + (u << 1);
		u += *cp++ - '0';	/* ascii dependent */
	}

	*uval = u;
	return 1;
}