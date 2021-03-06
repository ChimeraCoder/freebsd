
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

static const uint32_t
exp_ovfl  = 0x40862e42,			/* high bits of MAX_EXP * ln2 ~= 710 */
cexp_ovfl = 0x4096b8e4;			/* (MAX_EXP - MIN_DENORM_EXP) * ln2 */

double complex
cexp(double complex z)
{
	double x, y, exp_x;
	uint32_t hx, hy, lx, ly;

	x = creal(z);
	y = cimag(z);

	EXTRACT_WORDS(hy, ly, y);
	hy &= 0x7fffffff;

	/* cexp(x + I 0) = exp(x) + I 0 */
	if ((hy | ly) == 0)
		return (cpack(exp(x), y));
	EXTRACT_WORDS(hx, lx, x);
	/* cexp(0 + I y) = cos(y) + I sin(y) */
	if (((hx & 0x7fffffff) | lx) == 0)
		return (cpack(cos(y), sin(y)));

	if (hy >= 0x7ff00000) {
		if (lx != 0 || (hx & 0x7fffffff) != 0x7ff00000) {
			/* cexp(finite|NaN +- I Inf|NaN) = NaN + I NaN */
			return (cpack(y - y, y - y));
		} else if (hx & 0x80000000) {
			/* cexp(-Inf +- I Inf|NaN) = 0 + I 0 */
			return (cpack(0.0, 0.0));
		} else {
			/* cexp(+Inf +- I Inf|NaN) = Inf + I NaN */
			return (cpack(x, y - y));
		}
	}

	if (hx >= exp_ovfl && hx <= cexp_ovfl) {
		/*
		 * x is between 709.7 and 1454.3, so we must scale to avoid
		 * overflow in exp(x).
		 */
		return (__ldexp_cexp(z, 0));
	} else {
		/*
		 * Cases covered here:
		 *  -  x < exp_ovfl and exp(x) won't overflow (common case)
		 *  -  x > cexp_ovfl, so exp(x) * s overflows for all s > 0
		 *  -  x = +-Inf (generated by exp())
		 *  -  x = NaN (spurious inexact exception from y)
		 */
		exp_x = exp(x);
		return (cpack(exp_x * cos(y), exp_x * sin(y)));
	}
}