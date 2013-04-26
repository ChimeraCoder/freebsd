
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

OM_uint32 GSSAPI_CALLCONV _gsskrb5_process_context_token (
	OM_uint32          *minor_status,
	const gss_ctx_id_t context_handle,
	const gss_buffer_t token_buffer
    )
{
    krb5_context context;
    OM_uint32 ret = GSS_S_FAILURE;
    gss_buffer_desc empty_buffer;

    empty_buffer.length = 0;
    empty_buffer.value = NULL;

    GSSAPI_KRB5_INIT (&context);

    ret = _gsskrb5_verify_mic_internal(minor_status,
				       (gsskrb5_ctx)context_handle,
				       context,
				       token_buffer, &empty_buffer,
				       GSS_C_QOP_DEFAULT,
				       "\x01\x02");

    if (ret == GSS_S_COMPLETE)
	ret = _gsskrb5_delete_sec_context(minor_status,
					  rk_UNCONST(&context_handle),
					  GSS_C_NO_BUFFER);
    if (ret == GSS_S_COMPLETE)
	*minor_status = 0;

    return ret;
}