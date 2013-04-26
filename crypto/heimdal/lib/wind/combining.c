
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

#include "windlocl.h"

#include <stdlib.h>

#include "combining_table.h"

static int
translation_cmp(const void *key, const void *data)
{
    const struct translation *t1 = (const struct translation *)key;
    const struct translation *t2 = (const struct translation *)data;

    return t1->key - t2->key;
}

int
_wind_combining_class(uint32_t code_point)
{
    struct translation ts = {code_point};
    void *s = bsearch(&ts, _wind_combining_table, _wind_combining_table_size,
		      sizeof(_wind_combining_table[0]),
		      translation_cmp);
    if (s != NULL) {
	const struct translation *t = (const struct translation *)s;
	return t->combining_class;
    } else {
	return 0;
    }
}