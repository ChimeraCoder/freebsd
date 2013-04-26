
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

#include <libkern/quad.h>

/*
 * Divide two signed quads.
 * ??? if -1/2 should produce -1 on this machine, this code is wrong
 */
quad_t
__divdi3(a, b)
	quad_t a, b;
{
	u_quad_t ua, ub, uq;
	int neg;

	if (a < 0)
		ua = -(u_quad_t)a, neg = 1;
	else
		ua = a, neg = 0;
	if (b < 0)
		ub = -(u_quad_t)b, neg ^= 1;
	else
		ub = b;
	uq = __qdivrem(ua, ub, (u_quad_t *)0);
	return (neg ? -uq : uq);
}