
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
gss_encapsulate_token(gss_const_buffer_t input_token,
		      gss_const_OID oid,
		      gss_buffer_t output_token)
{
    GSSAPIContextToken ct;
    int ret;
    size_t size;

    ret = der_get_oid (oid->elements, oid->length, &ct.thisMech, &size);
    if (ret) {
	_mg_buffer_zero(output_token);
	return GSS_S_FAILURE;
    }

    ct.innerContextToken.data = input_token->value;
    ct.innerContextToken.length = input_token->length;

    ASN1_MALLOC_ENCODE(GSSAPIContextToken,
		       output_token->value, output_token->length,
		       &ct, &size, ret);
    der_free_oid(&ct.thisMech);
    if (ret) {
	_mg_buffer_zero(output_token);
	return GSS_S_FAILURE;
    }
    if (output_token->length != size)
	abort();

    return GSS_S_COMPLETE;
}