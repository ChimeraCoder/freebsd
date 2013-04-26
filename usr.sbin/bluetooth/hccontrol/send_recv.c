
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/endian.h>
#include <assert.h>
#include <errno.h>
#include <netgraph/bluetooth/include/ng_hci.h>
#include <string.h>
#include <unistd.h>
#include "hccontrol.h"

/* Send HCI request to the unit */
int
hci_request(int s, int opcode, char const *cp, int cp_size, char *rp, int *rp_size)
{
	char			 buffer[512];
	int			 n;
	ng_hci_cmd_pkt_t	*c = (ng_hci_cmd_pkt_t *) buffer;
	ng_hci_event_pkt_t	*e = (ng_hci_event_pkt_t *) buffer;

	assert(rp != NULL);
	assert(rp_size != NULL);
	assert(*rp_size > 0);

	c->type = NG_HCI_CMD_PKT;
	c->opcode = (uint16_t) opcode;
	c->opcode = htole16(c->opcode);

	if (cp != NULL) {
		assert(0 < cp_size && cp_size <= NG_HCI_CMD_PKT_SIZE);

		c->length = (uint8_t) cp_size;
		memcpy(buffer + sizeof(*c), cp, cp_size);
	} else
		c->length = 0;

	if (hci_send(s, buffer, sizeof(*c) + cp_size) == ERROR)
		return (ERROR);

again:
	n = sizeof(buffer);
	if (hci_recv(s, buffer, &n) == ERROR)
		return (ERROR);

	if (n < sizeof(*e)) {
		errno = EMSGSIZE;
		return (ERROR);
	}

	if (e->type != NG_HCI_EVENT_PKT) {
		errno = EIO;
		return (ERROR);
	}

	switch (e->event) {
	case NG_HCI_EVENT_COMMAND_COMPL: {
		ng_hci_command_compl_ep	*cc = 
				(ng_hci_command_compl_ep *)(e + 1);

		cc->opcode = le16toh(cc->opcode);

		if (cc->opcode == 0x0000 || cc->opcode != opcode)
			goto again; 

		n -= (sizeof(*e) + sizeof(*cc));
		if (n < *rp_size)
			*rp_size = n;

		memcpy(rp, buffer + sizeof(*e) + sizeof(*cc), *rp_size);
		} break;

	case NG_HCI_EVENT_COMMAND_STATUS: {
		ng_hci_command_status_ep	*cs = 
				(ng_hci_command_status_ep *)(e + 1);

		cs->opcode = le16toh(cs->opcode);

		if (cs->opcode == 0x0000 || cs->opcode != opcode)
			goto again; 

		*rp_size = 1;
		*rp = cs->status;
		} break;

	default:
		goto again;
	}

	return (OK);
} /* hci_request */

/* Send simple HCI request - Just HCI command packet (no parameters) */
int
hci_simple_request(int s, int opcode, char *rp, int *rp_size)
{
	return (hci_request(s, opcode, NULL, 0, rp, rp_size));
} /* hci_simple_request */

/* Send HCI data to the unit */
int
hci_send(int s, char const *buffer, int size)
{
	assert(buffer != NULL);
	assert(size >= sizeof(ng_hci_cmd_pkt_t));
	assert(size <= sizeof(ng_hci_cmd_pkt_t) + NG_HCI_CMD_PKT_SIZE);

	if (send(s, buffer, size, 0) < 0)
		return (ERROR);

	return (OK);
} /* hci_send */

/* Receive HCI data from the unit */
int
hci_recv(int s, char *buffer, int *size)
{
	struct timeval	tv;
	fd_set		rfd;
	int		n;

	assert(buffer != NULL);
	assert(size != NULL);
	assert(*size > sizeof(ng_hci_event_pkt_t));

again:
	FD_ZERO(&rfd);
	FD_SET(s, &rfd);

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	n = select(s + 1, &rfd, NULL, NULL, &tv);
	if (n <= 0) {
		if (n < 0) {
			if (errno == EINTR)
				goto again;
		} else
			errno = ETIMEDOUT;

		return (ERROR);
	}

	assert(FD_ISSET(s, &rfd));

	n = recv(s, buffer, *size, 0);
	if (n < 0)
		return (ERROR);

	*size = n;

	return (OK);
} /* hci_recv */