
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

#include "bidi_table.h"

static int
range_entry_cmp(const void *a, const void *b)
{
    const struct range_entry *ea = (const struct range_entry*)a;
    const struct range_entry *eb = (const struct range_entry*)b;

    if (ea->start >= eb->start && ea->start < eb->start + eb->len)
	return 0;
    return ea->start - eb->start;
}

static int
is_ral(uint32_t cp)
{
    struct range_entry ee = {cp};
    void *s = bsearch(&ee, _wind_ral_table, _wind_ral_table_size,
		      sizeof(_wind_ral_table[0]),
		      range_entry_cmp);
    return s != NULL;
}

static int
is_l(uint32_t cp)
{
    struct range_entry ee = {cp};
    void *s = bsearch(&ee, _wind_l_table, _wind_l_table_size,
		      sizeof(_wind_l_table[0]),
		      range_entry_cmp);
    return s != NULL;
}

int
_wind_stringprep_testbidi(const uint32_t *in, size_t in_len, wind_profile_flags flags)
{
    size_t i;
    unsigned ral = 0;
    unsigned l   = 0;

    if ((flags & (WIND_PROFILE_NAME|WIND_PROFILE_SASL)) == 0)
	return 0;

    for (i = 0; i < in_len; ++i) {
	ral |= is_ral(in[i]);
	l   |= is_l(in[i]);
    }
    if (ral) {
	if (l)
	    return 1;
	if (!is_ral(in[0]) || !is_ral(in[in_len - 1]))
	    return 1;
    }
    return 0;
}