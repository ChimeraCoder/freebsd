
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
 * Convert the simple string `s' into a NULL-terminated and freshly allocated
 * list in `list'.  Return an error code.
 */

static krb5_error_code
string_to_list (krb5_context context, const char *s, krb5_realm **list)
{

    *list = malloc (2 * sizeof(**list));
    if (*list == NULL) {
	krb5_set_error_message(context, ENOMEM,
			       N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    (*list)[0] = strdup (s);
    if ((*list)[0] == NULL) {
	free (*list);
	krb5_set_error_message(context, ENOMEM,
			       N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    (*list)[1] = NULL;
    return 0;
}

/*
 * Set the knowledge of the default realm(s) in `context'.
 * If realm != NULL, that's the new default realm.
 * Otherwise, the realm(s) are figured out from configuration or DNS.
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_set_default_realm(krb5_context context,
		       const char *realm)
{
    krb5_error_code ret = 0;
    krb5_realm *realms = NULL;

    if (realm == NULL) {
	realms = krb5_config_get_strings (context, NULL,
					  "libdefaults",
					  "default_realm",
					  NULL);
	if (realms == NULL)
	    ret = krb5_get_host_realm(context, NULL, &realms);
    } else {
	ret = string_to_list (context, realm, &realms);
    }
    if (ret)
	return ret;
    krb5_free_host_realm (context, context->default_realms);
    context->default_realms = realms;
    return 0;
}