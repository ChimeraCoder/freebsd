
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

/* Returns: the product of a + ib and c + id */

float _Complex
__mulsc3(float __a, float __b, float __c, float __d)
{
    float __ac = __a * __c;
    float __bd = __b * __d;
    float __ad = __a * __d;
    float __bc = __b * __c;
    float _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        int __recalc = 0;
        if (crt_isinf(__a) || crt_isinf(__b))
        {
            __a = crt_copysignf(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysignf(crt_isinf(__b) ? 1 : 0, __b);
            if (crt_isnan(__c))
                __c = crt_copysignf(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysignf(0, __d);
            __recalc = 1;
        }
        if (crt_isinf(__c) || crt_isinf(__d))
        {
            __c = crt_copysignf(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysignf(crt_isinf(__d) ? 1 : 0, __d);
            if (crt_isnan(__a))
                __a = crt_copysignf(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysignf(0, __b);
            __recalc = 1;
        }
        if (!__recalc && (crt_isinf(__ac) || crt_isinf(__bd) ||
                          crt_isinf(__ad) || crt_isinf(__bc)))
        {
            if (crt_isnan(__a))
                __a = crt_copysignf(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysignf(0, __b);
            if (crt_isnan(__c))
                __c = crt_copysignf(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysignf(0, __d);
            __recalc = 1;
        }
        if (__recalc)
        {
            __real__ z = CRT_INFINITY * (__a * __c - __b * __d);
            __imag__ z = CRT_INFINITY * (__a * __d + __b * __c);
        }
    }
    return z;
}