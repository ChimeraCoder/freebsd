
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

#include <math.h>

#include "fpmath.h"
#include "../src/math_private.h"

long double
nanl(const char *s)
{
	union {
		union IEEEl2bits ieee;
		uint32_t bits[3];
	} u;

	_scan_nan(u.bits, 3, s);
	u.ieee.bits.exp = 0x7fff;
	u.ieee.bits.manh |= 0xc0000000;	/* make it a quiet NaN */
	return (u.ieee.e);
}