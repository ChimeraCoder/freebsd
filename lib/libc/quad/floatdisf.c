
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
static char sccsid[] = "@(#)floatdisf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "quad.h"

/*
 * Convert (signed) quad to float.
 */
float
__floatdisf(x)
	quad_t x;
{
	float f;
	union uu u;
	int neg;

	/*
	 * Get an unsigned number first, by negating if necessary.
	 */
	if (x < 0)
		u.q = -x, neg = 1;
	else
		u.q = x, neg = 0;

	/*
	 * Now u.ul[H] has the factor of 2^32 (or whatever) and u.ul[L]
	 * has the units.  Ideally we could just set f, add LONG_BITS to
	 * its exponent, and then add the units, but this is portable
	 * code and does not know how to get at an exponent.  Machine-
	 * specific code may be able to do this more efficiently.
	 *
	 * Using double here may be excessive paranoia.
	 */
	f = (double)u.ul[H] * ((1 << (LONG_BITS - 2)) * 4.0);
	f += u.ul[L];

	return (neg ? -f : f);
}