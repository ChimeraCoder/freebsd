
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

/*
 * Return a NULL-terminated list of default realms in `realms'.
 * Free this memory with krb5_free_host_realm.
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_default_realms (krb5_context context,
			 krb5_realm **realms)
{
    if (context->default_realms == NULL) {
	krb5_error_code ret = krb5_set_default_realm (context, NULL);
	if (ret)
	    return KRB5_CONFIG_NODEFREALM;
    }

    return krb5_copy_host_realm (context,
				 context->default_realms,
				 realms);
}

/*
 * Return the first default realm.  For compatibility.
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_default_realm(krb5_context context,
		       krb5_realm *realm)
{
    krb5_error_code ret;
    char *res;

    if (context->default_realms == NULL
	|| context->default_realms[0] == NULL) {
	krb5_clear_error_message(context);
	ret = krb5_set_default_realm (context, NULL);
	if (ret)
	    return ret;
    }

    res = strdup (context->default_realms[0]);
    if (res == NULL) {
	krb5_set_error_message(context, ENOMEM,
			       N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    *realm = res;
    return 0;
}