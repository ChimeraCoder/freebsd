
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
 * Free all memory allocated by `realmlist'
 *
 * @param context A Kerberos 5 context.
 * @param realmlist realmlist to free, NULL is ok
 *
 * @return a Kerberos error code, always 0.
 *
 * @ingroup krb5_support
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_free_host_realm(krb5_context context,
		     krb5_realm *realmlist)
{
    krb5_realm *p;

    if(realmlist == NULL)
	return 0;
    for (p = realmlist; *p; ++p)
	free (*p);
    free (realmlist);
    return 0;
}