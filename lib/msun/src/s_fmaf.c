
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

#include <fenv.h>

#include "math.h"
#include "math_private.h"

/*
 * Fused multiply-add: Compute x * y + z with a single rounding error.
 *
 * A double has more than twice as much precision than a float, so
 * direct double-precision arithmetic suffices, except where double
 * rounding occurs.
 */
float
fmaf(float x, float y, float z)
{
	double xy, result;
	uint32_t hr, lr;

	xy = (double)x * y;
	result = xy + z;
	EXTRACT_WORDS(hr, lr, result);
	/* Common case: The double precision result is fine. */
	if ((lr & 0x1fffffff) != 0x10000000 ||	/* not a halfway case */
	    (hr & 0x7ff00000) == 0x7ff00000 ||	/* NaN */
	    result - xy == z ||			/* exact */
	    fegetround() != FE_TONEAREST)	/* not round-to-nearest */
		return (result);

	/*
	 * If result is inexact, and exactly halfway between two float values,
	 * we need to adjust the low-order bit in the direction of the error.
	 */
	fesetround(FE_TOWARDZERO);
	volatile double vxy = xy;  /* XXX work around gcc CSE bug */
	double adjusted_result = vxy + z;
	fesetround(FE_TONEAREST);
	if (result == adjusted_result)
		SET_LOW_WORD(adjusted_result, lr + 1);
	return (adjusted_result);
}