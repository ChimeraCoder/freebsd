
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

#include <sys/errno.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mfiutil.h"

SET_DECLARE(MFI_DATASET(top), struct mfiutil_command);

MFI_TABLE(top, start);
MFI_TABLE(top, stop);
MFI_TABLE(top, abort);

int mfi_unit;
u_int mfi_opts;
static int fw_name_width, fw_version_width, fw_date_width, fw_time_width;

static void
usage(void)
{

	fprintf(stderr, "usage: mfiutil [-de] [-u unit] <command> ...\n\n");
	fprintf(stderr, "Commands include:\n");
	fprintf(stderr, "    version\n");
	fprintf(stderr, "    show adapter              - display controller information\n");
	fprintf(stderr, "    show battery              - display battery information\n");
	fprintf(stderr, "    show config               - display RAID configuration\n");
	fprintf(stderr, "    show drives               - list physical drives\n");
	fprintf(stderr, "    show events               - display event log\n");
	fprintf(stderr, "    show firmware             - list firmware images\n");
	fprintf(stderr, "    show logstate             - display event log sequence numbers\n");
	fprintf(stderr, "    show volumes              - list logical volumes\n");
	fprintf(stderr, "    show patrol               - display patrol read status\n");
	fprintf(stderr, "    show progress             - display status of active operations\n");
	fprintf(stderr, "    fail <drive>              - fail a physical drive\n");
	fprintf(stderr, "    good <drive>              - mark a bad physical drive as good\n");
	fprintf(stderr, "    rebuild <drive>           - mark failed drive ready for rebuild\n");
	fprintf(stderr, "    drive progress <drive>    - display status of active operations\n");
	fprintf(stderr, "    drive clear <drive> <start|stop> - clear a drive with all 0x00\n");
	fprintf(stderr, "    start rebuild <drive>\n");
	fprintf(stderr, "    abort rebuild <drive>\n");
	fprintf(stderr, "    locate <drive> <on|off>   - toggle drive LED\n");
	fprintf(stderr, "    cache <volume> [command [setting]]\n");
	fprintf(stderr, "    name <volume> <name>\n");
	fprintf(stderr, "    volume progress <volume>  - display status of active operations\n");
	fprintf(stderr, "    clear                     - clear volume configuration\n");
	fprintf(stderr, "    create <type> [-v] <drive>[,<drive>[,...]] [<drive>[,<drive>[,...]]\n");
	fprintf(stderr, "    delete <volume>\n");
	fprintf(stderr, "    add <drive> [volume]      - add a hot spare\n");
	fprintf(stderr, "    remove <drive>            - remove a hot spare\n");
	fprintf(stderr, "    patrol <disable|auto|manual> [interval [start]]\n");
	fprintf(stderr, "    start patrol              - start a patrol read\n");
	fprintf(stderr, "    stop patrol               - stop a patrol read\n");
	fprintf(stderr, "    flash <firmware>\n");
	fprintf(stderr, "    start learn               - start a BBU relearn\n");
	fprintf(stderr, "    bbu <setting> <value>     - set BBU properties\n");
#ifdef DEBUG
	fprintf(stderr, "    debug                     - debug 'show config'\n");
	fprintf(stderr, "    dump                      - display 'saved' config\n");
#endif
	exit(1);
}

static int
version(int ac __unused, char **av __unused)
{

	printf("mfiutil version 1.0.14");
#ifdef DEBUG
	printf(" (DEBUG)");
#endif
	printf("\n");
	return (0);
}
MFI_COMMAND(top, version, version);

int
main(int ac, char **av)
{
	struct mfiutil_command **cmd;
	int ch;

	while ((ch = getopt(ac, av, "deu:")) != -1) {
		switch (ch) {
		case 'd':
			mfi_opts |= MFI_DNAME_DEVICE_ID;
			break;
		case 'e':
			mfi_opts |= MFI_DNAME_ES;
			break;
		case 'u':
			mfi_unit = atoi(optarg);
			break;
		case '?':
			usage();
		}
	}

	av += optind;
	ac -= optind;

	/* getopt() eats av[0], so we can't use mfi_table_handler() directly. */
	if (ac == 0)
		usage();

	SET_FOREACH(cmd, MFI_DATASET(top)) {
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

void
scan_firmware(struct mfi_info_component *comp)
{
	int len;

	len = strlen(comp->name);
	if (fw_name_width < len)
		fw_name_width = len;
	len = strlen(comp->version);
	if (fw_version_width < len)
		fw_version_width = len;
	len = strlen(comp->build_date);
	if (fw_date_width < len)
		fw_date_width = len;
	len = strlen(comp->build_time);
	if (fw_time_width < len)
		fw_time_width = len;
}

void
display_firmware(struct mfi_info_component *comp, const char *tag)
{

	printf("%-*s  %-*s  %-*s  %-*s  %s\n", fw_name_width, comp->name,
	    fw_version_width, comp->version, fw_date_width, comp->build_date,
	    fw_time_width, comp->build_time, tag);
}