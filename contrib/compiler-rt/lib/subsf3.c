
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

fp_t COMPILER_RT_ABI __addsf3(fp_t a, fp_t b);

ARM_EABI_FNALIAS(fsub, subsf3)

// Subtraction; flip the sign bit of b and add.
COMPILER_RT_ABI fp_t
__subsf3(fp_t a, fp_t b) {
    return __addsf3(a, fromRep(toRep(b) ^ signBit));
}

/* FIXME: rsub for ARM EABI */