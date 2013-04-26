
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

#include <assert.h>
#include <math.h>
#include <stdlib.h>

int
main(void)
{

	assert(fpclassify((float)0) == FP_ZERO);
	assert(fpclassify((float)-0.0) == FP_ZERO);
	assert(fpclassify((float)1) == FP_NORMAL);
	assert(fpclassify((float)1000) == FP_NORMAL);
#ifndef __alpha__
	assert(fpclassify(0x1.2p-150f) == FP_SUBNORMAL);
#endif
	assert(fpclassify(HUGE_VALF) == FP_INFINITE);
	assert(fpclassify((float)HUGE_VAL) == FP_INFINITE);
	assert(fpclassify((float)HUGE_VALL) == FP_INFINITE);
	assert(fpclassify(NAN) == FP_NAN);

	assert(fpclassify((double)0) == FP_ZERO);
	assert(fpclassify((double)-0) == FP_ZERO);
	assert(fpclassify((double)1) == FP_NORMAL);
	assert(fpclassify((double)1000) == FP_NORMAL);
#ifndef __alpha__
	assert(fpclassify(0x1.2p-1075) == FP_SUBNORMAL);
#endif
	assert(fpclassify(HUGE_VAL) == FP_INFINITE);
	assert(fpclassify((double)HUGE_VALF) == FP_INFINITE);
	assert(fpclassify((double)HUGE_VALL) == FP_INFINITE);
	assert(fpclassify((double)NAN) == FP_NAN);

	assert(fpclassify((long double)0) == FP_ZERO);
	assert(fpclassify((long double)-0.0) == FP_ZERO);
	assert(fpclassify((long double)1) == FP_NORMAL);
	assert(fpclassify((long double)1000) == FP_NORMAL);
#ifndef __alpha__
	assert(fpclassify(0x1.2p-16383L) == FP_SUBNORMAL);
#endif
	assert(fpclassify(HUGE_VALL) == FP_INFINITE);
	assert(fpclassify((long double)HUGE_VALF) == FP_INFINITE);
	assert(fpclassify((long double)HUGE_VAL) == FP_INFINITE);
	assert(fpclassify((long double)NAN) == FP_NAN);

	printf("PASS fpclassify()\n");
	exit(0);
}