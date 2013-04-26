
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

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "tcpp.h"

#define	BYTES_DEFAULT	10*1024*1024	/* Data per connection. */
#define	MAXTCPS_DEFAULT	32		/* Number of TCPs at a time per proc. */
#define	PROCS_DEFAULT	1		/* Processes used in run. */
#define	TCPS_DEFAULT	1		/* Number of connections per process. */
#define	BASEPORT_DEFAULT	10000

struct sockaddr_in remoteip; 		/* Base target address. */
struct sockaddr_in localipbase;		/* Base local address, if -l. */
int cflag, hflag, lflag, mflag, pflag, sflag, tflag, Mflag, Pflag;
uint64_t bflag;
u_short rflag;

static void
usage(void)
{

	fprintf(stderr, "client: tcpp"
	    " -c remoteIP"
	    " [-h]"
	    " [-P]"
	    " [-M localIPcount]"
	    " [-l localIPbase]"
	    "\n\t"
	    " [-b bytespertcp]"
	    " [-m maxtcpsatonce]"
	    " [-p procs]"
	    " [-t tcpsperproc]"
	    "\n"
	    "\t"
	    " [-r baseport]"
	    "\n");

	fprintf(stderr, "server: tcpp"
	    " -s"
	    " [-P]"
	    " [-l localIPbase]"
	    " [-m maxtcpsatonce]"
	    " [-p procs]"
	    "\n"
	    "\t"
	    " [-r baseport]"
	    "\n");
	exit(EX_USAGE);
}

int
main(int argc, char *argv[])
{
	long long ll;
	char *dummy;
	int ch;

	bzero(&localipbase, sizeof(localipbase));
	localipbase.sin_len = sizeof(localipbase);
	localipbase.sin_family = AF_INET;
	localipbase.sin_addr.s_addr = htonl(INADDR_ANY);	/* Default. */
	localipbase.sin_port = htons(0);				/* Default. */

	bzero(&remoteip, sizeof(remoteip));
	remoteip.sin_len = sizeof(remoteip);
	remoteip.sin_family = AF_INET;
	remoteip.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* Default. */
	remoteip.sin_port = htons(0);				/* Default. */

	bflag = BYTES_DEFAULT;
	mflag = MAXTCPS_DEFAULT;
	pflag = PROCS_DEFAULT;
	rflag = BASEPORT_DEFAULT;
	tflag = TCPS_DEFAULT;
	Mflag = 1;
	while ((ch = getopt(argc, argv, "b:c:hl:m:p:r:st:CM:PT")) != -1) {
		switch (ch) {
		case 'b':
			ll = strtoll(optarg, &dummy, 10);
			if (*dummy != '\0' || ll <= 0)
				usage();
			bflag = ll;
			break;

		case 'c':
			cflag++;
			if (inet_aton(optarg, &remoteip.sin_addr) != 1)
				err(-1, "inet_aton: %s", optarg);
			break;

		case 'h':
			hflag++;
			break;

		case 'l':
			lflag++;
			if (inet_aton(optarg, &localipbase.sin_addr) != 1)
				err(-1, "inet_aton: %s", optarg);
			break;

		case 'm':
			ll = strtoll(optarg, &dummy, 10);
			if (*dummy != '\0' || ll <= 0)
				usage();
			mflag = ll;
			break;

		case 'p':
			ll = strtoll(optarg, &dummy, 10);
			if (*dummy != '\0' || ll <= 0)
				usage();
			pflag = ll;
			break;

		case 'r':
			ll = strtol(optarg, &dummy, 10);
			if (*dummy != '\0' || ll < 1 || ll > 65535)
				usage();
			rflag = ll;
			break;

		case 's':
			sflag++;
			break;

		case 't':
			ll = strtoll(optarg, &dummy, 10);
			if (*dummy != '\0' || ll <= 0)
				usage();
			tflag = ll;
			break;

		case 'M':
			ll = strtoll(optarg, &dummy, 10);
			if (*dummy != '\0' || ll <= 1)
				usage();
			Mflag = ll;
			break;

		case 'P':
#if defined(CPU_SETSIZE) && 0
			Pflag++;
			break;
#else
			errx(EX_USAGE, "-P current unsupported");
#endif

		default:
			usage();
		}
	}

	/* Exactly one of client and server. */
	if (cflag > 1 || sflag > 1)
		usage();
	if ((cflag && sflag) || (!cflag && !sflag))
		usage();

	/* If Mflag is specified, we must have the lflag for a local IP. */
	if (Mflag > 1 && !lflag)
		usage();

	/* Several flags are valid only on the client, disallow if server. */
	if (sflag && (hflag || Mflag > 1))
		usage();

	if (cflag)
		tcpp_client();
	else
		tcpp_server();
	exit(0);
}