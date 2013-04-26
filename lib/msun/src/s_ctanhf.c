
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
 * Hyperbolic tangent of a complex argument z.  See s_ctanh.c for details.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <complex.h>
#include <math.h>

#include "math_private.h"

float complex
ctanhf(float complex z)
{
	float x, y;
	float t, beta, s, rho, denom;
	uint32_t hx, ix;

	x = crealf(z);
	y = cimagf(z);

	GET_FLOAT_WORD(hx, x);
	ix = hx & 0x7fffffff;

	if (ix >= 0x7f800000) {
		if (ix & 0x7fffff)
			return (cpackf(x, (y == 0 ? y : x * y)));
		SET_FLOAT_WORD(x, hx - 0x40000000);
		return (cpackf(x,
		    copysignf(0, isinf(y) ? y : sinf(y) * cosf(y))));
	}

	if (!isfinite(y))
		return (cpackf(y - y, y - y));

	if (ix >= 0x41300000) {	/* x >= 11 */
		float exp_mx = expf(-fabsf(x));
		return (cpackf(copysignf(1, x),
		    4 * sinf(y) * cosf(y) * exp_mx * exp_mx));
	}

	t = tanf(y);
	beta = 1.0 + t * t;
	s = sinhf(x);
	rho = sqrtf(1 + s * s);
	denom = 1 + beta * s * s;
	return (cpackf((beta * rho * s) / denom, t / denom));
}

float complex
ctanf(float complex z)
{

	z = ctanhf(cpackf(-cimagf(z), crealf(z)));
	return (cpackf(cimagf(z), -crealf(z)));
}