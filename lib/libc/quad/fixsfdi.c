
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
static char sccsid[] = "@(#)fixsfdi.c	5.1 (Berkeley) 7/7/92";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "quad.h"

/*
 * Convert float to (signed) quad.
 * We clamp anything that is out of range.
 *
 * N.B.: must use new ANSI syntax (sorry).
 */
long long
__fixsfdi(float x)
{
	if (x < 0)
		if (x <= QUAD_MIN)
			return (QUAD_MIN);
		else
			return ((quad_t)-(u_quad_t)-x);
	else
		if (x >= QUAD_MAX)
			return (QUAD_MAX);
		else
			return ((quad_t)(u_quad_t)x);
}