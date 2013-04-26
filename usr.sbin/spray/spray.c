
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
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rpc/rpc.h>
#include <rpcsvc/spray.h>

#ifndef SPRAYOVERHEAD
#define SPRAYOVERHEAD	86
#endif

static void usage(void);
static void print_xferstats(unsigned int, int, double);

/* spray buffer */
static char spray_buffer[SPRAYMAX];

/* RPC timeouts */
static struct timeval NO_DEFAULT = { -1, -1 };
static struct timeval ONE_WAY = { 0, 0 };
static struct timeval TIMEOUT = { 25, 0 };

int
main(int argc, char *argv[])
{
	spraycumul	host_stats;
	sprayarr	host_array;
	CLIENT *cl;
	int c;
	u_int i;
	u_int count = 0;
	int delay = 0;
	int length = 0;
	double xmit_time;			/* time to receive data */

	while ((c = getopt(argc, argv, "c:d:l:")) != -1) {
		switch (c) {
		case 'c':
			count = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'l':
			length = atoi(optarg);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage();
		/* NOTREACHED */
	}


	/* Correct packet length. */
	if (length > SPRAYMAX) {
		length = SPRAYMAX;
	} else if (length < SPRAYOVERHEAD) {
		length = SPRAYOVERHEAD;
	} else {
		/* The RPC portion of the packet is a multiple of 32 bits. */
		length -= SPRAYOVERHEAD - 3;
		length &= ~3;
		length += SPRAYOVERHEAD;
	}


	/*
	 * The default value of count is the number of packets required
	 * to make the total stream size 100000 bytes.
	 */
	if (!count) {
		count = 100000 / length;
	}

	/* Initialize spray argument */
	host_array.sprayarr_len = length - SPRAYOVERHEAD;
	host_array.sprayarr_val = spray_buffer;
	

	/* create connection with server */
	cl = clnt_create(*argv, SPRAYPROG, SPRAYVERS, "udp");
	if (cl == NULL)
		errx(1, "%s", clnt_spcreateerror(""));


	/*
	 * For some strange reason, RPC 4.0 sets the default timeout, 
	 * thus timeouts specified in clnt_call() are always ignored.  
	 *
	 * The following (undocumented) hack resets the internal state
	 * of the client handle.
	 */
	clnt_control(cl, CLSET_TIMEOUT, &NO_DEFAULT);


	/* Clear server statistics */
	if (clnt_call(cl, SPRAYPROC_CLEAR, (xdrproc_t)xdr_void, NULL,
		(xdrproc_t)xdr_void, NULL, TIMEOUT) != RPC_SUCCESS)
		errx(1, "%s", clnt_sperror(cl, ""));


	/* Spray server with packets */
	printf ("sending %u packets of lnth %d to %s ...", count, length,
	    *argv);
	fflush (stdout);

	for (i = 0; i < count; i++) {
		clnt_call(cl, SPRAYPROC_SPRAY, (xdrproc_t)xdr_sprayarr,
		    &host_array, (xdrproc_t)xdr_void, NULL, ONE_WAY);

		if (delay) {
			usleep(delay);
		}
	}


	/* Collect statistics from server */
	if (clnt_call(cl, SPRAYPROC_GET, (xdrproc_t)xdr_void, NULL,
		(xdrproc_t)xdr_spraycumul, &host_stats, TIMEOUT) != RPC_SUCCESS)
		errx(1, "%s", clnt_sperror(cl, ""));

	xmit_time = host_stats.clock.sec +
			(host_stats.clock.usec / 1000000.0);

	printf ("\n\tin %.2f seconds elapsed time\n", xmit_time);


	/* report dropped packets */
	if (host_stats.counter != count) {
		int packets_dropped = count - host_stats.counter;

		printf("\t%d packets (%.2f%%) dropped\n",
			packets_dropped,
			100.0 * packets_dropped / count );
	} else {
		printf("\tno packets dropped\n");
	}

	printf("Sent:");
	print_xferstats(count, length, xmit_time);

	printf("Rcvd:");
	print_xferstats(host_stats.counter, length, xmit_time);
	
	exit (0);
}


static void
print_xferstats(u_int packets, int packetlen, double xfertime)
{
	int datalen;
	double pps;		/* packets per second */
	double bps;		/* bytes per second */

	datalen = packets * packetlen;
	pps = packets / xfertime;
	bps = datalen / xfertime;

	printf("\t%.0f packets/sec, ", pps);

	if (bps >= 1024) 
		printf ("%.1fK ", bps / 1024);
	else
		printf ("%.0f ", bps);
	
	printf("bytes/sec\n");
}


static void
usage(void)
{
	fprintf(stderr,
		"usage: spray [-c count] [-l length] [-d delay] host\n");
	exit(1);
}