
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
 * Shift a (signed) quad value left (arithmetic shift left).
 * This is the same as logical shift left!
 */
quad_t
__ashldi3(a, shift)
	quad_t a;
	qshift_t shift;
{
	union uu aa;

	aa.q = a;
	if (shift >= LONG_BITS) {
		aa.ul[H] = shift >= QUAD_BITS ? 0 :
		    aa.ul[L] << (shift - LONG_BITS);
		aa.ul[L] = 0;
	} else if (shift > 0) {
		aa.ul[H] = (aa.ul[H] << shift) |
		    (aa.ul[L] >> (LONG_BITS - shift));
		aa.ul[L] <<= shift;
	}
	return (aa.q);
}