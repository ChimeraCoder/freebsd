
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
 * Test for remainder functions: remainder, remainderf, remainderl,
 * remquo, remquof, and remquol.
 * Missing tests: fmod, fmodf.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

static void test_invalid(long double, long double);
static void testl(long double, long double, long double, int);
static void testd(double, double, double, int);
static void testf(float, float, float, int);

#define	test(x, y, e_r, e_q) do {	\
	testl(x, y, e_r, e_q);		\
	testd(x, y, e_r, e_q);		\
	testf(x, y, e_r, e_q);		\
} while (0)

int
main(int argc, char *argv[])
{

	printf("1..3\n");

	test_invalid(0.0, 0.0);
	test_invalid(1.0, 0.0);
	test_invalid(INFINITY, 0.0);
	test_invalid(INFINITY, 1.0);
	test_invalid(-INFINITY, 1.0);
	test_invalid(NAN, 1.0);
	test_invalid(1.0, NAN);

	test(4, 4, 0, 1);
	test(0, 3.0, 0, 0);
	testd(0x1p-1074, 1, 0x1p-1074, 0x1p-1074);
	testf(0x1p-149, 1, 0x1p-149, 0x1p-149);
	test(3.0, 4, -1, 1);
	test(3.0, -4, -1, -1);
	testd(275 * 1193040, 275, 0, 1193040);
	test(4.5 * 7.5, 4.5, -2.25, 8);	/* we should get the even one */
	testf(0x1.9044f6p-1, 0x1.ce662ep-1, -0x1.f109cp-4, 1);
#if LDBL_MANT_DIG > 53
	testl(-0x1.23456789abcdefp-2000L, 0x1.fedcba987654321p-2000L,
	      0x1.b72ea61d950c862p-2001L, -1);
#endif

	printf("ok 1 - rem\n");

	/*
	 * The actual quotient here is 864062210.50000003..., but
	 * double-precision division gets -8.64062210.5, which rounds
	 * the wrong way.  This test ensures that remquo() is smart
	 * enough to get the low-order bit right.
	 */
	testd(-0x1.98260f22fc6dep-302, 0x1.fb3167c430a13p-332,
	    0x1.fb3165b82de72p-333, -864062211);
	/* Even harder cases with greater exponent separation */
	test(0x1.fp100, 0x1.ep-40, -0x1.cp-41, 143165577);
	testd(-0x1.abcdefp120, 0x1.87654321p-120,
	    -0x1.69c78ec4p-121, -63816414);

	printf("ok 2 - rem\n");

	test(0x1.66666cp+120, 0x1p+71, 0.0, 1476395008);
	testd(-0x1.0000000000003p+0, 0x1.0000000000003p+0, -0.0, -1);
	testl(-0x1.0000000000003p+0, 0x1.0000000000003p+0, -0.0, -1);
	testd(-0x1.0000000000001p-749, 0x1.4p-1072, 0x1p-1074, -1288490189);
	testl(-0x1.0000000000001p-749, 0x1.4p-1072, 0x1p-1074, -1288490189);

	printf("ok 3 - rem\n");

	return (0);
}

static void
test_invalid(long double x, long double y)
{
	int q;

	q = 0xdeadbeef;

	assert(isnan(remainder(x, y)));
	assert(isnan(remquo(x, y, &q)));
#ifdef STRICT
	assert(q == 0xdeadbeef);
#endif

	assert(isnan(remainderf(x, y)));
	assert(isnan(remquof(x, y, &q)));
#ifdef STRICT
	assert(q == 0xdeadbeef);
#endif

	assert(isnan(remainderl(x, y)));
	assert(isnan(remquol(x, y, &q)));
#ifdef STRICT
	assert(q == 0xdeadbeef);
#endif
}

/* 0x012345 ==> 0x01ffff */
static inline int
mask(int x)
{
	return ((unsigned)~0 >> (32 - fls(x)));
}

static void
testl(long double x, long double y, long double expected_rem, int expected_quo)
{
	int q;
	long double rem;

	q = random();
	rem = remainderl(x, y);
	assert(rem == expected_rem);
	assert(!signbit(rem) == !signbit(expected_rem));
	rem = remquol(x, y, &q);
	assert(rem == expected_rem);
	assert(!signbit(rem) == !signbit(expected_rem));
	assert((q ^ expected_quo) >= 0); /* sign(q) == sign(expected_quo) */
	assert((q & 0x7) == (expected_quo & 0x7));
	if (q != 0) {
		assert((q > 0) ^ !(expected_quo > 0));
		q = abs(q);
		assert(q == (abs(expected_quo) & mask(q)));
	}
}

static void
testd(double x, double y, double expected_rem, int expected_quo)
{
	int q;
	double rem;

	q = random();
	rem = remainder(x, y);
	assert(rem == expected_rem);
	assert(!signbit(rem) == !signbit(expected_rem));
	rem = remquo(x, y, &q);
	assert(rem == expected_rem);
	assert(!signbit(rem) == !signbit(expected_rem));
	assert((q ^ expected_quo) >= 0); /* sign(q) == sign(expected_quo) */
	assert((q & 0x7) == (expected_quo & 0x7));
	if (q != 0) {
		assert((q > 0) ^ !(expected_quo > 0));
		q = abs(q);
		assert(q == (abs(expected_quo) & mask(q)));
	}
}

static void
testf(float x, float y, float expected_rem, int expected_quo)
{
	int q;
	float rem;

	q = random();
	rem = remainderf(x, y);
	assert(rem == expected_rem);
	assert(!signbit(rem) == !signbit(expected_rem));
	rem = remquof(x, y, &q);
	assert(rem == expected_rem);
	assert(!signbit(rem) == !signbit(expected_rem));
	assert((q ^ expected_quo) >= 0); /* sign(q) == sign(expected_quo) */
	assert((q & 0x7) == (expected_quo & 0x7));
	if (q != 0) {
		assert((q > 0) ^ !(expected_quo > 0));
		q = abs(q);
		assert((q & mask(q)) == (abs(expected_quo) & mask(q)));
	}
}