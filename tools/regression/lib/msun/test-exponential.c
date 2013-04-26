
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

/*
 * Tests for corner cases in exp*().
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#ifdef __i386__
#include <ieeefp.h>
#endif

#define	ALL_STD_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | \
			 FE_OVERFLOW | FE_UNDERFLOW)

#pragma STDC FENV_ACCESS ON

/*
 * Test that a function returns the correct value and sets the
 * exception flags correctly. The exceptmask specifies which
 * exceptions we should check. We need to be lenient for several
 * reasoons, but mainly because on some architectures it's impossible
 * to raise FE_OVERFLOW without raising FE_INEXACT.
 *
 * These are macros instead of functions so that assert provides more
 * meaningful error messages.
 *
 * XXX The volatile here is to avoid gcc's bogus constant folding and work
 *     around the lack of support for the FENV_ACCESS pragma.
 */
#define	test(func, x, result, exceptmask, excepts)	do {		\
	volatile long double _d = x;					\
	assert(feclearexcept(FE_ALL_EXCEPT) == 0);			\
	assert(fpequal((func)(_d), (result)));				 \
	assert(((func), fetestexcept(exceptmask) == (excepts)));	\
} while (0)

/* Test all the functions that compute b^x. */
#define	testall0(x, result, exceptmask, excepts)	do {		\
	test(exp, x, result, exceptmask, excepts);			\
	test(expf, x, result, exceptmask, excepts);			\
	test(exp2, x, result, exceptmask, excepts);			\
	test(exp2f, x, result, exceptmask, excepts);			\
	test(exp2l, x, result, exceptmask, excepts);			\
} while (0)

/* Test all the functions that compute b^x - 1. */
#define	testall1(x, result, exceptmask, excepts)	do {		\
	test(expm1, x, result, exceptmask, excepts);			\
	test(expm1f, x, result, exceptmask, excepts);			\
} while (0)

/*
 * Determine whether x and y are equal, with two special rules:
 *	+0.0 != -0.0
 *	 NaN == NaN
 */
int
fpequal(long double x, long double y)
{
	return ((x == y && !signbit(x) == !signbit(y)) || isnan(x) && isnan(y));
}

void
run_generic_tests(void)
{

	/* exp(0) == 1, no exceptions raised */
	testall0(0.0, 1.0, ALL_STD_EXCEPT, 0);
	testall1(0.0, 0.0, ALL_STD_EXCEPT, 0);
	testall0(-0.0, 1.0, ALL_STD_EXCEPT, 0);
	testall1(-0.0, -0.0, ALL_STD_EXCEPT, 0);

	/* exp(NaN) == NaN, no exceptions raised */
	testall0(NAN, NAN, ALL_STD_EXCEPT, 0);
	testall1(NAN, NAN, ALL_STD_EXCEPT, 0);

	/* exp(Inf) == Inf, no exceptions raised */
	testall0(INFINITY, INFINITY, ALL_STD_EXCEPT, 0);
	testall1(INFINITY, INFINITY, ALL_STD_EXCEPT, 0);

	/* exp(-Inf) == 0, no exceptions raised */
	testall0(-INFINITY, 0.0, ALL_STD_EXCEPT, 0);
	testall1(-INFINITY, -1.0, ALL_STD_EXCEPT, 0);

	/* exp(big) == Inf, overflow exception */
	testall0(50000.0, INFINITY, ALL_STD_EXCEPT & ~FE_INEXACT, FE_OVERFLOW);
	testall1(50000.0, INFINITY, ALL_STD_EXCEPT & ~FE_INEXACT, FE_OVERFLOW);

	/* exp(small) == 0, underflow and inexact exceptions */
	testall0(-50000.0, 0.0, ALL_STD_EXCEPT, FE_UNDERFLOW | FE_INEXACT);
	testall1(-50000.0, -1.0, ALL_STD_EXCEPT, FE_INEXACT);
}

void
run_exp2_tests(void)
{
	int i;

	/*
	 * We should insist that exp2() return exactly the correct
	 * result and not raise an inexact exception for integer
	 * arguments.
	 */
	feclearexcept(FE_ALL_EXCEPT);
	for (i = FLT_MIN_EXP - FLT_MANT_DIG; i < FLT_MAX_EXP; i++) {
		assert(exp2f(i) == ldexpf(1.0, i));
		assert(fetestexcept(ALL_STD_EXCEPT) == 0);
	}
	for (i = DBL_MIN_EXP - DBL_MANT_DIG; i < DBL_MAX_EXP; i++) {
		assert(exp2(i) == ldexp(1.0, i));
		assert(fetestexcept(ALL_STD_EXCEPT) == 0);
	}
	for (i = LDBL_MIN_EXP - LDBL_MANT_DIG; i < LDBL_MAX_EXP; i++) {
		assert(exp2l(i) == ldexpl(1.0, i));
		assert(fetestexcept(ALL_STD_EXCEPT) == 0);
	}
}

int
main(int argc, char *argv[])
{

	printf("1..3\n");

	run_generic_tests();
	printf("ok 1 - exponential\n");

#ifdef __i386__
	fpsetprec(FP_PE);
	run_generic_tests();
#endif
	printf("ok 2 - exponential\n");

	run_exp2_tests();
	printf("ok 3 - exponential\n");

	return (0);
}