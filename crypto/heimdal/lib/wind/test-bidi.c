
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "windlocl.h"

#define MAX_LENGTH 10

struct test {
    unsigned len;
    uint32_t vals[MAX_LENGTH];
};

static struct test passing_cases[] = {
    {0, {0}},
    {1, {0x0041}},
    {1, {0x05be}},
};

static struct test failing_cases[] = {
    {2, {0x05be, 0x0041}},
    {3, {0x05be, 0x0041, 0x05be}},
};

int
main(void)
{
    unsigned i;
    unsigned failures = 0;

    for (i = 0; i < sizeof(passing_cases)/sizeof(passing_cases[0]); ++i) {
	const struct test *t = &passing_cases[i];
	if (_wind_stringprep_testbidi(t->vals, t->len, WIND_PROFILE_NAME)) {
	    printf ("passing case %u failed\n", i);
	    ++failures;
	}
    }

    for (i = 0; i < sizeof(failing_cases)/sizeof(failing_cases[0]); ++i) {
	const struct test *t = &failing_cases[i];
	if (!_wind_stringprep_testbidi(t->vals, t->len, WIND_PROFILE_NAME)) {
	    printf ("failing case %u passed\n", i);
	    ++failures;
	}
    }

    return failures != 0;
}