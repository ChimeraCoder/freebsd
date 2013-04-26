
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

OM_uint32
_gsskrb5_get_tkt_flags(OM_uint32 *minor_status,
		       gsskrb5_ctx ctx,
		       OM_uint32 *tkt_flags)
{
    if (ctx == NULL) {
	*minor_status = EINVAL;
	return GSS_S_NO_CONTEXT;
    }
    HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);

    if (ctx->ticket == NULL) {
	HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);
	*minor_status = EINVAL;
	return GSS_S_BAD_MECH;
    }

    *tkt_flags = TicketFlags2int(ctx->ticket->ticket.flags);
    HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);

    *minor_status = 0;
    return GSS_S_COMPLETE;
}