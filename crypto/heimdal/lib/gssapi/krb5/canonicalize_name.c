
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

OM_uint32 GSSAPI_CALLCONV _gsskrb5_canonicalize_name (
            OM_uint32 * minor_status,
            const gss_name_t input_name,
            const gss_OID mech_type,
            gss_name_t * output_name
           )
{
    krb5_context context;
    krb5_principal name;
    OM_uint32 ret;

    *output_name = NULL;

    GSSAPI_KRB5_INIT (&context);

    ret = _gsskrb5_canon_name(minor_status, context, 1, NULL, input_name, &name);
    if (ret)
	return ret;

    *output_name = (gss_name_t)name;

    return GSS_S_COMPLETE;
}