
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
 * Hyperbolic sine of a complex argument z = x + i y.
 *
 * sinh(z) = sinh(x+iy)
 *         = sinh(x) cos(y) + i cosh(x) sin(y).
 *
 * Exceptional values are noted in the comments within the source code.
 * These values and the return value were taken from n1124.pdf.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <complex.h>
#include <math.h>

#include "math_private.h"

static const double huge = 0x1p1023;

double complex
csinh(double complex z)
{
	double x, y, h;
	int32_t hx, hy, ix, iy, lx, ly;

	x = creal(z);
	y = cimag(z);

	EXTRACT_WORDS(hx, lx, x);
	EXTRACT_WORDS(hy, ly, y);

	ix = 0x7fffffff & hx;
	iy = 0x7fffffff & hy;

	/* Handle the nearly-non-exceptional cases where x and y are finite. */
	if (ix < 0x7ff00000 && iy < 0x7ff00000) {
		if ((iy | ly) == 0)
			return (cpack(sinh(x), y));
		if (ix < 0x40360000)	/* small x: normal case */
			return (cpack(sinh(x) * cos(y), cosh(x) * sin(y)));

		/* |x| >= 22, so cosh(x) ~= exp(|x|) */
		if (ix < 0x40862e42) {
			/* x < 710: exp(|x|) won't overflow */
			h = exp(fabs(x)) * 0.5;
			return (cpack(copysign(h, x) * cos(y), h * sin(y)));
		} else if (ix < 0x4096bbaa) {
			/* x < 1455: scale to avoid overflow */
			z = __ldexp_cexp(cpack(fabs(x), y), -1);
			return (cpack(creal(z) * copysign(1, x), cimag(z)));
		} else {
			/* x >= 1455: the result always overflows */
			h = huge * x;
			return (cpack(h * cos(y), h * h * sin(y)));
		}
	}

	/*
	 * sinh(+-0 +- I Inf) = sign(d(+-0, dNaN))0 + I dNaN.
	 * The sign of 0 in the result is unspecified.  Choice = normally
	 * the same as dNaN.  Raise the invalid floating-point exception.
	 *
	 * sinh(+-0 +- I NaN) = sign(d(+-0, NaN))0 + I d(NaN).
	 * The sign of 0 in the result is unspecified.  Choice = normally
	 * the same as d(NaN).
	 */
	if ((ix | lx) == 0 && iy >= 0x7ff00000)
		return (cpack(copysign(0, x * (y - y)), y - y));

	/*
	 * sinh(+-Inf +- I 0) = +-Inf + I +-0.
	 *
	 * sinh(NaN +- I 0)   = d(NaN) + I +-0.
	 */
	if ((iy | ly) == 0 && ix >= 0x7ff00000) {
		if (((hx & 0xfffff) | lx) == 0)
			return (cpack(x, y));
		return (cpack(x, copysign(0, y)));
	}

	/*
	 * sinh(x +- I Inf) = dNaN + I dNaN.
	 * Raise the invalid floating-point exception for finite nonzero x.
	 *
	 * sinh(x + I NaN) = d(NaN) + I d(NaN).
	 * Optionally raises the invalid floating-point exception for finite
	 * nonzero x.  Choice = don't raise (except for signaling NaNs).
	 */
	if (ix < 0x7ff00000 && iy >= 0x7ff00000)
		return (cpack(y - y, x * (y - y)));

	/*
	 * sinh(+-Inf + I NaN)  = +-Inf + I d(NaN).
	 * The sign of Inf in the result is unspecified.  Choice = normally
	 * the same as d(NaN).
	 *
	 * sinh(+-Inf +- I Inf) = +Inf + I dNaN.
	 * The sign of Inf in the result is unspecified.  Choice = always +.
	 * Raise the invalid floating-point exception.
	 *
	 * sinh(+-Inf + I y)   = +-Inf cos(y) + I Inf sin(y)
	 */
	if (ix >= 0x7ff00000 && ((hx & 0xfffff) | lx) == 0) {
		if (iy >= 0x7ff00000)
			return (cpack(x * x, x * (y - y)));
		return (cpack(x * cos(y), INFINITY * sin(y)));
	}

	/*
	 * sinh(NaN + I NaN)  = d(NaN) + I d(NaN).
	 *
	 * sinh(NaN +- I Inf) = d(NaN) + I d(NaN).
	 * Optionally raises the invalid floating-point exception.
	 * Choice = raise.
	 *
	 * sinh(NaN + I y)    = d(NaN) + I d(NaN).
	 * Optionally raises the invalid floating-point exception for finite
	 * nonzero y.  Choice = don't raise (except for signaling NaNs).
	 */
	return (cpack((x * x) * (y - y), (x + x) * (y - y)));
}

double complex
csin(double complex z)
{

	/* csin(z) = -I * csinh(I * z) */
	z = csinh(cpack(-cimag(z), creal(z)));
	return (cpack(cimag(z), -creal(z)));
}