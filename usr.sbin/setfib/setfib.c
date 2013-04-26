
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

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

static void usage(void);

int
main(int argc, char *argv[])
{
	long fib = 0;
	int ch;
	char *ep;
	int	numfibs;
	size_t intsize = sizeof(int);

        if (sysctlbyname("net.fibs", &numfibs, &intsize, NULL, 0) == -1)
		errx(1, "Multiple FIBS not supported");
	if (argc < 2)
		usage();
	ep = argv[1];
	/*
	 * convert -N or N to -FN. (N is a number)
	 */
	if (ep[0]== '-' && isdigit((unsigned char)ep[1]))
		ep++;
	if (isdigit((unsigned char)*ep))
               if (asprintf(&argv[1], "-F%s", ep) < 0)
                        err(1, "asprintf");

	while ((ch = getopt(argc, argv, "F:")) != -1) {
		switch (ch) {
		case 'F':
			errno = 0;
			fib = strtol(optarg, &ep, 10);
			if (ep == optarg || *ep != '\0' || errno ||
			    fib < 0 || fib >= numfibs)
				errx(1, "%s: invalid FIB (max %d)",
				    optarg, numfibs - 1);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		usage();

	errno = 0;
	if (setfib((int)fib))
		warn("setfib");
	execvp(*argv, argv);
	err(errno == ENOENT ? 127 : 126, "%s", *argv);
}

static void
usage(void)
{

	(void)fprintf(stderr,
	    "usage: setfib [-[F]]value command\n");
	exit(1);
}