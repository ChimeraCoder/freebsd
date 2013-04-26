
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

OM_uint32 GSSAPI_CALLCONV _gsskrb5_compare_name
           (OM_uint32 * minor_status,
            const gss_name_t name1,
            const gss_name_t name2,
            int * name_equal
           )
{
    krb5_const_principal princ1 = (krb5_const_principal)name1;
    krb5_const_principal princ2 = (krb5_const_principal)name2;
    krb5_context context;

    GSSAPI_KRB5_INIT(&context);

    *name_equal = krb5_principal_compare (context,
					  princ1, princ2);
    *minor_status = 0;
    return GSS_S_COMPLETE;
}