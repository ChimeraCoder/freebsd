
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)tftp.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Protocol Machines
 */
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <arpa/tftp.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "tftp.h"
#include "tftp-file.h"
#include "tftp-utils.h"
#include "tftp-io.h"
#include "tftp-transfer.h"
#include "tftp-options.h"

/*
 * Send the requested file.
 */
void
xmitfile(int peer, char *port, int fd, char *name, char *mode)
{
	struct tftphdr *rp;
	int n, i;
	uint16_t block;
	struct sockaddr_storage serv;	/* valid server port number */
	char recvbuffer[MAXPKTSIZE];
	struct tftp_stats tftp_stats;

	stats_init(&tftp_stats);

	memset(&serv, 0, sizeof(serv));
	rp = (struct tftphdr *)recvbuffer;

	if (port == NULL) {
		struct servent *se;
		se = getservbyname("tftp", "udp");
		((struct sockaddr_in *)&peer_sock)->sin_port = se->s_port;
	} else
		((struct sockaddr_in *)&peer_sock)->sin_port =
		    htons(atoi(port));

	for (i = 0; i < 12; i++) {
		struct sockaddr_storage from;

		/* Tell the other side what we want to do */
		if (debug&DEBUG_SIMPLE)
			printf("Sending %s\n", name);

		n = send_wrq(peer, name, mode);
		if (n > 0) {
			printf("Cannot send WRQ packet\n");
			return;
		}

		/*
		 * The first packet we receive has the new destination port
		 * we have to send the next packets to.
		 */
		n = receive_packet(peer, recvbuffer,
		    MAXPKTSIZE, &from, timeoutpacket);

		/* We got some data! */
		if (n >= 0) {
			((struct sockaddr_in *)&peer_sock)->sin_port =
			    ((struct sockaddr_in *)&from)->sin_port;
			break;
		}

		/* This should be retried */
		if (n == RP_TIMEOUT) {
			printf("Try %d, didn't receive answer from remote.\n",
			    i + 1);
			continue;
		}

		/* Everything else is fatal */
		break;
	}
	if (i == 12) {
		printf("Transfer timed out.\n");
		return;
	}
	if (rp->th_opcode == ERROR) {
		printf("Got ERROR, aborted\n");
		return;
	}

	/*
	 * If the first packet is an OACK instead of an ACK packet,
	 * handle it different.
	 */
	if (rp->th_opcode == OACK) {
		if (!options_rfc_enabled) {
			printf("Got OACK while options are not enabled!\n");
			send_error(peer, EBADOP);
			return;
		}

		parse_options(peer, rp->th_stuff, n + 2);
	}

	if (read_init(fd, NULL, mode) < 0) {
		warn("read_init()");
		return;
	}

	block = 1;
	tftp_send(peer, &block, &tftp_stats);

	read_close();
	if (tftp_stats.amount > 0)
		printstats("Sent", verbose, &tftp_stats);

	txrx_error = 1;
}

/*
 * Receive a file.
 */
void
recvfile(int peer, char *port, int fd, char *name, char *mode)
{
	struct tftphdr *rp;
	uint16_t block;
	char recvbuffer[MAXPKTSIZE];
	int n, i;
	struct tftp_stats tftp_stats;

	stats_init(&tftp_stats);

	rp = (struct tftphdr *)recvbuffer;

	if (port == NULL) {
		struct servent *se;
		se = getservbyname("tftp", "udp");
		((struct sockaddr_in *)&peer_sock)->sin_port = se->s_port;
	} else
		((struct sockaddr_in *)&peer_sock)->sin_port =
		    htons(atoi(port));

	for (i = 0; i < 12; i++) {
		struct sockaddr_storage from;

		/* Tell the other side what we want to do */
		if (debug&DEBUG_SIMPLE)
			printf("Requesting %s\n", name);

		n = send_rrq(peer, name, mode);
		if (n > 0) {
			printf("Cannot send RRQ packet\n");
			return;
		}

		/*
		 * The first packet we receive has the new destination port
		 * we have to send the next packets to.
		 */
		n = receive_packet(peer, recvbuffer,
		    MAXPKTSIZE, &from, timeoutpacket);

		/* We got something useful! */
		if (n >= 0) {
			((struct sockaddr_in *)&peer_sock)->sin_port =
			    ((struct sockaddr_in *)&from)->sin_port;
			break;
		}

		/* We should retry if this happens */
		if (n == RP_TIMEOUT) {
			printf("Try %d, didn't receive answer from remote.\n",
			    i + 1);
			continue;
		}

		/* Otherwise it is a fatal error */
		break;
	}
	if (i == 12) {
		printf("Transfer timed out.\n");
		return;
	}
	if (rp->th_opcode == ERROR) {
		tftp_log(LOG_ERR, "Error code %d: %s", rp->th_code, rp->th_msg);
		return;
	}

	if (write_init(fd, NULL, mode) < 0) {
		warn("write_init");
		return;
	}

	/*
	 * If the first packet is an OACK packet instead of an DATA packet,
	 * handle it different.
	 */
	if (rp->th_opcode == OACK) {
		if (!options_rfc_enabled) {
			printf("Got OACK while options are not enabled!\n");
			send_error(peer, EBADOP);
			return;
		}

		parse_options(peer, rp->th_stuff, n + 2);

		n = send_ack(peer, 0);
		if (n > 0) {
			printf("Cannot send ACK on OACK.\n");
			return;
		}
		block = 0;
		tftp_receive(peer, &block, &tftp_stats, NULL, 0);
	} else {
		block = 1;
		tftp_receive(peer, &block, &tftp_stats, rp, n);
	}

	write_close();
	if (tftp_stats.amount > 0)
		printstats("Received", verbose, &tftp_stats);
	return;
}