
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
 * Tests for nearbyint{,f,l}()
 *
 * TODO:
 * - adapt tests for rint(3)
 * - tests for harder values (more mantissa bits than float)
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <fenv.h>
#include <math.h>
#include <stdio.h>

#define	ALL_STD_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | \
			 FE_OVERFLOW | FE_UNDERFLOW)

static int testnum;

static const int rmodes[] = {
	FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO,
};

static const struct {
	float in;
	float out[3];	/* one answer per rounding mode except towardzero */
} tests[] = {
/* input	output (expected) */
    { 0.0,	{ 0.0, 0.0, 0.0 }},
    { 0.5,	{ 0.0, 0.0, 1.0 }},
    { M_PI,	{ 3.0, 3.0, 4.0 }},
    { 65536.5,	{ 65536, 65536, 65537 }},
    { INFINITY,	{ INFINITY, INFINITY, INFINITY }},
    { NAN,	{ NAN, NAN, NAN }},
};

static const int ntests = sizeof(tests) / sizeof(tests[0]);

/*
 * Compare d1 and d2 using special rules: NaN == NaN and +0 != -0.
 * Fail an assertion if they differ.
 */
static int
fpequal(long double d1, long double d2)
{

	if (d1 != d2)
		return (isnan(d1) && isnan(d2));
	return (copysignl(1.0, d1) == copysignl(1.0, d2));
}

/* Get the appropriate result for the current rounding mode. */
static float
get_output(int testindex, int rmodeindex, int negative)
{
	double out;

	if (negative) {	/* swap downwards and upwards if input is negative */
		if (rmodeindex == 1)
			rmodeindex = 2;
		else if (rmodeindex == 2)
			rmodeindex = 1;
	}
	if (rmodeindex == 3) /* FE_TOWARDZERO uses the value for downwards */
		rmodeindex = 1;
	out = tests[testindex].out[rmodeindex];
	return (negative ? -out : out);
}

static void
test_nearby(int testindex)
{
	float in, out;
	int i;

	for (i = 0; i < sizeof(rmodes) / sizeof(rmodes[0]); i++) {
		fesetround(rmodes[i]);
		feclearexcept(ALL_STD_EXCEPT);

		in = tests[testindex].in;
		out = get_output(testindex, i, 0);
		assert(fpequal(out, nearbyintf(in)));
		assert(fpequal(out, nearbyint(in)));
		assert(fpequal(out, nearbyintl(in)));
		assert(fetestexcept(ALL_STD_EXCEPT) == 0);

		in = -tests[testindex].in;
		out = get_output(testindex, i, 1);
		assert(fpequal(out, nearbyintf(in)));
		assert(fpequal(out, nearbyint(in)));
		assert(fpequal(out, nearbyintl(in)));
		assert(fetestexcept(ALL_STD_EXCEPT) == 0);
	}

	printf("ok %d\t\t# nearbyint(+%g)\n", testnum++, in);
}

static void
test_modf(int testindex)
{
	float in, out;
	float ipartf, ipart_expected;
	double ipart;
	long double ipartl;
	int i;

	for (i = 0; i < sizeof(rmodes) / sizeof(rmodes[0]); i++) {
		fesetround(rmodes[i]);
		feclearexcept(ALL_STD_EXCEPT);

		in = tests[testindex].in;
		ipart_expected = tests[testindex].out[1];
		out = copysignf(
		    isinf(ipart_expected) ? 0.0 : in - ipart_expected, in);
		ipartl = ipart = ipartf = 42.0;

		assert(fpequal(out, modff(in, &ipartf)));
		assert(fpequal(ipart_expected, ipartf));
		assert(fpequal(out, modf(in, &ipart)));
		assert(fpequal(ipart_expected, ipart));
		assert(fpequal(out, modfl(in, &ipartl)));
		assert(fpequal(ipart_expected, ipartl));
		assert(fetestexcept(ALL_STD_EXCEPT) == 0);

		in = -in;
		ipart_expected = -ipart_expected;
		out = -out;
		ipartl = ipart = ipartf = 42.0;
		assert(fpequal(out, modff(in, &ipartf)));
		assert(fpequal(ipart_expected, ipartf));
		assert(fpequal(out, modf(in, &ipart)));
		assert(fpequal(ipart_expected, ipart));
		assert(fpequal(out, modfl(in, &ipartl)));
		assert(fpequal(ipart_expected, ipartl));
		assert(fetestexcept(ALL_STD_EXCEPT) == 0);
	}

	printf("ok %d\t\t# modf(+%g)\n", testnum++, in);
}

int
main(int argc, char *argv[])
{
	int i;

	printf("1..%d\n", ntests * 2);
	testnum = 1;
	for (i = 0; i < ntests; i++) {
		test_nearby(i);
		test_modf(i);
	}

	return (0);
}