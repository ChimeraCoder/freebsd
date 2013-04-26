
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

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_aname_to_localname (krb5_context context,
			 krb5_const_principal aname,
			 size_t lnsize,
			 char *lname)
{
    krb5_error_code ret;
    krb5_realm *lrealms, *r;
    int valid;
    size_t len;
    const char *res;

    ret = krb5_get_default_realms (context, &lrealms);
    if (ret)
	return ret;

    valid = 0;
    for (r = lrealms; *r != NULL; ++r) {
	if (strcmp (*r, aname->realm) == 0) {
	    valid = 1;
	    break;
	}
    }
    krb5_free_host_realm (context, lrealms);
    if (valid == 0)
	return KRB5_NO_LOCALNAME;

    if (aname->name.name_string.len == 1)
	res = aname->name.name_string.val[0];
    else if (aname->name.name_string.len == 2
	     && strcmp (aname->name.name_string.val[1], "root") == 0) {
	krb5_principal rootprinc;
	krb5_boolean userok;

	res = "root";

	ret = krb5_copy_principal(context, aname, &rootprinc);
	if (ret)
	    return ret;

	userok = krb5_kuserok(context, rootprinc, res);
	krb5_free_principal(context, rootprinc);
	if (!userok)
	    return KRB5_NO_LOCALNAME;

    } else
	return KRB5_NO_LOCALNAME;

    len = strlen (res);
    if (len >= lnsize)
	return ERANGE;
    strlcpy (lname, res, lnsize);

    return 0;
}