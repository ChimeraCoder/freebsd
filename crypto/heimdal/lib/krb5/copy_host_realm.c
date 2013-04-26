
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

/**
 * Copy the list of realms from `from' to `to'.
 *
 * @param context Kerberos 5 context.
 * @param from list of realms to copy from.
 * @param to list of realms to copy to, free list of krb5_free_host_realm().
 *
 * @return Returns 0 to indicate success. Otherwise an kerberos et
 * error code is returned, see krb5_get_error_message().
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_host_realm(krb5_context context,
		     const krb5_realm *from,
		     krb5_realm **to)
{
    unsigned int n, i;
    const krb5_realm *p;

    for (n = 1, p = from; *p != NULL; ++p)
	++n;

    *to = calloc (n, sizeof(**to));
    if (*to == NULL) {
	krb5_set_error_message (context, ENOMEM,
				N_("malloc: out of memory", ""));
	return ENOMEM;
    }

    for (i = 0, p = from; *p != NULL; ++p, ++i) {
	(*to)[i] = strdup(*p);
	if ((*to)[i] == NULL) {
	    krb5_free_host_realm (context, *to);
	    krb5_set_error_message (context, ENOMEM,
				    N_("malloc: out of memory", ""));
	    return ENOMEM;
	}
    }
    return 0;
}