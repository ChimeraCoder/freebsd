
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

#include <complex.h>
#include <float.h>
#include <math.h>

#include "math_private.h"

/*
 * gcc doesn't implement complex multiplication or division correctly,
 * so we need to handle infinities specially. We turn on this pragma to
 * notify conforming c99 compilers that the fast-but-incorrect code that
 * gcc generates is acceptable, since the special cases have already been
 * handled.
 */
#pragma	STDC CX_LIMITED_RANGE	ON

/* We risk spurious overflow for components >= LDBL_MAX / (1 + sqrt(2)). */
#define	THRESH	(LDBL_MAX / 2.414213562373095048801688724209698L)

long double complex
csqrtl(long double complex z)
{
	long double complex result;
	long double a, b;
	long double t;
	int scale;

	a = creall(z);
	b = cimagl(z);

	/* Handle special cases. */
	if (z == 0)
		return (cpackl(0, b));
	if (isinf(b))
		return (cpackl(INFINITY, b));
	if (isnan(a)) {
		t = (b - b) / (b - b);	/* raise invalid if b is not a NaN */
		return (cpackl(a, t));	/* return NaN + NaN i */
	}
	if (isinf(a)) {
		/*
		 * csqrt(inf + NaN i)  = inf +  NaN i
		 * csqrt(inf + y i)    = inf +  0 i
		 * csqrt(-inf + NaN i) = NaN +- inf i
		 * csqrt(-inf + y i)   = 0   +  inf i
		 */
		if (signbit(a))
			return (cpackl(fabsl(b - b), copysignl(a, b)));
		else
			return (cpackl(a, copysignl(b - b, b)));
	}
	/*
	 * The remaining special case (b is NaN) is handled just fine by
	 * the normal code path below.
	 */

	/* Scale to avoid overflow. */
	if (fabsl(a) >= THRESH || fabsl(b) >= THRESH) {
		a *= 0.25;
		b *= 0.25;
		scale = 1;
	} else {
		scale = 0;
	}

	/* Algorithm 312, CACM vol 10, Oct 1967. */
	if (a >= 0) {
		t = sqrtl((a + hypotl(a, b)) * 0.5);
		result = cpackl(t, b / (2 * t));
	} else {
		t = sqrtl((-a + hypotl(a, b)) * 0.5);
		result = cpackl(fabsl(b) / (2 * t), copysignl(t, b));
	}

	/* Rescale. */
	if (scale)
		return (result * 2);
	else
		return (result);
}