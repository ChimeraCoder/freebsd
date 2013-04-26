
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

#include <fenv.h>
#include <float.h>

#include "fpmath.h"
#include "math.h"

/* Return (x + ulp) for normal positive x. Assumes no overflow. */
static inline long double
inc(long double x)
{
	union IEEEl2bits u;

	u.e = x;
	if (++u.bits.manl == 0) {
		if (++u.bits.manh == 0) {
			u.bits.exp++;
			u.bits.manh |= LDBL_NBIT;
		}
	}
	return (u.e);
}

/* Return (x - ulp) for normal positive x. Assumes no underflow. */
static inline long double
dec(long double x)
{
	union IEEEl2bits u;

	u.e = x;
	if (u.bits.manl-- == 0) {
		if (u.bits.manh-- == LDBL_NBIT) {
			u.bits.exp--;
			u.bits.manh |= LDBL_NBIT;
		}
	}
	return (u.e);
}

#pragma STDC FENV_ACCESS ON

/*
 * This is slow, but simple and portable. You should use hardware sqrt
 * if possible.
 */

long double
sqrtl(long double x)
{
	union IEEEl2bits u;
	int k, r;
	long double lo, xn;
	fenv_t env;

	u.e = x;

	/* If x = NaN, then sqrt(x) = NaN. */
	/* If x = Inf, then sqrt(x) = Inf. */
	/* If x = -Inf, then sqrt(x) = NaN. */
	if (u.bits.exp == LDBL_MAX_EXP * 2 - 1)
		return (x * x + x);

	/* If x = +-0, then sqrt(x) = +-0. */
	if ((u.bits.manh | u.bits.manl | u.bits.exp) == 0)
		return (x);

	/* If x < 0, then raise invalid and return NaN */
	if (u.bits.sign)
		return ((x - x) / (x - x));

	feholdexcept(&env);

	if (u.bits.exp == 0) {
		/* Adjust subnormal numbers. */
		u.e *= 0x1.0p514;
		k = -514;
	} else {
		k = 0;
	}
	/*
	 * u.e is a normal number, so break it into u.e = e*2^n where
	 * u.e = (2*e)*2^2k for odd n and u.e = (4*e)*2^2k for even n.
	 */
	if ((u.bits.exp - 0x3ffe) & 1) {	/* n is odd.     */
		k += u.bits.exp - 0x3fff;	/* 2k = n - 1.   */
		u.bits.exp = 0x3fff;		/* u.e in [1,2). */
	} else {
		k += u.bits.exp - 0x4000;	/* 2k = n - 2.   */
		u.bits.exp = 0x4000;		/* u.e in [2,4). */
	}

	/*
	 * Newton's iteration.
	 * Split u.e into a high and low part to achieve additional precision.
	 */
	xn = sqrt(u.e);			/* 53-bit estimate of sqrtl(x). */
#if LDBL_MANT_DIG > 100
	xn = (xn + (u.e / xn)) * 0.5;	/* 106-bit estimate. */
#endif
	lo = u.e;
	u.bits.manl = 0;		/* Zero out lower bits. */
	lo = (lo - u.e) / xn;		/* Low bits divided by xn. */
	xn = xn + (u.e / xn);		/* High portion of estimate. */
	u.e = xn + lo;			/* Combine everything. */
	u.bits.exp += (k >> 1) - 1;

	feclearexcept(FE_INEXACT);
	r = fegetround();
	fesetround(FE_TOWARDZERO);	/* Set to round-toward-zero. */
	xn = x / u.e;			/* Chopped quotient (inexact?). */

	if (!fetestexcept(FE_INEXACT)) { /* Quotient is exact. */
		if (xn == u.e) {
			fesetenv(&env);
			return (u.e);
		}
		/* Round correctly for inputs like x = y**2 - ulp. */
		xn = dec(xn);		/* xn = xn - ulp. */
	}

	if (r == FE_TONEAREST) {
		xn = inc(xn);		/* xn = xn + ulp. */
	} else if (r == FE_UPWARD) {
		u.e = inc(u.e);		/* u.e = u.e + ulp. */
		xn = inc(xn);		/* xn  = xn + ulp. */
	}
	u.e = u.e + xn;				/* Chopped sum. */
	feupdateenv(&env);	/* Restore env and raise inexact */
	u.bits.exp--;
	return (u.e);
}