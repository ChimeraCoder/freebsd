
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

/* @(#)exp.c	8.1 (Berkeley) 6/4/93 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");


/* EXP(X)
 * RETURN THE EXPONENTIAL OF X
 * DOUBLE PRECISION (IEEE 53 bits, VAX D FORMAT 56 BITS)
 * CODED IN C BY K.C. NG, 1/19/85;
 * REVISED BY K.C. NG on 2/6/85, 2/15/85, 3/7/85, 3/24/85, 4/16/85, 6/14/86.
 *
 * Required system supported functions:
 *	scalb(x,n)
 *	copysign(x,y)
 *	finite(x)
 *
 * Method:
 *	1. Argument Reduction: given the input x, find r and integer k such
 *	   that
 *	                   x = k*ln2 + r,  |r| <= 0.5*ln2 .
 *	   r will be represented as r := z+c for better accuracy.
 *
 *	2. Compute exp(r) by
 *
 *		exp(r) = 1 + r + r*R1/(2-R1),
 *	   where
 *		R1 = x - x^2*(p1+x^2*(p2+x^2*(p3+x^2*(p4+p5*x^2)))).
 *
 *	3. exp(x) = 2^k * exp(r) .
 *
 * Special cases:
 *	exp(INF) is INF, exp(NaN) is NaN;
 *	exp(-INF)=  0;
 *	for finite argument, only exp(0)=1 is exact.
 *
 * Accuracy:
 *	exp(x) returns the exponential of x nearly rounded. In a test run
 *	with 1,156,000 random arguments on a VAX, the maximum observed
 *	error was 0.869 ulps (units in the last place).
 */

#include "mathimpl.h"

static const double p1 = 0x1.555555555553ep-3;
static const double p2 = -0x1.6c16c16bebd93p-9;
static const double p3 = 0x1.1566aaf25de2cp-14;
static const double p4 = -0x1.bbd41c5d26bf1p-20;
static const double p5 = 0x1.6376972bea4d0p-25;
static const double ln2hi = 0x1.62e42fee00000p-1;
static const double ln2lo = 0x1.a39ef35793c76p-33;
static const double lnhuge = 0x1.6602b15b7ecf2p9;
static const double lntiny = -0x1.77af8ebeae354p9;
static const double invln2 = 0x1.71547652b82fep0;

#if 0
double exp(x)
double x;
{
	double  z,hi,lo,c;
	int k;

#if !defined(vax)&&!defined(tahoe)
	if(x!=x) return(x);	/* x is NaN */
#endif	/* !defined(vax)&&!defined(tahoe) */
	if( x <= lnhuge ) {
		if( x >= lntiny ) {

		    /* argument reduction : x --> x - k*ln2 */

			k=invln2*x+copysign(0.5,x);	/* k=NINT(x/ln2) */

		    /* express x-k*ln2 as hi-lo and let x=hi-lo rounded */

			hi=x-k*ln2hi;
			x=hi-(lo=k*ln2lo);

		    /* return 2^k*[1+x+x*c/(2+c)]  */
			z=x*x;
			c= x - z*(p1+z*(p2+z*(p3+z*(p4+z*p5))));
			return  scalb(1.0+(hi-(lo-(x*c)/(2.0-c))),k);

		}
		/* end of x > lntiny */

		else
		     /* exp(-big#) underflows to zero */
		     if(finite(x))  return(scalb(1.0,-5000));

		     /* exp(-INF) is zero */
		     else return(0.0);
	}
	/* end of x < lnhuge */

	else
	/* exp(INF) is INF, exp(+big#) overflows to INF */
	    return( finite(x) ?  scalb(1.0,5000)  : x);
}
#endif

/* returns exp(r = x + c) for |c| < |x| with no overlap.  */

double __exp__D(x, c)
double x, c;
{
	double  z,hi,lo;
	int k;

	if (x != x)	/* x is NaN */
		return(x);
	if ( x <= lnhuge ) {
		if ( x >= lntiny ) {

		    /* argument reduction : x --> x - k*ln2 */
			z = invln2*x;
			k = z + copysign(.5, x);

		    /* express (x+c)-k*ln2 as hi-lo and let x=hi-lo rounded */

			hi=(x-k*ln2hi);			/* Exact. */
			x= hi - (lo = k*ln2lo-c);
		    /* return 2^k*[1+x+x*c/(2+c)]  */
			z=x*x;
			c= x - z*(p1+z*(p2+z*(p3+z*(p4+z*p5))));
			c = (x*c)/(2.0-c);

			return  scalb(1.+(hi-(lo - c)), k);
		}
		/* end of x > lntiny */

		else
		     /* exp(-big#) underflows to zero */
		     if(finite(x))  return(scalb(1.0,-5000));

		     /* exp(-INF) is zero */
		     else return(0.0);
	}
	/* end of x < lnhuge */

	else
	/* exp(INF) is INF, exp(+big#) overflows to INF */
	    return( finite(x) ?  scalb(1.0,5000)  : x);
}