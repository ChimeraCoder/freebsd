
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

ARM_EABI_FNALIAS(ui2d, floatunsidf)

fp_t __floatunsidf(unsigned int a) {
    
    const int aWidth = sizeof a * CHAR_BIT;
    
    // Handle zero as a special case to protect clz
    if (a == 0) return fromRep(0);
    
    // Exponent of (fp_t)a is the width of abs(a).
    const int exponent = (aWidth - 1) - __builtin_clz(a);
    rep_t result;
    
    // Shift a into the significand field and clear the implicit bit.
    const int shift = significandBits - exponent;
    result = (rep_t)a << shift ^ implicitBit;
    
    // Insert the exponent
    result += (rep_t)(exponent + exponentBias) << significandBits;
    return fromRep(result);
}