
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
 * double logb(x)
 * IEEE 754 logb. Included to pass IEEE test suite. Not recommend.
 * Use ilogb instead.
 */

#include <float.h>

#include "math.h"
#include "math_private.h"

static const double
two54 = 1.80143985094819840000e+16;	/* 43500000 00000000 */

double
logb(double x)
{
	int32_t lx,ix;
	EXTRACT_WORDS(ix,lx,x);
	ix &= 0x7fffffff;			/* high |x| */
	if((ix|lx)==0) return -1.0/fabs(x);
	if(ix>=0x7ff00000) return x*x;
	if(ix<0x00100000) {
		x *= two54;		 /* convert subnormal x to normal */
		GET_HIGH_WORD(ix,x);
		ix &= 0x7fffffff;
		return (double) ((ix>>20)-1023-54);
	} else
		return (double) ((ix>>20)-1023);
}

#if (LDBL_MANT_DIG == 53)
__weak_reference(logb, logbl);
#endif