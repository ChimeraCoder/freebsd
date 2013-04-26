
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
 * See comments in e_acos.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */

#include <float.h>

#include "invtrig.h"
#include "math.h"
#include "math_private.h"

static const long double
one=  1.00000000000000000000e+00;

#ifdef __i386__
/* XXX Work around the fact that gcc truncates long double constants on i386 */
static volatile double
pi1 =  3.14159265358979311600e+00,	/*  0x1.921fb54442d18p+1  */
pi2 =  1.22514845490862001043e-16;	/*  0x1.1a80000000000p-53 */
#define	pi	((long double)pi1 + pi2)
#else
static const long double
pi =  3.14159265358979323846264338327950280e+00L;
#endif

long double
acosl(long double x)
{
	union IEEEl2bits u;
	long double z,p,q,r,w,s,c,df;
	int16_t expsign, expt;
	u.e = x;
	expsign = u.xbits.expsign;
	expt = expsign & 0x7fff;
	if(expt >= BIAS) {	/* |x| >= 1 */
	    if(expt==BIAS && ((u.bits.manh&~LDBL_NBIT)|u.bits.manl)==0) {
		if (expsign>0) return 0.0;	/* acos(1) = 0  */
		else return pi+2.0*pio2_lo;	/* acos(-1)= pi */
	    }
	    return (x-x)/(x-x);		/* acos(|x|>1) is NaN */
	}
	if(expt<BIAS-1) {	/* |x| < 0.5 */
	    if(expt<ACOS_CONST) return pio2_hi+pio2_lo;/*x tiny: acosl=pi/2*/
	    z = x*x;
	    p = P(z);
	    q = Q(z);
	    r = p/q;
	    return pio2_hi - (x - (pio2_lo-x*r));
	} else  if (expsign<0) {	/* x < -0.5 */
	    z = (one+x)*0.5;
	    p = P(z);
	    q = Q(z);
	    s = sqrtl(z);
	    r = p/q;
	    w = r*s-pio2_lo;
	    return pi - 2.0*(s+w);
	} else {			/* x > 0.5 */
	    z = (one-x)*0.5;
	    s = sqrtl(z);
	    u.e = s;
	    u.bits.manl = 0;
	    df = u.e;
	    c  = (z-df*df)/(s+df);
	    p = P(z);
	    q = Q(z);
	    r = p/q;
	    w = r*s+c;
	    return 2.0*(df+w);
	}
}