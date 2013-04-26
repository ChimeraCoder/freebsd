
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
#include <sys/types.h>

#include <stdio.h>
#include <sysexits.h>

#include <fs/nandfs/nandfs_fs.h>
#include <libnandfs.h>

#include "nandfs.h"

static void
mksnap_usage(void)
{

	fprintf(stderr, "usage:\n");
	fprintf(stderr, "\tmksnap node\n");
}

int
nandfs_mksnap(int argc, char **argv)
{
	struct nandfs fs;
	uint64_t cpno;
	int error;

	if (argc != 1) {
		mksnap_usage();
		return (EX_USAGE);
	}

	nandfs_init(&fs, argv[0]);
	error = nandfs_open(&fs);
	if (error == -1) {
		fprintf(stderr, "nandfs_open: %s\n", nandfs_errmsg(&fs));
		goto out;
	}

	error = nandfs_make_snap(&fs, &cpno);
	if (error == -1)
		fprintf(stderr, "nandfs_make_snap: %s\n", nandfs_errmsg(&fs));
	else
		printf("%jd\n", cpno);

out:
	nandfs_close(&fs);
	nandfs_destroy(&fs);
	return (error);
}