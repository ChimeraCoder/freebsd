
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
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	char buf[128], *end;
	double d;
	float f;
	long double ld;
	int e, i;

	printf("1..3\n");
	assert(ilogb(0) == FP_ILOGB0);
	assert(ilogb(NAN) == FP_ILOGBNAN);
	assert(ilogb(INFINITY) == INT_MAX);
	for (e = DBL_MIN_EXP - DBL_MANT_DIG; e < DBL_MAX_EXP; e++) {
		snprintf(buf, sizeof(buf), "0x1.p%d", e);
		d = strtod(buf, &end);
		assert(*end == '\0');
		i = ilogb(d);
		assert(i == e);
	}
	printf("ok 1 - ilogb\n");

	assert(ilogbf(0) == FP_ILOGB0);
	assert(ilogbf(NAN) == FP_ILOGBNAN);
	assert(ilogbf(INFINITY) == INT_MAX);
	for (e = FLT_MIN_EXP - FLT_MANT_DIG; e < FLT_MAX_EXP; e++) {
		snprintf(buf, sizeof(buf), "0x1.p%d", e);
		f = strtof(buf, &end);
		assert(*end == '\0');
		i = ilogbf(f);
		assert(i == e);
	}
	printf("ok 2 - ilogbf\n");

	assert(ilogbl(0) == FP_ILOGB0);
	assert(ilogbl(NAN) == FP_ILOGBNAN);
	assert(ilogbl(INFINITY) == INT_MAX);
	for (e = LDBL_MIN_EXP - LDBL_MANT_DIG; e < LDBL_MAX_EXP; e++) {
		snprintf(buf, sizeof(buf), "0x1.p%d", e);
		ld = strtold(buf, &end);
		assert(*end == '\0');
		i = ilogbl(ld);
		assert(i == e);
	}
	printf("ok 3 - ilogbl\n");

	return (0);
}