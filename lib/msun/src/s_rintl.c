
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
#include <math.h>

#include "fpmath.h"

#if LDBL_MAX_EXP != 0x4000
/* We also require the usual bias, min exp and expsign packing. */
#error "Unsupported long double format"
#endif

#define	BIAS	(LDBL_MAX_EXP - 1)

static const float
shift[2] = {
#if LDBL_MANT_DIG == 64
	0x1.0p63, -0x1.0p63
#elif LDBL_MANT_DIG == 113
	0x1.0p112, -0x1.0p112
#else
#error "Unsupported long double format"
#endif
};
static const float zero[2] = { 0.0, -0.0 };

long double
rintl(long double x)
{
	union IEEEl2bits u;
	uint32_t expsign;
	int ex, sign;

	u.e = x;
	expsign = u.xbits.expsign;
	ex = expsign & 0x7fff;

	if (ex >= BIAS + LDBL_MANT_DIG - 1) {
		if (ex == BIAS + LDBL_MAX_EXP)
			return (x + x);	/* Inf, NaN, or unsupported format */
		return (x);		/* finite and already an integer */
	}
	sign = expsign >> 15;

	/*
	 * The following code assumes that intermediate results are
	 * evaluated in long double precision. If they are evaluated in
	 * greater precision, double rounding may occur, and if they are
	 * evaluated in less precision (as on i386), results will be
	 * wildly incorrect.
	 */
	x += shift[sign];
	x -= shift[sign];

	/*
	 * If the result is +-0, then it must have the same sign as x, but
	 * the above calculation doesn't always give this.  Fix up the sign.
	 */
	if (ex < BIAS && x == 0.0L)
		return (zero[sign]);

	return (x);
}