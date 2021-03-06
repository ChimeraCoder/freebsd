
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
static const char sccsid[] = "@(#)network.c	8.2 (Berkeley) 12/15/93";
#endif
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <errno.h>
#include <stdlib.h>

#include <arpa/telnet.h>
#include <unistd.h>

#include "ring.h"

#include "defines.h"
#include "externs.h"
#include "fdset.h"

Ring		netoring, netiring;
unsigned char	netobuf[2*BUFSIZ], netibuf[BUFSIZ];

/*
 * Initialize internal network data structures.
 */

void
init_network(void)
{
    if (ring_init(&netoring, netobuf, sizeof netobuf) != 1) {
	exit(1);
    }
    if (ring_init(&netiring, netibuf, sizeof netibuf) != 1) {
	exit(1);
    }
    NetTrace = stdout;
}


/*
 * Check to see if any out-of-band data exists on a socket (for
 * Telnet "synch" processing).
 */

int
stilloob(void)
{
    static struct timeval timeout = { 0, 0 };
    fd_set	excepts;
    int value;

    do {
	FD_ZERO(&excepts);
	FD_SET(net, &excepts);
	value = select(net+1, (fd_set *)0, (fd_set *)0, &excepts, &timeout);
    } while ((value == -1) && (errno == EINTR));

    if (value < 0) {
	perror("select");
	(void) quit();
	/* NOTREACHED */
    }
    if (FD_ISSET(net, &excepts)) {
	return 1;
    } else {
	return 0;
    }
}


/*
 *  setneturg()
 *
 *	Sets "neturg" to the current location.
 */

void
setneturg(void)
{
    ring_mark(&netoring);
}


/*
 *  netflush
 *		Send as much data as possible to the network,
 *	handling requests for urgent data.
 *
 *		The return value indicates whether we did any
 *	useful work.
 */

int
netflush(void)
{
    int n, n1;

#ifdef	ENCRYPTION
    if (encrypt_output)
	ring_encrypt(&netoring, encrypt_output);
#endif	/* ENCRYPTION */
    if ((n1 = n = ring_full_consecutive(&netoring)) > 0) {
	if (!ring_at_mark(&netoring)) {
	    n = send(net, (char *)netoring.consume, n, 0); /* normal write */
	} else {
	    /*
	     * In 4.2 (and 4.3) systems, there is some question about
	     * what byte in a sendOOB operation is the "OOB" data.
	     * To make ourselves compatible, we only send ONE byte
	     * out of band, the one WE THINK should be OOB (though
	     * we really have more the TCP philosophy of urgent data
	     * rather than the Unix philosophy of OOB data).
	     */
	    n = send(net, (char *)netoring.consume, 1, MSG_OOB);/* URGENT data */
	}
    }
    if (n < 0) {
	if (errno != ENOBUFS && errno != EWOULDBLOCK) {
	    setcommandmode();
	    perror(hostname);
	    (void)NetClose(net);
	    ring_clear_mark(&netoring);
	    ExitString("Connection closed by foreign host.\n", 1);
	    /*NOTREACHED*/
	}
	n = 0;
    }
    if (netdata && n) {
	Dump('>', netoring.consume, n);
    }
    if (n) {
	ring_consumed(&netoring, n);
	/*
	 * If we sent all, and more to send, then recurse to pick
	 * up the other half.
	 */
	if ((n1 == n) && ring_full_consecutive(&netoring)) {
	    (void) netflush();
	}
	return 1;
    } else {
	return 0;
    }
}