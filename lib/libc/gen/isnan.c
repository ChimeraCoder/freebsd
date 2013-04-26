
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
 *
 * Note this only applies to the dynamic versions of libm and libc, so
 * for the static and profiled versions we stub out the definitions.
 * Otherwise you cannot link statically to libm and libc at the same
 * time, when calling both functions.
 */

#ifdef PIC
__weak_reference(__isnan, isnan);
__weak_reference(__isnanf, isnanf);

int
__isnan(double d)
{
	union IEEEd2bits u;

	u.d = d;
	return (u.bits.exp == 2047 && (u.bits.manl != 0 || u.bits.manh != 0));
}

int
__isnanf(float f)
{
	union IEEEf2bits u;

	u.f = f;
	return (u.bits.exp == 255 && u.bits.man != 0);
}
#endif /* PIC */