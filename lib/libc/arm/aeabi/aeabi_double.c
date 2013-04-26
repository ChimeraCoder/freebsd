
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

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

flag __unorddf2(float64, float64);

int __aeabi_dcmpeq(float64 a, float64 b)
{
	return float64_eq(a, b);
}

int __aeabi_dcmplt(float64 a, float64 b)
{
	return float64_lt(a, b);
}

int __aeabi_dcmple(float64 a, float64 b)
{
	return float64_le(a, b);
}

int __aeabi_dcmpge(float64 a, float64 b)
{
	return float64_le(b, a);
}

int __aeabi_dcmpgt(float64 a, float64 b)
{
	return float64_lt(b, a);
}

int __aeabi_dcmpun(float64 a, float64 b)
{
	return __unorddf2(a, b);
}

int __aeabi_d2iz(float64 a)
{
	return float64_to_int32_round_to_zero(a);
}

float32 __aeabi_d2f(float64 a)
{
	return float64_to_float32(a);
}

float64 __aeabi_i2d(int a)
{
	return int32_to_float64(a);
}

float64 __aeabi_dadd(float64 a, float64 b)
{
	return float64_add(a, b);
}

float64 __aeabi_ddiv(float64 a, float64 b)
{
	return float64_div(a, b);
}

float64 __aeabi_dmul(float64 a, float64 b)
{
	return float64_mul(a, b);
}

float64 __aeabi_dsub(float64 a, float64 b)
{
	return float64_sub(a, b);
}