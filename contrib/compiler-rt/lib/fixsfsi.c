
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

#define SINGLE_PRECISION
#include "fp_lib.h"

ARM_EABI_FNALIAS(f2iz, fixsfsi)

COMPILER_RT_ABI int
__fixsfsi(fp_t a) {
    // Break a into sign, exponent, significand
    const rep_t aRep = toRep(a);
    const rep_t aAbs = aRep & absMask;
    const int sign = aRep & signBit ? -1 : 1;
    const int exponent = (aAbs >> significandBits) - exponentBias;
    const rep_t significand = (aAbs & significandMask) | implicitBit;
    
    // If 0 < exponent < significandBits, right shift to get the result.
    if ((unsigned int)exponent < significandBits) {
        return sign * (significand >> (significandBits - exponent));
    }
    
    // If exponent is negative, the result is zero.
    else if (exponent < 0) {
        return 0;
    }
    
    // If significandBits < exponent, left shift to get the result.  This shift
    // may end up being larger than the type width, which incurs undefined
    // behavior, but the conversion itself is undefined in that case, so
    // whatever the compiler decides to do is fine.
    else {
        return sign * (significand << (exponent - significandBits));
    }
}