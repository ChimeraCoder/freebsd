
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

/*
 * Perform an FPU multiply (return x * y).
 */

#include <sys/types.h>

#include <machine/frame.h>
#include <machine/fp.h>

#include "fpu_arith.h"
#include "fpu_emu.h"
#include "fpu_extern.h"

/*
 * The multiplication algorithm for normal numbers is as follows:
 *
 * The fraction of the product is built in the usual stepwise fashion.
 * Each step consists of shifting the accumulator right one bit
 * (maintaining any guard bits) and, if the next bit in y is set,
 * adding the multiplicand (x) to the accumulator.  Then, in any case,
 * we advance one bit leftward in y.  Algorithmically:
 *
 *	A = 0;
 *	for (bit = 0; bit < FP_NMANT; bit++) {
 *		sticky |= A & 1, A >>= 1;
 *		if (Y & (1 << bit))
 *			A += X;
 *	}
 *
 * (X and Y here represent the mantissas of x and y respectively.)
 * The resultant accumulator (A) is the product's mantissa.  It may
 * be as large as 11.11111... in binary and hence may need to be
 * shifted right, but at most one bit.
 *
 * Since we do not have efficient multiword arithmetic, we code the
 * accumulator as four separate words, just like any other mantissa.
 * We use local `register' variables in the hope that this is faster
 * than memory.  We keep x->fp_mant in locals for the same reason.
 *
 * In the algorithm above, the bits in y are inspected one at a time.
 * We will pick them up 32 at a time and then deal with those 32, one
 * at a time.  Note, however, that we know several things about y:
 *
 *    - the guard and round bits at the bottom are sure to be zero;
 *
 *    - often many low bits are zero (y is often from a single or double
 *	precision source);
 *
 *    - bit FP_NMANT-1 is set, and FP_1*2 fits in a word.
 *
 * We can also test for 32-zero-bits swiftly.  In this case, the center
 * part of the loop---setting sticky, shifting A, and not adding---will
 * run 32 times without adding X to A.  We can do a 32-bit shift faster
 * by simply moving words.  Since zeros are common, we optimize this case.
 * Furthermore, since A is initially zero, we can omit the shift as well
 * until we reach a nonzero word.
 */
struct fpn *
__fpu_mul(fe)
	struct fpemu *fe;
{
	struct fpn *x = &fe->fe_f1, *y = &fe->fe_f2;
	u_int a3, a2, a1, a0, x3, x2, x1, x0, bit, m;
	int sticky;
	FPU_DECL_CARRY

	/*
	 * Put the `heavier' operand on the right (see fpu_emu.h).
	 * Then we will have one of the following cases, taken in the
	 * following order:
	 *
	 *  - y = NaN.  Implied: if only one is a signalling NaN, y is.
	 *	The result is y.
	 *  - y = Inf.  Implied: x != NaN (is 0, number, or Inf: the NaN
	 *    case was taken care of earlier).
	 *	If x = 0, the result is NaN.  Otherwise the result
	 *	is y, with its sign reversed if x is negative.
	 *  - x = 0.  Implied: y is 0 or number.
	 *	The result is 0 (with XORed sign as usual).
	 *  - other.  Implied: both x and y are numbers.
	 *	The result is x * y (XOR sign, multiply bits, add exponents).
	 */
	ORDER(x, y);
	if (ISNAN(y))
		return (y);
	if (ISINF(y)) {
		if (ISZERO(x))
			return (__fpu_newnan(fe));
		y->fp_sign ^= x->fp_sign;
		return (y);
	}
	if (ISZERO(x)) {
		x->fp_sign ^= y->fp_sign;
		return (x);
	}

	/*
	 * Setup.  In the code below, the mask `m' will hold the current
	 * mantissa byte from y.  The variable `bit' denotes the bit
	 * within m.  We also define some macros to deal with everything.
	 */
	x3 = x->fp_mant[3];
	x2 = x->fp_mant[2];
	x1 = x->fp_mant[1];
	x0 = x->fp_mant[0];
	sticky = a3 = a2 = a1 = a0 = 0;

#define	ADD	/* A += X */ \
	FPU_ADDS(a3, a3, x3); \
	FPU_ADDCS(a2, a2, x2); \
	FPU_ADDCS(a1, a1, x1); \
	FPU_ADDC(a0, a0, x0)

#define	SHR1	/* A >>= 1, with sticky */ \
	sticky |= a3 & 1, a3 = (a3 >> 1) | (a2 << 31), \
	a2 = (a2 >> 1) | (a1 << 31), a1 = (a1 >> 1) | (a0 << 31), a0 >>= 1

#define	SHR32	/* A >>= 32, with sticky */ \
	sticky |= a3, a3 = a2, a2 = a1, a1 = a0, a0 = 0

#define	STEP	/* each 1-bit step of the multiplication */ \
	SHR1; if (bit & m) { ADD; }; bit <<= 1

	/*
	 * We are ready to begin.  The multiply loop runs once for each
	 * of the four 32-bit words.  Some words, however, are special.
	 * As noted above, the low order bits of Y are often zero.  Even
	 * if not, the first loop can certainly skip the guard bits.
	 * The last word of y has its highest 1-bit in position FP_NMANT-1,
	 * so we stop the loop when we move past that bit.
	 */
	if ((m = y->fp_mant[3]) == 0) {
		/* SHR32; */			/* unneeded since A==0 */
	} else {
		bit = 1 << FP_NG;
		do {
			STEP;
		} while (bit != 0);
	}
	if ((m = y->fp_mant[2]) == 0) {
		SHR32;
	} else {
		bit = 1;
		do {
			STEP;
		} while (bit != 0);
	}
	if ((m = y->fp_mant[1]) == 0) {
		SHR32;
	} else {
		bit = 1;
		do {
			STEP;
		} while (bit != 0);
	}
	m = y->fp_mant[0];		/* definitely != 0 */
	bit = 1;
	do {
		STEP;
	} while (bit <= m);

	/*
	 * Done with mantissa calculation.  Get exponent and handle
	 * 11.111...1 case, then put result in place.  We reuse x since
	 * it already has the right class (FP_NUM).
	 */
	m = x->fp_exp + y->fp_exp;
	if (a0 >= FP_2) {
		SHR1;
		m++;
	}
	x->fp_sign ^= y->fp_sign;
	x->fp_exp = m;
	x->fp_sticky = sticky;
	x->fp_mant[3] = a3;
	x->fp_mant[2] = a2;
	x->fp_mant[1] = a1;
	x->fp_mant[0] = a0;
	return (x);
}