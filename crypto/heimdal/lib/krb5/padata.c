
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION PA_DATA * KRB5_LIB_CALL
krb5_find_padata(PA_DATA *val, unsigned len, int type, int *idx)
{
    for(; *idx < (int)len; (*idx)++)
	if(val[*idx].padata_type == (unsigned)type)
	    return val + *idx;
    return NULL;
}

KRB5_LIB_FUNCTION int KRB5_LIB_CALL
krb5_padata_add(krb5_context context, METHOD_DATA *md,
		int type, void *buf, size_t len)
{
    PA_DATA *pa;

    pa = realloc (md->val, (md->len + 1) * sizeof(*md->val));
    if (pa == NULL) {
	krb5_set_error_message(context, ENOMEM,
			       N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    md->val = pa;

    pa[md->len].padata_type = type;
    pa[md->len].padata_value.length = len;
    pa[md->len].padata_value.data = buf;
    md->len++;

    return 0;
}