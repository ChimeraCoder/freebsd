
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

#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bluetooth.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sdp-int.h>
#include <sdp.h>

static int32_t sdp_receive_error_pdu(sdp_session_p ss);

int32_t
sdp_register_service(void *xss, uint16_t uuid, bdaddr_p const bdaddr,
		uint8_t const *data, uint32_t datalen, uint32_t *handle)
{
	sdp_session_p	ss = (sdp_session_p) xss;
	struct iovec	iov[4];
	sdp_pdu_t	pdu;
	int32_t		len;

	if (ss == NULL)
		return (-1);
	if (bdaddr == NULL || data == NULL ||
	    datalen == 0 || !(ss->flags & SDP_SESSION_LOCAL)) {
		ss->error = EINVAL;
		return (-1);
	}
	if (sizeof(pdu)+sizeof(uuid)+sizeof(*bdaddr)+datalen > SDP_LOCAL_MTU) {
		ss->error = EMSGSIZE;
		return (-1);
	}

	pdu.pid = SDP_PDU_SERVICE_REGISTER_REQUEST;
	pdu.tid = htons(++ss->tid);
	pdu.len = htons(sizeof(uuid) + sizeof(*bdaddr) + datalen);

	uuid = htons(uuid);

	iov[0].iov_base = (void *) &pdu;
	iov[0].iov_len = sizeof(pdu);

	iov[1].iov_base = (void *) &uuid;
	iov[1].iov_len = sizeof(uuid);

	iov[2].iov_base = (void *) bdaddr;
	iov[2].iov_len = sizeof(*bdaddr);

	iov[3].iov_base = (void *) data;
	iov[3].iov_len = datalen;

	do {
		len = writev(ss->s, iov, sizeof(iov)/sizeof(iov[0]));
	} while (len < 0 && errno == EINTR);

	if (len < 0) {
		ss->error = errno;
		return (-1);
	}

	len = sdp_receive_error_pdu(ss);
	if (len < 0)
		return (-1);
	if (len != sizeof(pdu) + sizeof(uint16_t) + sizeof(uint32_t)) {
		ss->error = EIO;
		return (-1);
	}

	if (handle != NULL) {
		*handle  = (uint32_t) ss->rsp[--len];
		*handle |= (uint32_t) ss->rsp[--len] << 8;
		*handle |= (uint32_t) ss->rsp[--len] << 16;
		*handle |= (uint32_t) ss->rsp[--len] << 24;
	}

	return (0);
}

int32_t
sdp_unregister_service(void *xss, uint32_t handle)
{
	sdp_session_p	ss = (sdp_session_p) xss;
	struct iovec	iov[2];
	sdp_pdu_t	pdu;
	int32_t		len;

	if (ss == NULL)
		return (-1);
	if (!(ss->flags & SDP_SESSION_LOCAL)) {
		ss->error = EINVAL;
		return (-1);
	}
	if (sizeof(pdu) + sizeof(handle) > SDP_LOCAL_MTU) {
		ss->error = EMSGSIZE;
		return (-1);
	}

	pdu.pid = SDP_PDU_SERVICE_UNREGISTER_REQUEST;
	pdu.tid = htons(++ss->tid);
	pdu.len = htons(sizeof(handle));

	handle = htonl(handle);

	iov[0].iov_base = (void *) &pdu;
	iov[0].iov_len = sizeof(pdu);

	iov[1].iov_base = (void *) &handle;
	iov[1].iov_len = sizeof(handle);

	do {
		len = writev(ss->s, iov, sizeof(iov)/sizeof(iov[0]));
	} while (len < 0 && errno == EINTR);

	if (len < 0) {
		ss->error = errno;
		return (-1);
	}

	return ((sdp_receive_error_pdu(ss) < 0)? -1 : 0);
}

int32_t
sdp_change_service(void *xss, uint32_t handle,
		uint8_t const *data, uint32_t datalen)
{
	sdp_session_p	ss = (sdp_session_p) xss;
	struct iovec	iov[3];
	sdp_pdu_t	pdu;
	int32_t		len;

	if (ss == NULL)
		return (-1);
	if (data == NULL || datalen == 0 || !(ss->flags & SDP_SESSION_LOCAL)) {
		ss->error = EINVAL;
		return (-1);
	}
	if (sizeof(pdu) + sizeof(handle) + datalen > SDP_LOCAL_MTU) {
		ss->error = EMSGSIZE;
		return (-1);
	}

	pdu.pid = SDP_PDU_SERVICE_CHANGE_REQUEST;
	pdu.tid = htons(++ss->tid);
	pdu.len = htons(sizeof(handle) + datalen);

	handle = htons(handle);

	iov[0].iov_base = (void *) &pdu;
	iov[0].iov_len = sizeof(pdu);

	iov[1].iov_base = (void *) &handle;
	iov[1].iov_len = sizeof(handle);

	iov[2].iov_base = (void *) data;
	iov[2].iov_len = datalen;

	do {
		len = writev(ss->s, iov, sizeof(iov)/sizeof(iov[0]));
	} while (len < 0 && errno == EINTR);

	if (len < 0) {
		ss->error = errno;
		return (-1);
	}

	return ((sdp_receive_error_pdu(ss) < 0)? -1 : 0);
}

static int32_t
sdp_receive_error_pdu(sdp_session_p ss)
{
	sdp_pdu_p	pdu;
	int32_t		len;
	uint16_t	error;

	do {
		len = read(ss->s, ss->rsp, ss->rsp_e - ss->rsp);
	} while (len < 0 && errno == EINTR);

	if (len < 0) {
		ss->error = errno;
		return (-1);
	}

	pdu = (sdp_pdu_p) ss->rsp;
	pdu->tid = ntohs(pdu->tid);
	pdu->len = ntohs(pdu->len);

	if (pdu->pid != SDP_PDU_ERROR_RESPONSE || pdu->tid != ss->tid ||
	    pdu->len < 2 || pdu->len != len - sizeof(*pdu)) {
		ss->error = EIO;
		return (-1);
	}

	error  = (uint16_t) ss->rsp[sizeof(pdu)] << 8;
	error |= (uint16_t) ss->rsp[sizeof(pdu) + 1];

	if (error != 0) {
		ss->error = EIO;
		return (-1);
	}

	return (len);
}