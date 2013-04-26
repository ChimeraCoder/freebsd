
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
/* e_remainderf.c -- float version of e_remainder.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "math.h"
#include "math_private.h"

static const float zero = 0.0;


float
__ieee754_remainderf(float x, float p)
{
	int32_t hx,hp;
	u_int32_t sx;
	float p_half;

	GET_FLOAT_WORD(hx,x);
	GET_FLOAT_WORD(hp,p);
	sx = hx&0x80000000;
	hp &= 0x7fffffff;
	hx &= 0x7fffffff;

    /* purge off exception values */
	if(hp==0) return (x*p)/(x*p);	 	/* p = 0 */
	if((hx>=0x7f800000)||			/* x not finite */
	  ((hp>0x7f800000)))			/* p is NaN */
	    return ((long double)x*p)/((long double)x*p);


	if (hp<=0x7effffff) x = __ieee754_fmodf(x,p+p);	/* now x < 2p */
	if ((hx-hp)==0) return zero*x;
	x  = fabsf(x);
	p  = fabsf(p);
	if (hp<0x01000000) {
	    if(x+x>p) {
		x-=p;
		if(x+x>=p) x -= p;
	    }
	} else {
	    p_half = (float)0.5*p;
	    if(x>p_half) {
		x-=p;
		if(x>=p_half) x -= p;
	    }
	}
	GET_FLOAT_WORD(hx,x);
	if ((hx&0x7fffffff)==0) hx = 0;
	SET_FLOAT_WORD(x,hx^sx);
	return x;
}