
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

#define DOUBLE_PRECISION
#include "fp_lib.h"

#include "int_lib.h"

ARM_EABI_FNALIAS(i2d, floatsidf)

fp_t __floatsidf(int a) {
    
    const int aWidth = sizeof a * CHAR_BIT;
    
    // Handle zero as a special case to protect clz
    if (a == 0)
        return fromRep(0);
    
    // All other cases begin by extracting the sign and absolute value of a
    rep_t sign = 0;
    if (a < 0) {
        sign = signBit;
        a = -a;
    }
    
    // Exponent of (fp_t)a is the width of abs(a).
    const int exponent = (aWidth - 1) - __builtin_clz(a);
    rep_t result;
    
    // Shift a into the significand field and clear the implicit bit.  Extra
    // cast to unsigned int is necessary to get the correct behavior for
    // the input INT_MIN.
    const int shift = significandBits - exponent;
    result = (rep_t)(unsigned int)a << shift ^ implicitBit;
    
    // Insert the exponent
    result += (rep_t)(exponent + exponentBias) << significandBits;
    // Insert the sign bit and return
    return fromRep(result | sign);
}