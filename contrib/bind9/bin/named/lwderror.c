
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

/* $Id: lwderror.c,v 1.12 2007/06/19 23:46:59 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/socket.h>
#include <isc/util.h>

#include <named/types.h>
#include <named/lwdclient.h>

/*%
 * Generate an error packet for the client, schedule a send, and put us in
 * the SEND state.
 *
 * The client->pkt structure will be modified to form an error return.
 * The receiver needs to verify that it is in fact an error, and do the
 * right thing with it.  The opcode will be unchanged.  The result needs
 * to be set before calling this function.
 *
 * The only change this code makes is to set the receive buffer size to the
 * size we use, set the reply bit, and recompute any security information.
 */
void
ns_lwdclient_errorpktsend(ns_lwdclient_t *client, isc_uint32_t _result) {
	isc_result_t result;
	int lwres;
	isc_region_t r;
	lwres_buffer_t b;

	REQUIRE(NS_LWDCLIENT_ISRUNNING(client));

	/*
	 * Since we are only sending the packet header, we can safely toss
	 * the receive buffer.  This means we won't need to allocate space
	 * for sending an error reply.  This is a Good Thing.
	 */
	client->pkt.length = LWRES_LWPACKET_LENGTH;
	client->pkt.pktflags |= LWRES_LWPACKETFLAG_RESPONSE;
	client->pkt.recvlength = LWRES_RECVLENGTH;
	client->pkt.authtype = 0; /* XXXMLG */
	client->pkt.authlength = 0;
	client->pkt.result = _result;

	lwres_buffer_init(&b, client->buffer, LWRES_RECVLENGTH);
	lwres = lwres_lwpacket_renderheader(&b, &client->pkt);
	if (lwres != LWRES_R_SUCCESS) {
		ns_lwdclient_stateidle(client);
		return;
	}

	r.base = client->buffer;
	r.length = b.used;
	client->sendbuf = client->buffer;
	result = ns_lwdclient_sendreply(client, &r);
	if (result != ISC_R_SUCCESS) {
		ns_lwdclient_stateidle(client);
		return;
	}

	NS_LWDCLIENT_SETSEND(client);
}