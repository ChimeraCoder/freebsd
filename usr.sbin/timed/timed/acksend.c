
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)acksend.c	8.1 (Berkeley) 6/6/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include "globals.h"

struct tsp *answer;

extern u_short sequence;

void
xmit(int type, u_int seq, struct sockaddr_in *addr)
{
	static struct tsp msg;

	msg.tsp_type = type;
	msg.tsp_seq = seq;
	msg.tsp_vers = TSPVERSION;
	(void)strcpy(msg.tsp_name, hostname);
	bytenetorder(&msg);
	if (sendto(sock, (char *)&msg, sizeof(struct tsp), 0,
		   (struct sockaddr*)addr, sizeof(struct sockaddr)) < 0) {
		trace_sendto_err(addr->sin_addr);
	}
}


/*
 * Acksend implements reliable datagram transmission by using sequence
 * numbers and retransmission when necessary.
 * If `name' is ANYADDR, this routine implements reliable broadcast.
 *
 * Because this function calls readmsg(), none of its args may be in
 *	a message provided by readmsg().
 * message		this message
 * addr			to here
 * ack			look for this ack
 * net			receive from this network
 * bad			1=losing patience
 */
struct tsp *
acksend(struct tsp *message, struct sockaddr_in *addr, char *name,
	int ack, struct netinfo *net, int bad)
{
	struct timeval twait;
	int count;
	long msec;

	message->tsp_vers = TSPVERSION;
	message->tsp_seq = sequence;
	if (trace) {
		fprintf(fd, "acksend: to %s: ",
			(name == ANYADDR ? "broadcast" : name));
		print(message, addr);
	}
	bytenetorder(message);

	msec = 200;
	count = bad ? 1 : 5;	/* 5 packets in 6.4 seconds */
	answer = 0;
	do {
		if (!answer) {
			/* do not go crazy transmitting just because the
			 * other guy cannot keep our sequence numbers
			 * straight.
			 */
			if (sendto(sock, (char *)message, sizeof(struct tsp),
				   0, (struct sockaddr*)addr,
				   sizeof(struct sockaddr)) < 0) {
				trace_sendto_err(addr->sin_addr);
				break;
			}
		}

		mstotvround(&twait, msec);
		answer  = readmsg(ack, name, &twait, net);
		if (answer != 0) {
			if (answer->tsp_seq != sequence) {
				if (trace)
					fprintf(fd,"acksend: seq # %u!=%u\n",
						answer->tsp_seq, sequence);
				continue;
			}
			break;
		}

		msec *= 2;
	} while (--count > 0);
	sequence++;

	return(answer);
}