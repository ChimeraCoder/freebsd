
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

enum LE_RESULT {
    LE_LESS      = -1,
    LE_EQUAL     =  0,
    LE_GREATER   =  1,
    LE_UNORDERED =  1
};

enum LE_RESULT __lesf2(fp_t a, fp_t b) {
    
    const srep_t aInt = toRep(a);
    const srep_t bInt = toRep(b);
    const rep_t aAbs = aInt & absMask;
    const rep_t bAbs = bInt & absMask;
    
    // If either a or b is NaN, they are unordered.
    if (aAbs > infRep || bAbs > infRep) return LE_UNORDERED;
    
    // If a and b are both zeros, they are equal.
    if ((aAbs | bAbs) == 0) return LE_EQUAL;
    
    // If at least one of a and b is positive, we get the same result comparing
    // a and b as signed integers as we would with a fp_ting-point compare.
    if ((aInt & bInt) >= 0) {
        if (aInt < bInt) return LE_LESS;
        else if (aInt == bInt) return LE_EQUAL;
        else return LE_GREATER;
    }
    
    // Otherwise, both are negative, so we need to flip the sense of the
    // comparison to get the correct result.  (This assumes a twos- or ones-
    // complement integer representation; if integers are represented in a
    // sign-magnitude representation, then this flip is incorrect).
    else {
        if (aInt > bInt) return LE_LESS;
        else if (aInt == bInt) return LE_EQUAL;
        else return LE_GREATER;
    }
}

enum GE_RESULT {
    GE_LESS      = -1,
    GE_EQUAL     =  0,
    GE_GREATER   =  1,
    GE_UNORDERED = -1   // Note: different from LE_UNORDERED
};

enum GE_RESULT __gesf2(fp_t a, fp_t b) {
    
    const srep_t aInt = toRep(a);
    const srep_t bInt = toRep(b);
    const rep_t aAbs = aInt & absMask;
    const rep_t bAbs = bInt & absMask;
    
    if (aAbs > infRep || bAbs > infRep) return GE_UNORDERED;
    if ((aAbs | bAbs) == 0) return GE_EQUAL;
    if ((aInt & bInt) >= 0) {
        if (aInt < bInt) return GE_LESS;
        else if (aInt == bInt) return GE_EQUAL;
        else return GE_GREATER;
    } else {
        if (aInt > bInt) return GE_LESS;
        else if (aInt == bInt) return GE_EQUAL;
        else return GE_GREATER;
    }
}

int __unordsf2(fp_t a, fp_t b) {
    const rep_t aAbs = toRep(a) & absMask;
    const rep_t bAbs = toRep(b) & absMask;
    return aAbs > infRep || bAbs > infRep;
}

// The following are alternative names for the preceeding routines.

enum LE_RESULT __eqsf2(fp_t a, fp_t b) {
    return __lesf2(a, b);
}

enum LE_RESULT __ltsf2(fp_t a, fp_t b) {
    return __lesf2(a, b);
}

enum LE_RESULT __nesf2(fp_t a, fp_t b) {
    return __lesf2(a, b);
}

enum GE_RESULT __gtsf2(fp_t a, fp_t b) {
    return __gesf2(a, b);
}