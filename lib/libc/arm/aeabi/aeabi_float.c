
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

flag __unordsf2(float32, float32);

int __aeabi_fcmpeq(float32 a, float32 b)
{
	return float32_eq(a, b);
}

int __aeabi_fcmplt(float32 a, float32 b)
{
	return float32_lt(a, b);
}

int __aeabi_fcmple(float32 a, float32 b)
{
	return float32_le(a, b);
}

int __aeabi_fcmpge(float32 a, float32 b)
{
	return float32_le(b, a);
}

int __aeabi_fcmpgt(float32 a, float32 b)
{
	return float32_lt(b, a);
}

int __aeabi_fcmpun(float32 a, float32 b)
{
	return __unordsf2(a, b);
}

int __aeabi_f2iz(float32 a)
{
	return float32_to_int32_round_to_zero(a);
}

float32 __aeabi_f2d(float32 a)
{
	return float32_to_float64(a);
}

float32 __aeabi_i2f(int a)
{
	return int32_to_float32(a);
}

float32 __aeabi_fadd(float32 a, float32 b)
{
	return float32_add(a, b);
}

float32 __aeabi_fdiv(float32 a, float32 b)
{
	return float32_div(a, b);
}

float32 __aeabi_fmul(float32 a, float32 b)
{
	return float32_mul(a, b);
}

float32 __aeabi_fsub(float32 a, float32 b)
{
	return float32_sub(a, b);
}