
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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "nandfs.h"

static void
usage(void)
{

	fprintf(stderr, "usage: nandfs [lssnap | mksnap | rmsnap <snap>] "
	    "node\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	int error = 0;
	char *cmd;

	if (argc < 2)
		usage();

	cmd = argv[1];
	argc -= 2;
	argv += 2;

	if (strcmp(cmd, "lssnap") == 0)
		error = nandfs_lssnap(argc, argv);
	else if (strcmp(cmd, "mksnap") == 0)
		error = nandfs_mksnap(argc, argv);
	else if (strcmp(cmd, "rmsnap") == 0)
		error = nandfs_rmsnap(argc, argv);
	else
		usage();

	return (error);
}