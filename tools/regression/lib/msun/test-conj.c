
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
 * Tests for conj{,f,l}()
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <complex.h>
#include <fenv.h>
#include <math.h>
#include <stdio.h>

#pragma	STDC CX_LIMITED_RANGE	off

/* Make sure gcc doesn't use builtin versions of these or honor __pure2. */
static float complex (*libconjf)(float complex) = conjf;
static double complex (*libconj)(double complex) = conj;
static long double complex (*libconjl)(long double complex) = conjl;
static float (*libcrealf)(float complex) = crealf;
static double (*libcreal)(double complex) = creal;
static long double (*libcreall)(long double complex) = creall;
static float (*libcimagf)(float complex) = cimagf;
static double (*libcimag)(double complex) = cimag;
static long double (*libcimagl)(long double complex) = cimagl;

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

static int
cfpequal(long double complex d1, long double complex d2)
{

	return (fpequal(creall(d1), creall(d2)) &&
		fpequal(cimagl(d1), cimagl(d2)));
}

static const double tests[] = {
	/* a +  bI */
	0.0,	0.0,
	0.0,	1.0,
	1.0,	0.0,
	-1.0,	0.0,
	1.0,	-0.0,
	0.0,	-1.0,
	2.0,	4.0,
	0.0,	INFINITY,
	0.0,	-INFINITY,
	INFINITY, 0.0,
	NAN,	1.0,
	1.0,	NAN,
	NAN,	NAN,
	-INFINITY, INFINITY,
};

int
main(int argc, char *argv[])
{
	static const int ntests = sizeof(tests) / sizeof(tests[0]) / 2;
	complex float in;
	complex long double expected;
	int i;

	printf("1..%d\n", ntests * 3);

	for (i = 0; i < ntests; i++) {
		__real__ expected = __real__ in = tests[2 * i];
		__imag__ in = tests[2 * i + 1];
		__imag__ expected = -cimag(in);

		assert(fpequal(libcrealf(in), __real__ in));
		assert(fpequal(libcreal(in), __real__ in));
		assert(fpequal(libcreall(in), __real__ in));
		assert(fpequal(libcimagf(in), __imag__ in));
		assert(fpequal(libcimag(in), __imag__ in));
		assert(fpequal(libcimagl(in), __imag__ in));		

		feclearexcept(FE_ALL_EXCEPT);
		if (!cfpequal(libconjf(in), expected)) {
			printf("not ok %d\t# conjf(%#.2g + %#.2gI): "
			       "wrong value\n",
			       3 * i + 1, creal(in), cimag(in));
		} else if (fetestexcept(FE_ALL_EXCEPT)) {
			printf("not ok %d\t# conjf(%#.2g + %#.2gI): "
			       "threw an exception\n",
			       3 * i + 1, creal(in), cimag(in));
		} else {
			printf("ok %d\t\t# conjf(%#.2g + %#.2gI)\n",
			       3 * i + 1, creal(in), cimag(in));
		}

		feclearexcept(FE_ALL_EXCEPT);
		if (!cfpequal(libconj(in), expected)) {
			printf("not ok %d\t# conj(%#.2g + %#.2gI): "
			       "wrong value\n",
			       3 * i + 2, creal(in), cimag(in));
		} else if (fetestexcept(FE_ALL_EXCEPT)) {
			printf("not ok %d\t# conj(%#.2g + %#.2gI): "
			       "threw an exception\n",
			       3 * i + 2, creal(in), cimag(in));
		} else {
			printf("ok %d\t\t# conj(%#.2g + %#.2gI)\n",
			       3 * i + 2, creal(in), cimag(in));
		}

		feclearexcept(FE_ALL_EXCEPT);
		if (!cfpequal(libconjl(in), expected)) {
			printf("not ok %d\t# conjl(%#.2g + %#.2gI): "
			       "wrong value\n",
			       3 * i + 3, creal(in), cimag(in));
		} else if (fetestexcept(FE_ALL_EXCEPT)) {
			printf("not ok %d\t# conjl(%#.2g + %#.2gI): "
			       "threw an exception\n",
			       3 * i + 3, creal(in), cimag(in));
		} else {
			printf("ok %d\t\t# conjl(%#.2g + %#.2gI)\n",
			       3 * i + 3, creal(in), cimag(in));
		}
	}

	return (0);
}