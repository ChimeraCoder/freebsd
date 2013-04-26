
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

#include "ntlm.h"

OM_uint32 GSSAPI_CALLCONV
_gss_ntlm_export_sec_context (
    OM_uint32 * minor_status,
    gss_ctx_id_t * context_handle,
    gss_buffer_t interprocess_token
    )
{
    if (minor_status)
	*minor_status = 0;
    if (interprocess_token) {
	interprocess_token->length = 0;
	interprocess_token->value = NULL;
    }
    return GSS_S_COMPLETE;
}