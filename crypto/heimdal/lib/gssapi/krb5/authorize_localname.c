
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

#include "gsskrb5_locl.h"

OM_uint32 GSSAPI_CALLCONV
_gsskrb5_authorize_localname(OM_uint32 *minor_status,
                             const gss_name_t input_name,
                             gss_const_buffer_t user_name,
                             gss_const_OID user_name_type)
{
    krb5_context context;
    krb5_principal princ = (krb5_principal)input_name;
    char *user;
    int user_ok;

    if (!gss_oid_equal(user_name_type, GSS_C_NT_USER_NAME))
        return GSS_S_BAD_NAMETYPE;

    GSSAPI_KRB5_INIT(&context);

    user = malloc(user_name->length + 1);
    if (user == NULL) {
        *minor_status = ENOMEM;
        return GSS_S_FAILURE;
    }

    memcpy(user, user_name->value, user_name->length);
    user[user_name->length] = '\0';

    *minor_status = 0;
    user_ok = krb5_kuserok(context, princ, user);

    free(user);

    return user_ok ? GSS_S_COMPLETE : GSS_S_UNAUTHORIZED;
}