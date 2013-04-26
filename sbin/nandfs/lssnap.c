
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
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>

#include <fs/nandfs/nandfs_fs.h>
#include <libnandfs.h>

#include "nandfs.h"

#define NCPINFO	512

static void
lssnap_usage(void)
{

	fprintf(stderr, "usage:\n");
	fprintf(stderr, "\tlssnap node\n");
}

static void
print_cpinfo(struct nandfs_cpinfo *cpinfo)
{
	struct tm tm;
	time_t t;
	char timebuf[128];

	t = (time_t)cpinfo->nci_create;
	localtime_r(&t, &tm);
	strftime(timebuf, sizeof(timebuf), "%F %T", &tm);

	printf("%20llu  %s\n", (unsigned long long)cpinfo->nci_cno, timebuf);
}

int
nandfs_lssnap(int argc, char **argv)
{
	struct nandfs_cpinfo *cpinfos;
	struct nandfs fs;
	uint64_t next;
	int error, nsnap, i;

	if (argc != 1) {
		lssnap_usage();
		return (EX_USAGE);
	}

	cpinfos = malloc(sizeof(*cpinfos) * NCPINFO);
	if (cpinfos == NULL) {
		fprintf(stderr, "cannot allocate memory\n");
		return (-1);
	}

	nandfs_init(&fs, argv[0]);
	error = nandfs_open(&fs);
	if (error == -1) {
		fprintf(stderr, "nandfs_open: %s\n", nandfs_errmsg(&fs));
		goto out;
	}

	for (next = 1; next != 0; next = cpinfos[nsnap - 1].nci_next) {
		nsnap = nandfs_get_snap(&fs, next, cpinfos, NCPINFO);
		if (nsnap < 1)
			break;

		for (i = 0; i < nsnap; i++)
			print_cpinfo(&cpinfos[i]);
	}

	if (nsnap == -1)
		fprintf(stderr, "nandfs_get_snap: %s\n", nandfs_errmsg(&fs));

out:
	nandfs_close(&fs);
	nandfs_destroy(&fs);
	free(cpinfos);
	return (error);
}