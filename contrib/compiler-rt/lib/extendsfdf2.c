
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

#include "int_lib.h"

typedef float src_t;
typedef uint32_t src_rep_t;
#define SRC_REP_C UINT32_C
static const int srcSigBits = 23;
#define src_rep_t_clz __builtin_clz

typedef double dst_t;
typedef uint64_t dst_rep_t;
#define DST_REP_C UINT64_C
static const int dstSigBits = 52;

// End of specialization parameters.  Two helper routines for conversion to and
// from the representation of floating-point data as integer values follow.

static inline src_rep_t srcToRep(src_t x) {
    const union { src_t f; src_rep_t i; } rep = {.f = x};
    return rep.i;
}

static inline dst_t dstFromRep(dst_rep_t x) {
    const union { dst_t f; dst_rep_t i; } rep = {.i = x};
    return rep.f;
}

// End helper routines.  Conversion implementation follows.

ARM_EABI_FNALIAS(f2d, extendsfdf2)

dst_t __extendsfdf2(src_t a) {
    
    // Various constants whose values follow from the type parameters.
    // Any reasonable optimizer will fold and propagate all of these.
    const int srcBits = sizeof(src_t)*CHAR_BIT;
    const int srcExpBits = srcBits - srcSigBits - 1;
    const int srcInfExp = (1 << srcExpBits) - 1;
    const int srcExpBias = srcInfExp >> 1;
    
    const src_rep_t srcMinNormal = SRC_REP_C(1) << srcSigBits;
    const src_rep_t srcInfinity = (src_rep_t)srcInfExp << srcSigBits;
    const src_rep_t srcSignMask = SRC_REP_C(1) << (srcSigBits + srcExpBits);
    const src_rep_t srcAbsMask = srcSignMask - 1;
    const src_rep_t srcQNaN = SRC_REP_C(1) << (srcSigBits - 1);
    const src_rep_t srcNaNCode = srcQNaN - 1;
    
    const int dstBits = sizeof(dst_t)*CHAR_BIT;
    const int dstExpBits = dstBits - dstSigBits - 1;
    const int dstInfExp = (1 << dstExpBits) - 1;
    const int dstExpBias = dstInfExp >> 1;
    
    const dst_rep_t dstMinNormal = DST_REP_C(1) << dstSigBits;
    
    // Break a into a sign and representation of the absolute value
    const src_rep_t aRep = srcToRep(a);
    const src_rep_t aAbs = aRep & srcAbsMask;
    const src_rep_t sign = aRep & srcSignMask;
    dst_rep_t absResult;
    
    if (aAbs - srcMinNormal < srcInfinity - srcMinNormal) {
        // a is a normal number.
        // Extend to the destination type by shifting the significand and
        // exponent into the proper position and rebiasing the exponent.
        absResult = (dst_rep_t)aAbs << (dstSigBits - srcSigBits);
        absResult += (dst_rep_t)(dstExpBias - srcExpBias) << dstSigBits;
    }
    
    else if (aAbs >= srcInfinity) {
        // a is NaN or infinity.
        // Conjure the result by beginning with infinity, then setting the qNaN
        // bit (if needed) and right-aligning the rest of the trailing NaN
        // payload field.
        absResult = (dst_rep_t)dstInfExp << dstSigBits;
        absResult |= (dst_rep_t)(aAbs & srcQNaN) << (dstSigBits - srcSigBits);
        absResult |= aAbs & srcNaNCode;
    }
    
    else if (aAbs) {
        // a is denormal.
        // renormalize the significand and clear the leading bit, then insert
        // the correct adjusted exponent in the destination type.
        const int scale = src_rep_t_clz(aAbs) - src_rep_t_clz(srcMinNormal);
        absResult = (dst_rep_t)aAbs << (dstSigBits - srcSigBits + scale);
        absResult ^= dstMinNormal;
        const int resultExponent = dstExpBias - srcExpBias - scale + 1;
        absResult |= (dst_rep_t)resultExponent << dstSigBits;
    }

    else {
        // a is zero.
        absResult = 0;
    }
    
    // Apply the signbit to (dst_t)abs(a).
    const dst_rep_t result = absResult | (dst_rep_t)sign << (dstBits - srcBits);
    return dstFromRep(result);
}