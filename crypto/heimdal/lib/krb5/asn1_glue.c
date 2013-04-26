
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

/*
 *
 */

#include "krb5_locl.h"

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
_krb5_principal2principalname (PrincipalName *p,
			       const krb5_principal from)
{
    return copy_PrincipalName(&from->name, p);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
_krb5_principalname2krb5_principal (krb5_context context,
				    krb5_principal *principal,
				    const PrincipalName from,
				    const Realm realm)
{
    krb5_error_code ret;
    krb5_principal p;

    p = malloc(sizeof(*p));
    if (p == NULL)
	return ENOMEM;
    ret = copy_PrincipalName(&from, &p->name);
    if (ret) {
	free(p);
	return ret;
    }
    p->realm = strdup(realm);
    if (p->realm == NULL) {
	free_PrincipalName(&p->name);
        free(p);
	return ENOMEM;
    }
    *principal = p;
    return 0;
}