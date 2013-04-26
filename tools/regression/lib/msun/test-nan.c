
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

/*
 * Test for nan(), nanf(), and nanl(). We also test that strtod("nan(...)")
 * and sscanf("nan(...)", ...) work identically.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <fenv.h>
#include <float.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
testnan(const char *nan_format)
{
	char nan_str[128];
	char *end;
	long double ald[4];
	double ad[4];
	float af[4];
	int i;

	snprintf(nan_str, sizeof(nan_str), "nan(%s)", nan_format);
	for (i = 0; i < 4; i++) {
		/*
		 * x86 has an 80-bit long double stored in 96 bits,
		 * so we need to initialize the memory for the memcmp()
		 * checks below to work.
		 */
		bzero(&af[i], sizeof(float));
		bzero(&ad[i], sizeof(double));
		bzero(&ald[i], sizeof(long double));
		   
	}

	af[0] = nanf(nan_format);
	assert(isnan(af[0]));
	af[1] = strtof(nan_str, &end);
	assert(end == nan_str + strlen(nan_str));
	assert(sscanf(nan_str, "%e", &af[2]) == 1);
	assert(memcmp(&af[0], &af[1], sizeof(float)) == 0);
	assert(memcmp(&af[1], &af[2], sizeof(float)) == 0);
	if (*nan_format == '\0') {
		/* nanf("") == strtof("nan") */
		af[3] = strtof("nan", NULL);
		assert(memcmp(&af[2], &af[3], sizeof(float)) == 0);
	}

	ad[0] = nan(nan_format);
	assert(isnan(ad[0]));
	ad[1] = strtod(nan_str, &end);
	assert(end == nan_str + strlen(nan_str));
	assert(sscanf(nan_str, "%le", &ad[2]) == 1);
	assert(memcmp(&ad[0], &ad[1], sizeof(double)) == 0);
	assert(memcmp(&ad[1], &ad[2], sizeof(double)) == 0);
	if (*nan_format == '\0') {
		/* nan("") == strtod("nan") */
		ad[3] = strtod("nan", NULL);
		assert(memcmp(&ad[2], &ad[3], sizeof(double)) == 0);
	}

	ald[0] = nanl(nan_format);
	assert(isnan(ald[0]));
	ald[1] = strtold(nan_str, &end);
	assert(end == nan_str + strlen(nan_str));
	assert(sscanf(nan_str, "%Le", &ald[2]) == 1);
	assert(memcmp(&ald[0], &ald[1], sizeof(long double)) == 0);
	assert(memcmp(&ald[1], &ald[2], sizeof(long double)) == 0);
	if (*nan_format == '\0') {
		/* nanl("") == strtold("nan") */
		ald[3] = strtold("nan", NULL);
		assert(memcmp(&ald[2], &ald[3], sizeof(long double)) == 0);
	}
}

int
main(int argc, char *argv[])
{

	printf("1..1\n");

	/* Die if a signalling NaN is returned */
	feenableexcept(FE_INVALID);

	testnan("0x1234");
	testnan("");

	printf("ok 1 - nan\n");

	return (0);
}