
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

#include <float.h>
#ifdef __i386__
#include <ieeefp.h>
#endif

#include "math.h"
#include "math_private.h"
#if LDBL_MANT_DIG == 64
#include "../ld80/e_rem_pio2l.h"
#elif LDBL_MANT_DIG == 113
#include "../ld128/e_rem_pio2l.h"
#else
#error "Unsupported long double format"
#endif

long double
sinl(long double x)
{
	union IEEEl2bits z;
	int e0, s;
	long double y[2];
	long double hi, lo;

	z.e = x;
	s = z.bits.sign;
	z.bits.sign = 0;

	/* If x = +-0 or x is a subnormal number, then sin(x) = x */
	if (z.bits.exp == 0)
		return (x);

	/* If x = NaN or Inf, then sin(x) = NaN. */
	if (z.bits.exp == 32767)
		return ((x - x) / (x - x));

	ENTERI();

	/* Optimize the case where x is already within range. */
	if (z.e < M_PI_4) {
		hi = __kernel_sinl(z.e, 0, 0);
		RETURNI(s ? -hi : hi);
	}

	e0 = __ieee754_rem_pio2l(x, y);
	hi = y[0];
	lo = y[1];

	switch (e0 & 3) {
	case 0:
	    hi = __kernel_sinl(hi, lo, 1);
	    break;
	case 1:
	    hi = __kernel_cosl(hi, lo);
	    break;
	case 2:
	    hi = - __kernel_sinl(hi, lo, 1);
	    break;
	case 3:
	    hi = - __kernel_cosl(hi, lo);
	    break;
	}
	
	RETURNI(hi);
}