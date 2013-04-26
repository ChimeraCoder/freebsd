
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

#include "mech_locl.h"

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_decapsulate_token(gss_const_buffer_t input_token,
		      gss_const_OID oid,
		      gss_buffer_t output_token)
{
    GSSAPIContextToken ct;
    heim_oid o;
    OM_uint32 status;
    int ret;
    size_t size;

    _mg_buffer_zero(output_token);

    ret = der_get_oid (oid->elements, oid->length, &o, &size);
    if (ret)
	return GSS_S_FAILURE;

    ret = decode_GSSAPIContextToken(input_token->value, input_token->length,
				    &ct, NULL);
    if (ret) {
	der_free_oid(&o);
	return GSS_S_FAILURE;
    }

    if (der_heim_oid_cmp(&ct.thisMech, &o) == 0) {
	status = GSS_S_COMPLETE;
	output_token->value = ct.innerContextToken.data;
	output_token->length = ct.innerContextToken.length;
	der_free_oid(&ct.thisMech);
    } else {
	free_GSSAPIContextToken(&ct);
 	status = GSS_S_FAILURE;
    }
    der_free_oid(&o);

    return status;
}