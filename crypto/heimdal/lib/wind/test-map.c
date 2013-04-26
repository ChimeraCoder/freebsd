
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
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "windlocl.h"

#define MAX_LENGTH 2

struct example {
    uint32_t in[MAX_LENGTH];
    size_t in_len;
    uint32_t out[MAX_LENGTH];
    size_t out_len;
};

static struct example cases[] = {
    {{0}, 0, {0}, 0},
    {{0x0041}, 1, {0x0061}, 1},
    {{0x0061}, 1, {0x0061}, 1},
    {{0x00AD}, 1, {0}, 0},
    {{0x00DF}, 1, {0x0073, 0x0073}, 2}
};

static int
try(const struct example *c)
{
    int ret;
    size_t out_len = c->out_len;
    uint32_t *tmp = malloc(out_len * sizeof(uint32_t));
    if (tmp == NULL && out_len != 0)
	err(1, "malloc");
    ret = _wind_stringprep_map(c->in, c->in_len, tmp, &out_len, WIND_PROFILE_NAME);
    if (ret) {
	printf("wind_stringprep_map failed\n");
	return 1;
    }
    if (out_len != c->out_len) {
	printf("wrong out len\n");
	free(tmp);
	return 1;
    }
    if (memcmp(c->out, tmp, out_len * sizeof(uint32_t)) != 0) {
	printf("wrong out data\n");
	free(tmp);
	return 1;
    }
    free(tmp);
    return 0;
}

int
main(void)
{
    unsigned i;
    unsigned failures = 0;

    for (i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i)
	failures += try(&cases[i]);
    return failures != 0;
}