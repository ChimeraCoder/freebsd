
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
gss_verify(OM_uint32 *minor_status,
    gss_ctx_id_t context_handle,
    gss_buffer_t message_buffer,
    gss_buffer_t token_buffer,
    int *qop_state)
{

	return (gss_verify_mic(minor_status,
		    context_handle, message_buffer, token_buffer,
		    (gss_qop_t *)qop_state));
}