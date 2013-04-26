
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

#include <sys/queue.h>
#include <bluetooth.h>
#include <errno.h>
#include <sdp.h>
#include <string.h>
#include "profile.h"
#include "provider.h"
#include "server.h"

/*
 * Prepare Service Unregister response
 */

int32_t
server_prepare_service_unregister_response(server_p srv, int32_t fd)
{
	uint8_t const	*req = srv->req + sizeof(sdp_pdu_t);
	uint8_t const	*req_end = req + ((sdp_pdu_p)(srv->req))->len;
	uint8_t		*rsp = srv->fdidx[fd].rsp;

	provider_t	*provider = NULL;
	uint32_t	 handle;

	/*
	 * Minimal Service Unregister Request
	 *
	 * value32	- uuid 4 bytes
	 */

	if (!srv->fdidx[fd].control ||
	    !srv->fdidx[fd].priv || req_end - req < 4)
		return (SDP_ERROR_CODE_INVALID_REQUEST_SYNTAX);

	/* Get handle */
	SDP_GET32(handle, req);

	/* Lookup provider */
	provider = provider_by_handle(handle);
	if (provider == NULL || provider->fd != fd)
		return (SDP_ERROR_CODE_INVALID_SERVICE_RECORD_HANDLE);

	provider_unregister(provider);
	SDP_PUT16(0, rsp);

	/* Set reply size */
	srv->fdidx[fd].rsp_limit = srv->fdidx[fd].omtu - sizeof(sdp_pdu_t);
	srv->fdidx[fd].rsp_size = rsp - srv->fdidx[fd].rsp;
	srv->fdidx[fd].rsp_cs = 0;

	return (0);
}