
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

#include <math.h>

#include "fpmath.h"

/*
 * XXX These routines belong in libm, but they must remain in libc for
 *     binary compat until we can bump libm's major version number.
 */

__weak_reference(__isinf, isinf);

int
__isinf(double d)
{
	union IEEEd2bits u;

	u.d = d;
	return (u.bits.exp == 2047 && u.bits.manl == 0 && u.bits.manh == 0);
}

int
__isinff(float f)
{
	union IEEEf2bits u;

	u.f = f;
	return (u.bits.exp == 255 && u.bits.man == 0);
}

int
__isinfl(long double e)
{
	union IEEEl2bits u;

	u.e = e;
	mask_nbit_l(u);
#ifndef __alpha__
	return (u.bits.exp == 32767 && u.bits.manl == 0 && u.bits.manh == 0);
#else
	return (u.bits.exp == 2047 && u.bits.manl == 0 && u.bits.manh == 0);
#endif
}