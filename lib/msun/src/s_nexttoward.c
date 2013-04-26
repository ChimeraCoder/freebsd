
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
 * We assume that a long double has a 15-bit exponent.  On systems
 * where long double is the same as double, nexttoward() is an alias
 * for nextafter(), so we don't use this routine.
 */

#include <float.h>

#include "fpmath.h"
#include "math.h"
#include "math_private.h"

#if LDBL_MAX_EXP != 0x4000
#error "Unsupported long double format"
#endif

double
nexttoward(double x, long double y)
{
	union IEEEl2bits uy;
	volatile double t;
	int32_t hx,ix;
	u_int32_t lx;

	EXTRACT_WORDS(hx,lx,x);
	ix = hx&0x7fffffff;		/* |x| */
	uy.e = y;

	if(((ix>=0x7ff00000)&&((ix-0x7ff00000)|lx)!=0) ||
	    (uy.bits.exp == 0x7fff &&
	     ((uy.bits.manh&~LDBL_NBIT)|uy.bits.manl) != 0))
	   return x+y;	/* x or y is nan */
	if(x==y) return (double)y;		/* x=y, return y */
	if(x==0.0) {
	    INSERT_WORDS(x,uy.bits.sign<<31,1);	/* return +-minsubnormal */
	    t = x*x;
	    if(t==x) return t; else return x;	/* raise underflow flag */
	}
	if(hx>0.0 ^ x < y) {			/* x -= ulp */
	    if(lx==0) hx -= 1;
	    lx -= 1;
	} else {				/* x += ulp */
	    lx += 1;
	    if(lx==0) hx += 1;
	}
	ix = hx&0x7ff00000;
	if(ix>=0x7ff00000) return x+x;	/* overflow  */
	if(ix<0x00100000) {		/* underflow */
	    t = x*x;
	    if(t!=x) {		/* raise underflow flag */
	        INSERT_WORDS(x,hx,lx);
		return x;
	    }
	}
	INSERT_WORDS(x,hx,lx);
	return x;
}