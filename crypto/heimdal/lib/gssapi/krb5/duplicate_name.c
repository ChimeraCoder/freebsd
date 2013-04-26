
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

OM_uint32 GSSAPI_CALLCONV _gsskrb5_duplicate_name (
            OM_uint32 * minor_status,
            const gss_name_t src_name,
            gss_name_t * dest_name
           )
{
    krb5_const_principal src = (krb5_const_principal)src_name;
    krb5_context context;
    krb5_principal dest;
    krb5_error_code kret;

    GSSAPI_KRB5_INIT (&context);

    kret = krb5_copy_principal (context, src, &dest);
    if (kret) {
	*minor_status = kret;
	return GSS_S_FAILURE;
    } else {
	*dest_name = (gss_name_t)dest;
	*minor_status = 0;
	return GSS_S_COMPLETE;
    }
}