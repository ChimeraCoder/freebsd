
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

#include "int_lib.h"
#include "int_math.h"

/* Returns: the quotient of (a + ib) / (c + id) */

float _Complex
__divsc3(float __a, float __b, float __c, float __d)
{
    int __ilogbw = 0;
    float __logbw = crt_logbf(crt_fmaxf(crt_fabsf(__c), crt_fabsf(__d)));
    if (crt_isfinite(__logbw))
    {
        __ilogbw = (int)__logbw;
        __c = crt_scalbnf(__c, -__ilogbw);
        __d = crt_scalbnf(__d, -__ilogbw);
    }
    float __denom = __c * __c + __d * __d;
    float _Complex z;
    __real__ z = crt_scalbnf((__a * __c + __b * __d) / __denom, -__ilogbw);
    __imag__ z = crt_scalbnf((__b * __c - __a * __d) / __denom, -__ilogbw);
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        if ((__denom == 0) && (!crt_isnan(__a) || !crt_isnan(__b)))
        {
            __real__ z = crt_copysignf(CRT_INFINITY, __c) * __a;
            __imag__ z = crt_copysignf(CRT_INFINITY, __c) * __b;
        }
        else if ((crt_isinf(__a) || crt_isinf(__b)) &&
                 crt_isfinite(__c) && crt_isfinite(__d))
        {
            __a = crt_copysignf(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysignf(crt_isinf(__b) ? 1 : 0, __b);
            __real__ z = CRT_INFINITY * (__a * __c + __b * __d);
            __imag__ z = CRT_INFINITY * (__b * __c - __a * __d);
        }
        else if (crt_isinf(__logbw) && __logbw > 0 &&
                 crt_isfinite(__a) && crt_isfinite(__b))
        {
            __c = crt_copysignf(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysignf(crt_isinf(__d) ? 1 : 0, __d);
            __real__ z = 0 * (__a * __c + __b * __d);
            __imag__ z = 0 * (__b * __c - __a * __d);
        }
    }
    return z;
}