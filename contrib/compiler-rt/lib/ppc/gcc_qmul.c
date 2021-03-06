
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

/* long double __gcc_qmul(long double x, long double y);
 * This file implements the PowerPC 128-bit double-double multiply operation.
 * This implementation is shamelessly cribbed from Apple's DDRT, circa 1993(!)
 */

#include "DD.h"

long double __gcc_qmul(long double x, long double y)
{	
	static const uint32_t infinityHi = UINT32_C(0x7ff00000);
	DD dst = { .ld = x }, src = { .ld = y };
	
	register double A = dst.s.hi, a = dst.s.lo,
					B = src.s.hi, b = src.s.lo;
	
	double aHi, aLo, bHi, bLo;
    double ab, tmp, tau;
	
	ab = A * B;
	
	/* Detect special cases */
	if (ab == 0.0) {
		dst.s.hi = ab;
		dst.s.lo = 0.0;
		return dst.ld;
	}
	
	const doublebits abBits = { .d = ab };
	if (((uint32_t)(abBits.x >> 32) & infinityHi) == infinityHi) {
		dst.s.hi = ab;
		dst.s.lo = 0.0;
		return dst.ld;
	}
	
	/* Generic cases handled here. */
    aHi = high26bits(A);
    bHi = high26bits(B);
    aLo = A - aHi;
    bLo = B - bHi;
	
    tmp = LOWORDER(ab, aHi, aLo, bHi, bLo);
    tmp += (A * b + a * B);
    tau = ab + tmp;
	
    dst.s.lo = (ab - tau) + tmp;
    dst.s.hi = tau;
	
    return dst.ld;
}