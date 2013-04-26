
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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define	LIBSTDBUF	"/usr/lib/libstdbuf.so"
#define	LIBSTDBUF32	"/usr/lib32/libstdbuf.so"

extern char *__progname;

static void
usage(int s)
{

	fprintf(stderr, "Usage: %s [-e 0|L|<sz>] [-i 0|L|<sz>] [-o 0|L|<sz>] "
	    "<cmd> [args ...]\n", __progname);
	exit(s);
}

int
main(int argc, char *argv[])
{
	char *ibuf, *obuf, *ebuf;
	char *preload0, *preload1;
	int i;

	ibuf = obuf = ebuf = NULL;
	while ((i = getopt(argc, argv, "e:i:o:")) != -1) {
		switch (i) {
		case 'e':
			ebuf = optarg;
			break;
		case 'i':
			ibuf = optarg;
			break;
		case 'o':
			obuf = optarg;
			break;
		case '?':
		default:
			usage(1);
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		exit(0);

	if (ibuf != NULL && setenv("_STDBUF_I", ibuf, 1) == -1)
		warn("Failed to set environment variable: %s=%s",
		    "_STDBUF_I", ibuf);
	if (obuf != NULL && setenv("_STDBUF_O", obuf, 1) == -1)
		warn("Failed to set environment variable: %s=%s",
		    "_STDBUF_O", obuf);
	if (ebuf != NULL && setenv("_STDBUF_E", ebuf, 1) == -1)
		warn("Failed to set environment variable: %s=%s",
		    "_STDBUF_E", ebuf);

	preload0 = getenv("LD_PRELOAD");
	if (preload0 == NULL)
		i = asprintf(&preload1, "LD_PRELOAD=" LIBSTDBUF);
	else
		i = asprintf(&preload1, "LD_PRELOAD=%s:%s", preload0,
		    LIBSTDBUF);

	if (i < 0 || putenv(preload1) == -1)
		warn("Failed to set environment variable: LD_PRELOAD");

	preload0 = getenv("LD_32_PRELOAD");
	if (preload0 == NULL)
		i = asprintf(&preload1, "LD_32_PRELOAD=" LIBSTDBUF32);
	else
		i = asprintf(&preload1, "LD_32_PRELOAD=%s:%s", preload0,
		    LIBSTDBUF32);

	if (i < 0 || putenv(preload1) == -1)
		warn("Failed to set environment variable: LD_32_PRELOAD");

	execvp(argv[0], argv);
	err(2, "%s", argv[0]);
}