
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
static char sccsid[] = "@(#)floatunsdidf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "quad.h"

/*
 * Convert (unsigned) quad to double.
 * This is exactly like floatdidf.c except that negatives never occur.
 */
double
__floatunsdidf(x)
	u_quad_t x;
{
	double d;
	union uu u;

	u.uq = x;
	d = (double)u.ul[H] * ((1 << (LONG_BITS - 2)) * 4.0);
	d += u.ul[L];
	return (d);
}