
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
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

static void	usage(void);

int
main(int argc, char *argv[])
{
	int fd;
	int i;
	int rval;
	
	if (argc < 2) {
		usage();
		/* NOTREACHED */
	}
	
	rval = EX_OK;
	for (i = 1; i < argc; ++i) {
		if ((fd = open(argv[i], O_RDONLY)) == -1) {
			warn("open %s", argv[i]);
			if (rval == EX_OK)
				rval = EX_NOINPUT;
			continue;
		}

		if (fsync(fd) == -1) {
			warn("fsync %s", argv[i]);
			if (rval == EX_OK)
				rval = EX_OSERR;
		}
		close(fd);
	}
	exit(rval);
	/* NOTREACHED */
}

static void
usage(void)
{

	fprintf(stderr, "usage: fsync file ...\n");
	exit(EX_USAGE);
	/* NOTREACHED */
}