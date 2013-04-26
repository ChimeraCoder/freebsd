
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
#include <assert.h>

static int
put_char(uint32_t *out, size_t *o, uint32_t c, size_t out_len)
{
    if (*o >= out_len)
	return 1;
    out[*o] = c;
    (*o)++;
    return 0;
}

int
_wind_ldap_case_exact_attribute(const uint32_t *tmp,
				size_t olen,
				uint32_t *out,
				size_t *out_len)
{
    size_t o = 0, i = 0;

    if (olen == 0) {
	*out_len = 0;
	return 0;
    }

    if (put_char(out, &o, 0x20, *out_len))
	return WIND_ERR_OVERRUN;
    while(i < olen && tmp[i] == 0x20) /* skip initial spaces */
	i++;

    while (i < olen) {
	if (tmp[i] == 0x20) {
	    if (put_char(out, &o, 0x20, *out_len) ||
		put_char(out, &o, 0x20, *out_len))
		return WIND_ERR_OVERRUN;
	    while(i < olen && tmp[i] == 0x20) /* skip middle spaces */
		i++;
	} else {
	    if (put_char(out, &o, tmp[i++], *out_len))
		return WIND_ERR_OVERRUN;
	}
    }
    assert(o > 0);

    /* only one spaces at the end */
    if (o == 1 && out[0] == 0x20)
	o = 0;
    else if (out[o - 1] == 0x20) {
	if (out[o - 2] == 0x20)
	    o--;
    } else
	put_char(out, &o, 0x20, *out_len);

    *out_len = o;

    return 0;
}