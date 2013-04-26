
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
 * Hyperbolic sine of a complex argument z.  See s_csinh.c for details.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <complex.h>
#include <math.h>

#include "math_private.h"

static const float huge = 0x1p127;

float complex
csinhf(float complex z)
{
	float x, y, h;
	int32_t hx, hy, ix, iy;

	x = crealf(z);
	y = cimagf(z);

	GET_FLOAT_WORD(hx, x);
	GET_FLOAT_WORD(hy, y);

	ix = 0x7fffffff & hx;
	iy = 0x7fffffff & hy;

	if (ix < 0x7f800000 && iy < 0x7f800000) {
		if (iy == 0)
			return (cpackf(sinhf(x), y));
		if (ix < 0x41100000)	/* small x: normal case */
			return (cpackf(sinhf(x) * cosf(y), coshf(x) * sinf(y)));

		/* |x| >= 9, so cosh(x) ~= exp(|x|) */
		if (ix < 0x42b17218) {
			/* x < 88.7: expf(|x|) won't overflow */
			h = expf(fabsf(x)) * 0.5f;
			return (cpackf(copysignf(h, x) * cosf(y), h * sinf(y)));
		} else if (ix < 0x4340b1e7) {
			/* x < 192.7: scale to avoid overflow */
			z = __ldexp_cexpf(cpackf(fabsf(x), y), -1);
			return (cpackf(crealf(z) * copysignf(1, x), cimagf(z)));
		} else {
			/* x >= 192.7: the result always overflows */
			h = huge * x;
			return (cpackf(h * cosf(y), h * h * sinf(y)));
		}
	}

	if (ix == 0 && iy >= 0x7f800000)
		return (cpackf(copysignf(0, x * (y - y)), y - y));

	if (iy == 0 && ix >= 0x7f800000) {
		if ((hx & 0x7fffff) == 0)
			return (cpackf(x, y));
		return (cpackf(x, copysignf(0, y)));
	}

	if (ix < 0x7f800000 && iy >= 0x7f800000)
		return (cpackf(y - y, x * (y - y)));

	if (ix >= 0x7f800000 && (hx & 0x7fffff) == 0) {
		if (iy >= 0x7f800000)
			return (cpackf(x * x, x * (y - y)));
		return (cpackf(x * cosf(y), INFINITY * sinf(y)));
	}

	return (cpackf((x * x) * (y - y), (x + x) * (y - y)));
}

float complex
csinf(float complex z)
{

	z = csinhf(cpackf(-cimagf(z), crealf(z)));
	return (cpackf(cimagf(z), -crealf(z)));
}