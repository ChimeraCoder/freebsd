
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <timeconv.h>
#include <unistd.h>

#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

static void
usage(void)
{
	fprintf(stderr, "usage: yppoll [-h host] [-d domainname] mapname\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *domainname;
        char *hostname = "localhost";
        char *inmap, *master;
        int order;
	int c, r;
	time_t t;

        yp_get_default_domain(&domainname);

	while ((c = getopt(argc, argv, "h:d:")) != -1)
		switch (c) {
		case 'd':
                        domainname = optarg;
			break;
                case 'h':
                        hostname = optarg;
                        break;
                case '?':
                        usage();
                        /*NOTREACHED*/
		}

	if (optind + 1 != argc)
		usage();

	inmap = argv[optind];

	r = yp_order(domainname, inmap, &order);
        if (r != 0)
		errx(1, "no such map %s. Reason: %s", inmap, yperr_string(r));
	t = _int_to_time(order);
        printf("Map %s has order number %d. %s", inmap, order, ctime(&t));
	r = yp_master(domainname, inmap, &master);
        if (r != 0)
		errx(1, "no such map %s. Reason: %s", inmap, yperr_string(r));
        printf("The master server is %s.\n", master);

        exit(0);
}