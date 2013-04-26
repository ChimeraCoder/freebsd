
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

/*
 * ceill(x)
 * Return x rounded toward -inf to integral value
 * Method:
 *	Bit twiddling.
 * Exception:
 *	Inexact flag raised if x not equal to ceill(x).
 */

#include <float.h>
#include <math.h>
#include <stdint.h>

#include "fpmath.h"

#ifdef LDBL_IMPLICIT_NBIT
#define	MANH_SIZE	(LDBL_MANH_SIZE + 1)
#define	INC_MANH(u, c)	do {					\
	uint64_t o = u.bits.manh;				\
	u.bits.manh += (c);					\
	if (u.bits.manh < o)					\
		u.bits.exp++;					\
} while (0)
#else
#define	MANH_SIZE	LDBL_MANH_SIZE
#define	INC_MANH(u, c)	do {					\
	uint64_t o = u.bits.manh;				\
	u.bits.manh += (c);					\
	if (u.bits.manh < o) {					\
		u.bits.exp++;					\
		u.bits.manh |= 1llu << (LDBL_MANH_SIZE - 1);	\
	}							\
} while (0)
#endif

static const long double huge = 1.0e300;

long double
ceill(long double x)
{
	union IEEEl2bits u = { .e = x };
	int e = u.bits.exp - LDBL_MAX_EXP + 1;

	if (e < MANH_SIZE - 1) {
		if (e < 0) {			/* raise inexact if x != 0 */
			if (huge + x > 0.0)
				if (u.bits.exp > 0 ||
				    (u.bits.manh | u.bits.manl) != 0)
					u.e = u.bits.sign ? -0.0 : 1.0;
		} else {
			uint64_t m = ((1llu << MANH_SIZE) - 1) >> (e + 1);
			if (((u.bits.manh & m) | u.bits.manl) == 0)
				return (x);	/* x is integral */
			if (!u.bits.sign) {
#ifdef LDBL_IMPLICIT_NBIT
				if (e == 0)
					u.bits.exp++;
				else
#endif
				INC_MANH(u, 1llu << (MANH_SIZE - e - 1));
			}
			if (huge + x > 0.0) {	/* raise inexact flag */
				u.bits.manh &= ~m;
				u.bits.manl = 0;
			}
		}
	} else if (e < LDBL_MANT_DIG - 1) {
		uint64_t m = (uint64_t)-1 >> (64 - LDBL_MANT_DIG + e + 1);
		if ((u.bits.manl & m) == 0)
			return (x);	/* x is integral */
		if (!u.bits.sign) {
			if (e == MANH_SIZE - 1)
				INC_MANH(u, 1);
			else {
				uint64_t o = u.bits.manl;
				u.bits.manl += 1llu << (LDBL_MANT_DIG - e - 1);
				if (u.bits.manl < o)	/* got a carry */
					INC_MANH(u, 1);
			}
		}
		if (huge + x > 0.0)		/* raise inexact flag */
			u.bits.manl &= ~m;
	}
	return (u.e);
}