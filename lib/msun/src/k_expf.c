
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

#include <complex.h>

#include "math.h"
#include "math_private.h"

static const uint32_t k = 235;			/* constant for reduction */
static const float kln2 =  162.88958740F;	/* k * ln2 */

/*
 * See k_exp.c for details.
 *
 * Input:  ln(FLT_MAX) <= x < ln(2 * FLT_MAX / FLT_MIN_DENORM) ~= 192.7
 * Output: 2**127 <= y < 2**128
 */
static float
__frexp_expf(float x, int *expt)
{
	float exp_x;
	uint32_t hx;

	exp_x = expf(x - kln2);
	GET_FLOAT_WORD(hx, exp_x);
	*expt = (hx >> 23) - (0x7f + 127) + k;
	SET_FLOAT_WORD(exp_x, (hx & 0x7fffff) | ((0x7f + 127) << 23));
	return (exp_x);
}

float
__ldexp_expf(float x, int expt)
{
	float exp_x, scale;
	int ex_expt;

	exp_x = __frexp_expf(x, &ex_expt);
	expt += ex_expt;
	SET_FLOAT_WORD(scale, (0x7f + expt) << 23);
	return (exp_x * scale);
}

float complex
__ldexp_cexpf(float complex z, int expt)
{
	float x, y, exp_x, scale1, scale2;
	int ex_expt, half_expt;

	x = crealf(z);
	y = cimagf(z);
	exp_x = __frexp_expf(x, &ex_expt);
	expt += ex_expt;

	half_expt = expt / 2;
	SET_FLOAT_WORD(scale1, (0x7f + half_expt) << 23);
	half_expt = expt - half_expt;
	SET_FLOAT_WORD(scale2, (0x7f + half_expt) << 23);

	return (cpackf(cosf(y) * exp_x * scale1 * scale2,
	    sinf(y) * exp_x * scale1 * scale2));
}