
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
#include "windlocl.h"

#include <stdlib.h>

#include "map_table.h"

static int
translation_cmp(const void *key, const void *data)
{
    const struct translation *t1 = (const struct translation *)key;
    const struct translation *t2 = (const struct translation *)data;

    return t1->key - t2->key;
}

int
_wind_stringprep_map(const uint32_t *in, size_t in_len,
		     uint32_t *out, size_t *out_len,
		     wind_profile_flags flags)
{
    unsigned i;
    unsigned o = 0;

    for (i = 0; i < in_len; ++i) {
	struct translation ts = {in[i]};
	const struct translation *s;

	s = (const struct translation *)
	    bsearch(&ts, _wind_map_table, _wind_map_table_size,
		    sizeof(_wind_map_table[0]),
		    translation_cmp);
	if (s != NULL && (s->flags & flags)) {
	    unsigned j;

	    for (j = 0; j < s->val_len; ++j) {
		if (o >= *out_len)
		    return WIND_ERR_OVERRUN;
		out[o++] = _wind_map_table_val[s->val_offset + j];
	    }
	} else {
	    if (o >= *out_len)
		return WIND_ERR_OVERRUN;
	    out[o++] = in[i];

	}
    }
    *out_len = o;
    return 0;
}