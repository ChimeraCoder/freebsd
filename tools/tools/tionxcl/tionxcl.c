
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>

#include <libgen.h>
#include <sysexits.h>

#define DEVPATHNAME "/dev"

int
main(int argc, char *argv[])
{
	char *progname;
	char *ttyname;
	int fd;
	int dofree;

	dofree = 0;

	progname = basename(argv[0]);
	if (argc != 2)
		errx(EX_USAGE, "usage: %s <ttyname>\n", progname);
	if (geteuid() != 0)
		errx(EX_NOPERM, "Sorry\n");

	if (argv[1][0] == '/') {
		ttyname = argv[1];
	} else {
		size_t len, maxpath, result;

		len = strlen(argv[1]) + sizeof(DEVPATHNAME) + 1;

		maxpath = pathconf(DEVPATHNAME, _PC_PATH_MAX);
		if (len > maxpath) {
			warnc(ENAMETOOLONG, ttyname);
			exit(EX_DATAERR);
		}

		ttyname = malloc(len);
		if (ttyname == NULL) {
			warnc(ENOMEM, "malloc");
			exit(EX_OSERR);
		}
		dofree = 1;

		result = snprintf(ttyname, len, "%s/%s", DEVPATHNAME, argv[1]);
		if (result >= len)
			warnc(ENOMEM, "snprintf");
	}

	fd = open(ttyname, O_RDWR);
	if (fd == -1) {
		warnc(errno, "open %s", ttyname);
		if (dofree)
			free(ttyname);
		exit(EX_OSERR);
	}

	if (0 != ioctl(fd, TIOCNXCL, 0))
		warnc(errno, "ioctl TIOCNXCL %s", ttyname);

	if (dofree)
		free(ttyname);
	exit(0);
}