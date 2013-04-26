
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
 * Tests for fmax{,f,l}() and fmin{,f,l}.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#define	ALL_STD_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | \
			 FE_OVERFLOW | FE_UNDERFLOW)

#pragma STDC FENV_ACCESS ON

/*
 * Test for equality with two special rules:
 *   fpequal(NaN, NaN) is true
 *   fpequal(+0.0, -0.0) is false
 */
static inline int
fpequal(long double x, long double y)
{

	return ((x == y && !signbit(x) == !signbit(y))
		|| (isnan(x) && isnan(y)));
}

/*
 * Test whether func(x, y) has the expected result, and make sure no
 * exceptions are raised.
 */
#define	TEST(func, type, x, y, expected) do {				      \
	type __x = (x);	/* convert before we clear exceptions */	      \
	type __y = (y);							      \
	feclearexcept(ALL_STD_EXCEPT);					      \
	long double __result = func((__x), (__y));			      \
	if (fetestexcept(ALL_STD_EXCEPT)) {				      \
		fprintf(stderr, #func "(%.20Lg, %.20Lg) raised 0x%x\n",	      \
			(x), (y), fetestexcept(FE_ALL_EXCEPT));		      \
		ok = 0;							      \
	}								      \
	if (!fpequal(__result, (expected)))	{			      \
		fprintf(stderr, #func "(%.20Lg, %.20Lg) = %.20Lg, "	      \
			"expected %.20Lg\n", (x), (y), __result, (expected)); \
		ok = 0;							      \
	}								      \
} while (0)

int
testall_r(long double big, long double small)
{
	int ok;

	long double expected_max = isnan(big) ? small : big;
	long double expected_min = isnan(small) ? big : small;
	ok = 1;

	TEST(fmaxf, float, big, small, expected_max);
	TEST(fmaxf, float, small, big, expected_max);
	TEST(fmax, double, big, small, expected_max);
	TEST(fmax, double, small, big, expected_max);
	TEST(fmaxl, long double, big, small, expected_max);
	TEST(fmaxl, long double, small, big, expected_max);
	TEST(fminf, float, big, small, expected_min);
	TEST(fminf, float, small, big, expected_min);
	TEST(fmin, double, big, small, expected_min);
	TEST(fmin, double, small, big, expected_min);
	TEST(fminl, long double, big, small, expected_min);
	TEST(fminl, long double, small, big, expected_min);

	return (ok);
}

/*
 * Test all the functions: fmaxf, fmax, fmaxl, fminf, fmin, and fminl,
 * in all rounding modes and with the arguments in different orders.
 * The input 'big' must be >= 'small'.
 */
void
testall(int testnum, long double big, long double small)
{
	static const int rmodes[] = {
		FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO
	};
	int i;

	for (i = 0; i < 4; i++) {
		fesetround(rmodes[i]);
		if (!testall_r(big, small)) {
			fprintf(stderr, "FAILURE in rounding mode %d\n",
				rmodes[i]);
			break;
		}
	}
	printf("%sok %d - big = %.20Lg, small = %.20Lg\n",
	       (i == 4) ? "" : "not ", testnum, big, small);
}

int
main(int argc, char *argv[])
{

	printf("1..12\n");

	testall(1, 1.0, 0.0);
	testall(2, 42.0, nextafterf(42.0, -INFINITY));
	testall(3, nextafterf(42.0, INFINITY), 42.0);
	testall(4, -5.0, -5.0);
	testall(5, -3.0, -4.0);
	testall(6, 1.0, NAN);
	testall(7, INFINITY, NAN);
	testall(8, INFINITY, 1.0);
	testall(9, -3.0, -INFINITY);
	testall(10, 3.0, -INFINITY);
	testall(11, NAN, NAN);

	/* This test isn't strictly required to work by C99. */
	testall(12, 0.0, -0.0);

	return (0);
}