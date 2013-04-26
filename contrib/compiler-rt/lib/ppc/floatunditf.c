
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

/* long double __floatunditf(unsigned long long x); */
/* This file implements the PowerPC unsigned long long -> long double conversion */

#include "DD.h"

long double __floatunditf(uint64_t a) {
	
	/* Begins with an exact copy of the code from __floatundidf */
	
	static const double twop52 = 0x1.0p52;
	static const double twop84 = 0x1.0p84;
	static const double twop84_plus_twop52 = 0x1.00000001p84;
	
	doublebits high = { .d = twop84 };
	doublebits low  = { .d = twop52 };
	
	high.x |= a >> 32;							/* 0x1.0p84 + high 32 bits of a */
	low.x |= a & UINT64_C(0x00000000ffffffff);	/* 0x1.0p52 + low 32 bits of a */
	
	const double high_addend = high.d - twop84_plus_twop52;
	
	/* At this point, we have two double precision numbers
	 * high_addend and low.d, and we wish to return their sum
	 * as a canonicalized long double:
	 */

	/* This implementation sets the inexact flag spuriously. */
	/* This could be avoided, but at some substantial cost. */
	
	DD result;
	
	result.s.hi = high_addend + low.d;
	result.s.lo = (high_addend - result.s.hi) + low.d;
	
	return result.ld;
	
}