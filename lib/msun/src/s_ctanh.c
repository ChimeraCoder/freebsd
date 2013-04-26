
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
 * Hyperbolic tangent of a complex argument z = x + i y.
 *
 * The algorithm is from:
 *
 *   W. Kahan.  Branch Cuts for Complex Elementary Functions or Much
 *   Ado About Nothing's Sign Bit.  In The State of the Art in
 *   Numerical Analysis, pp. 165 ff.  Iserles and Powell, eds., 1987.
 *
 * Method:
 *
 *   Let t    = tan(x)
 *       beta = 1/cos^2(y)
 *       s    = sinh(x)
 *       rho  = cosh(x)
 *
 *   We have:
 *
 *   tanh(z) = sinh(z) / cosh(z)
 *
 *             sinh(x) cos(y) + i cosh(x) sin(y)
 *           = ---------------------------------
 *             cosh(x) cos(y) + i sinh(x) sin(y)
 *
 *             cosh(x) sinh(x) / cos^2(y) + i tan(y)
 *           = -------------------------------------
 *                    1 + sinh^2(x) / cos^2(y)
 *
 *             beta rho s + i t
 *           = ----------------
 *               1 + beta s^2
 *
 * Modifications:
 *
 *   I omitted the original algorithm's handling of overflow in tan(x) after
 *   verifying with nearpi.c that this can't happen in IEEE single or double
 *   precision.  I also handle large x differently.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <complex.h>
#include <math.h>

#include "math_private.h"

double complex
ctanh(double complex z)
{
	double x, y;
	double t, beta, s, rho, denom;
	uint32_t hx, ix, lx;

	x = creal(z);
	y = cimag(z);

	EXTRACT_WORDS(hx, lx, x);
	ix = hx & 0x7fffffff;

	/*
	 * ctanh(NaN + i 0) = NaN + i 0
	 *
	 * ctanh(NaN + i y) = NaN + i NaN		for y != 0
	 *
	 * The imaginary part has the sign of x*sin(2*y), but there's no
	 * special effort to get this right.
	 *
	 * ctanh(+-Inf +- i Inf) = +-1 +- 0
	 *
	 * ctanh(+-Inf + i y) = +-1 + 0 sin(2y)		for y finite
	 *
	 * The imaginary part of the sign is unspecified.  This special
	 * case is only needed to avoid a spurious invalid exception when
	 * y is infinite.
	 */
	if (ix >= 0x7ff00000) {
		if ((ix & 0xfffff) | lx)	/* x is NaN */
			return (cpack(x, (y == 0 ? y : x * y)));
		SET_HIGH_WORD(x, hx - 0x40000000);	/* x = copysign(1, x) */
		return (cpack(x, copysign(0, isinf(y) ? y : sin(y) * cos(y))));
	}

	/*
	 * ctanh(x + i NAN) = NaN + i NaN
	 * ctanh(x +- i Inf) = NaN + i NaN
	 */
	if (!isfinite(y))
		return (cpack(y - y, y - y));

	/*
	 * ctanh(+-huge + i +-y) ~= +-1 +- i 2sin(2y)/exp(2x), using the
	 * approximation sinh^2(huge) ~= exp(2*huge) / 4.
	 * We use a modified formula to avoid spurious overflow.
	 */
	if (ix >= 0x40360000) {	/* x >= 22 */
		double exp_mx = exp(-fabs(x));
		return (cpack(copysign(1, x),
		    4 * sin(y) * cos(y) * exp_mx * exp_mx));
	}

	/* Kahan's algorithm */
	t = tan(y);
	beta = 1.0 + t * t;	/* = 1 / cos^2(y) */
	s = sinh(x);
	rho = sqrt(1 + s * s);	/* = cosh(x) */
	denom = 1 + beta * s * s;
	return (cpack((beta * rho * s) / denom, t / denom));
}

double complex
ctan(double complex z)
{

	/* ctan(z) = -I * ctanh(I * z) */
	z = ctanh(cpack(-cimag(z), creal(z)));
	return (cpack(cimag(z), -creal(z)));
}