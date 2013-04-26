
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

#include <sys/types.h>

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <syslog.h>
#include <time.h>

#define KASSERT(val, msg) assert(val)

typedef u_int32_t comp_t;

#define AHZ 1000000

#include "convert.c"

static int nerr;

union cf {
	comp_t c;
	float f;
};

static void
check_result(const char *name, float expected, union cf v)
{
	double eps;

	eps = fabs(expected - v.f) / expected;
	if (eps > FLT_EPSILON) {
		printf("Error in %s\n", name);
		printf("Got      0x%08x %12g\n", v.c, v.f);
		v.f = expected;
		printf("Expected 0x%08x %12g (%.15lg)\n", v.c, v.f, expected);
		printf("Epsilon=%lg, rather than %g\n", eps, FLT_EPSILON);
		nerr++;
	}
}

int
main(int argc, char *argv[])
{
	union cf v;
	long l;
	int i, end;
	struct timeval tv;

	if (argc == 2) {
		/* Loop test */
		end = atoi(argv[1]);
		for (i = 0; i < end; i++) {
			tv.tv_sec = random();
			tv.tv_usec = (random() % 1000000);
			v.c = encode_timeval(tv);
			check_result("encode_timeval",
			    (float)tv.tv_sec * AHZ + tv.tv_usec, v);
			l = random();
			v.c = encode_long(l);
			check_result("encode_long", l, v);
		}
	} else if (argc == 3) {
		/* Single-value timeval/long test */
		tv.tv_sec = atol(argv[1]);
		tv.tv_usec = atol(argv[2]);
		v.c = encode_timeval(tv);
		check_result("encode_timeval",
		    (float)tv.tv_sec * AHZ + tv.tv_usec, v);
		v.c = encode_long(tv.tv_sec);
		check_result("encode_long", tv.tv_sec, v);
	} else {
		fprintf(stderr, "usage:\n%s repetitions\n%s sec usec\n",
		    argv[0], argv[0]);
		return (1);
	}
	return (nerr);
}