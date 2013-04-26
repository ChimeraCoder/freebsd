
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
 * Test for lround(), lroundf(), llround(), and llroundf().
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <fenv.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

/*
 * XXX The volatile here is to avoid gcc's bogus constant folding and work
 *     around the lack of support for the FENV_ACCESS pragma.
 */
#define	test(func, x, result, excepts)	do {				\
	volatile double _d = x;						\
	assert(feclearexcept(FE_ALL_EXCEPT) == 0);			\
	assert((func)(_d) == (result) || fetestexcept(FE_INVALID));	\
	assert(fetestexcept(FE_ALL_EXCEPT) == (excepts));		\
} while (0)

#define	testall(x, result, excepts)	do {				\
	test(lround, x, result, excepts);				\
	test(lroundf, x, result, excepts);				\
	test(llround, x, result, excepts);				\
	test(llroundf, x, result, excepts);				\
} while (0)

#define	IGNORE	0

#pragma STDC FENV_ACCESS ON

int
main(int argc, char *argv[])
{

	printf("1..1\n");

	testall(0.0, 0, 0);
	testall(0.25, 0, FE_INEXACT);
	testall(0.5, 1, FE_INEXACT);
	testall(-0.5, -1, FE_INEXACT);
	testall(1.0, 1, 0);
	testall(0x12345000p0, 0x12345000, 0);
	testall(0x1234.fp0, 0x1235, FE_INEXACT);
	testall(INFINITY, IGNORE, FE_INVALID);
	testall(NAN, IGNORE, FE_INVALID);

#if (LONG_MAX == 0x7fffffffl)
	test(lround, 0x7fffffff.8p0, IGNORE, FE_INVALID);
	test(lround, -0x80000000.8p0, IGNORE, FE_INVALID);
	test(lround, 0x80000000.0p0, IGNORE, FE_INVALID);
	test(lround, 0x7fffffff.4p0, 0x7fffffffl, FE_INEXACT);
	test(lround, -0x80000000.4p0, -0x80000000l, FE_INEXACT);
	test(lroundf, 0x80000000.0p0f, IGNORE, FE_INVALID);
	test(lroundf, 0x7fffff80.0p0f, 0x7fffff80l, 0);
#elif (LONG_MAX == 0x7fffffffffffffffll)
	test(lround, 0x8000000000000000.0p0, IGNORE, FE_INVALID);
	test(lroundf, 0x8000000000000000.0p0f, IGNORE, FE_INVALID);
	test(lround, 0x7ffffffffffffc00.0p0, 0x7ffffffffffffc00l, 0);
	test(lroundf, 0x7fffff8000000000.0p0f, 0x7fffff8000000000l, 0);
	test(lround, -0x8000000000000800.0p0, IGNORE, FE_INVALID);
	test(lroundf, -0x8000010000000000.0p0f, IGNORE, FE_INVALID);
	test(lround, -0x8000000000000000.0p0, -0x8000000000000000l, 0);
	test(lroundf, -0x8000000000000000.0p0f, -0x8000000000000000l, 0);
#else
#error "Unsupported long size"
#endif

#if (LLONG_MAX == 0x7fffffffffffffffLL)
	test(llround, 0x8000000000000000.0p0, IGNORE, FE_INVALID);
	test(llroundf, 0x8000000000000000.0p0f, IGNORE, FE_INVALID);
	test(llround, 0x7ffffffffffffc00.0p0, 0x7ffffffffffffc00ll, 0);
	test(llroundf, 0x7fffff8000000000.0p0f, 0x7fffff8000000000ll, 0);
	test(llround, -0x8000000000000800.0p0, IGNORE, FE_INVALID);
	test(llroundf, -0x8000010000000000.0p0f, IGNORE, FE_INVALID);
	test(llround, -0x8000000000000000.0p0, -0x8000000000000000ll, 0);
	test(llroundf, -0x8000000000000000.0p0f, -0x8000000000000000ll, 0);
#else
#error "Unsupported long long size"
#endif

	printf("ok 1 - lround\n");

	return (0);
}