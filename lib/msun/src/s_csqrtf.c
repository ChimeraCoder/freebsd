
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

float complex
csqrtf(float complex z)
{
	float a = crealf(z), b = cimagf(z);
	double t;

	/* Handle special cases. */
	if (z == 0)
		return (cpackf(0, b));
	if (isinf(b))
		return (cpackf(INFINITY, b));
	if (isnan(a)) {
		t = (b - b) / (b - b);	/* raise invalid if b is not a NaN */
		return (cpackf(a, t));	/* return NaN + NaN i */
	}
	if (isinf(a)) {
		/*
		 * csqrtf(inf + NaN i)  = inf +  NaN i
		 * csqrtf(inf + y i)    = inf +  0 i
		 * csqrtf(-inf + NaN i) = NaN +- inf i
		 * csqrtf(-inf + y i)   = 0   +  inf i
		 */
		if (signbit(a))
			return (cpackf(fabsf(b - b), copysignf(a, b)));
		else
			return (cpackf(a, copysignf(b - b, b)));
	}
	/*
	 * The remaining special case (b is NaN) is handled just fine by
	 * the normal code path below.
	 */

	/*
	 * We compute t in double precision to avoid overflow and to
	 * provide correct rounding in nearly all cases.
	 * This is Algorithm 312, CACM vol 10, Oct 1967.
	 */
	if (a >= 0) {
		t = sqrt((a + hypot(a, b)) * 0.5);
		return (cpackf(t, b / (2.0 * t)));
	} else {
		t = sqrt((-a + hypot(a, b)) * 0.5);
		return (cpackf(fabsf(b) / (2.0 * t), copysignf(t, b)));
	}
}