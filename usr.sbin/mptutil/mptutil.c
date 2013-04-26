
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
__RCSID("$FreeBSD$");

#include <sys/param.h>
#include <sys/errno.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mptutil.h"

SET_DECLARE(MPT_DATASET(top), struct mptutil_command);

int mpt_unit;

static void
usage(void)
{

	fprintf(stderr, "usage: mptutil [-u unit] <command> ...\n\n");
	fprintf(stderr, "Commands include:\n");
	fprintf(stderr, "    version\n");
	fprintf(stderr, "    show adapter              - display controller information\n");
	fprintf(stderr, "    show config               - display RAID configuration\n");
	fprintf(stderr, "    show drives               - list physical drives\n");
	fprintf(stderr, "    show events               - display event log\n");
	fprintf(stderr, "    show volumes              - list logical volumes\n");
	fprintf(stderr, "    fail <drive>              - fail a physical drive\n");
	fprintf(stderr, "    online <drive>            - bring an offline physical drive online\n");
	fprintf(stderr, "    offline <drive>           - mark a physical drive offline\n");
	fprintf(stderr, "    name <volume> <name>\n");
	fprintf(stderr, "    volume status <volume>    - display volume status\n");
	fprintf(stderr, "    volume cache <volume> <enable|disable>\n");
	fprintf(stderr, "                              - Enable or disable the volume drive caches\n");
	fprintf(stderr, "    clear                     - clear volume configuration\n");
	fprintf(stderr, "    create <type> [-vq] [-s stripe] <drive>[,<drive>[,...]]\n");
	fprintf(stderr, "    delete <volume>\n");
	fprintf(stderr, "    add <drive> [volume]      - add a hot spare\n");
	fprintf(stderr, "    remove <drive>            - remove a hot spare\n");
#ifdef DEBUG
	fprintf(stderr, "    pd create <drive>         - create RAID physdisk\n");
	fprintf(stderr, "    pd delete <drive>         - delete RAID physdisk\n");
	fprintf(stderr, "    debug                     - debug 'show config'\n");
#endif
	exit(1);
}

static int
version(int ac, char **av)
{

	printf("mptutil version 1.0.3");
#ifdef DEBUG
	printf(" (DEBUG)");
#endif
	printf("\n");
	return (0);
}
MPT_COMMAND(top, version, version);

int
main(int ac, char **av)
{
	struct mptutil_command **cmd;
	int ch;

	while ((ch = getopt(ac, av, "u:")) != -1) {
		switch (ch) {
		case 'u':
			mpt_unit = atoi(optarg);
			break;
		case '?':
			usage();
		}
	}

	av += optind;
	ac -= optind;

	/* getopt() eats av[0], so we can't use mpt_table_handler() directly. */
	if (ac == 0)
		usage();

	SET_FOREACH(cmd, MPT_DATASET(top)) {
		if (strcmp((*cmd)->name, av[0]) == 0) {
			if ((*cmd)->handler(ac, av))
				return (1);
			else
				return (0);
		}
	}
	warnx("Unknown command %s.", av[0]);
	return (1);
}