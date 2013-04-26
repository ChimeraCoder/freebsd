
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

#include "DD.h"
#include "../int_math.h"

#define makeFinite(x) { \
    (x).s.hi = crt_copysign(crt_isinf((x).s.hi) ? 1.0 : 0.0, (x).s.hi); \
    (x).s.lo = 0.0;                                                     \
  }

#define zeroNaN(x) { \
    if (crt_isnan((x).s.hi)) {                                          \
      (x).s.hi = crt_copysign(0.0, (x).s.hi);                     \
      (x).s.lo = 0.0;                                                   \
    }                                                                   \
  }

long double __gcc_qadd(long double, long double);
long double __gcc_qsub(long double, long double);
long double __gcc_qmul(long double, long double);

long double _Complex
__multc3(long double a, long double b, long double c, long double d)
{
	long double ac = __gcc_qmul(a,c);
	long double bd = __gcc_qmul(b,d);
	long double ad = __gcc_qmul(a,d);
	long double bc = __gcc_qmul(b,c);
	
	DD real = { .ld = __gcc_qsub(ac,bd) };
	DD imag = { .ld = __gcc_qadd(ad,bc) };
	
	if (crt_isnan(real.s.hi) && crt_isnan(imag.s.hi))
	{
		int recalc = 0;
		
		DD aDD = { .ld = a };
		DD bDD = { .ld = b };
		DD cDD = { .ld = c };
		DD dDD = { .ld = d };
		
		if (crt_isinf(aDD.s.hi) || crt_isinf(bDD.s.hi))
		{
			makeFinite(aDD);
			makeFinite(bDD);
			zeroNaN(cDD);
			zeroNaN(dDD);
			recalc = 1;
		}
		
		if (crt_isinf(cDD.s.hi) || crt_isinf(dDD.s.hi))
		{
			makeFinite(cDD);
			makeFinite(dDD);
			zeroNaN(aDD);
			zeroNaN(bDD);
			recalc = 1;
		}
		
		if (!recalc)
		{
			DD acDD = { .ld = ac };
			DD bdDD = { .ld = bd };
			DD adDD = { .ld = ad };
			DD bcDD = { .ld = bc };
			
			if (crt_isinf(acDD.s.hi) || crt_isinf(bdDD.s.hi) ||
                            crt_isinf(adDD.s.hi) || crt_isinf(bcDD.s.hi))
			{
				zeroNaN(aDD);
				zeroNaN(bDD);
				zeroNaN(cDD);
				zeroNaN(dDD);
				recalc = 1;
			}
		}
		
		if (recalc)
		{
			real.s.hi = CRT_INFINITY * (aDD.s.hi*cDD.s.hi - bDD.s.hi*dDD.s.hi);
			real.s.lo = 0.0;
			imag.s.hi = CRT_INFINITY * (aDD.s.hi*dDD.s.hi + bDD.s.hi*cDD.s.hi);
			imag.s.lo = 0.0;
		}
	}
	
	long double _Complex z;
	__real__ z = real.ld;
	__imag__ z = imag.ld;
	
	return z;
}