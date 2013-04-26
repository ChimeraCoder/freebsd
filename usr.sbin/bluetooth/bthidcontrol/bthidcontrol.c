
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

#include <sys/queue.h>
#include <assert.h>
#include <bluetooth.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <usbhid.h>
#include "bthid_config.h"
#include "bthidcontrol.h"

static int do_bthid_command(bdaddr_p bdaddr, int argc, char **argv);
static struct bthid_command * find_bthid_command(char const *command, struct bthid_command *category);
static void print_bthid_command(struct bthid_command *category);
static void usage(void);

int32_t hid_sdp_query(bdaddr_t const *local, bdaddr_t const *remote, int32_t *error);

uint32_t verbose = 0;

/*
 * bthidcontrol
 */

int
main(int argc, char *argv[])
{
	bdaddr_t	bdaddr;
	int		opt;

	hid_init(NULL);
	memcpy(&bdaddr, NG_HCI_BDADDR_ANY, sizeof(bdaddr));

	while ((opt = getopt(argc, argv, "a:c:H:hv")) != -1) {
		switch (opt) {
		case 'a': /* bdaddr */
			if (!bt_aton(optarg, &bdaddr)) {
				struct hostent  *he = NULL;

				if ((he = bt_gethostbyname(optarg)) == NULL)
					errx(1, "%s: %s", optarg, hstrerror(h_errno));

				memcpy(&bdaddr, he->h_addr, sizeof(bdaddr));
			}
			break;

		case 'c': /* config file */
			config_file = optarg;
			break;

		case 'H': /* HIDs file */
			hids_file = optarg;
			break;

		case 'v': /* verbose */
			verbose++;
			break;

		case 'h':
		default:
			usage();
			/* NOT REACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (*argv == NULL)
		usage();

	return (do_bthid_command(&bdaddr, argc, argv));
} /* main */

/* Execute commands */
static int
do_bthid_command(bdaddr_p bdaddr, int argc, char **argv)
{
	char			*cmd = argv[0];
	struct bthid_command	*c = NULL;
	int			 e, help;

	help = 0;
	if (strcasecmp(cmd, "help") == 0) {
		argc --;
		argv ++;

		if (argc <= 0) {
			fprintf(stdout, "Supported commands:\n");
			print_bthid_command(sdp_commands);
			print_bthid_command(hid_commands);
			fprintf(stdout, "\nFor more information use " \
					"'help command'\n");

			return (OK);
		}

		help = 1;
		cmd = argv[0];
	}

	c = find_bthid_command(cmd, sdp_commands); 
	if (c == NULL)
		c = find_bthid_command(cmd, hid_commands); 

	if (c == NULL) {
		fprintf(stdout, "Unknown command: \"%s\"\n", cmd);
		return (ERROR);
	}

	if (!help)
		e = (c->handler)(bdaddr, -- argc, ++ argv);
	else
		e = USAGE;

	switch (e) {
	case OK:
	case FAILED:
		break;

	case ERROR:
		fprintf(stdout, "Could not execute command \"%s\". %s\n",
				cmd, strerror(errno));
		break;

	case USAGE:
		fprintf(stdout, "Usage: %s\n%s\n", c->command, c->description);
		break;

	default: assert(0); break;
	}

	return (e);
} /* do_bthid_command */

/* Try to find command in specified category */
static struct bthid_command *
find_bthid_command(char const *command, struct bthid_command *category)
{ 
	struct bthid_command	*c = NULL;
  
	for (c = category; c->command != NULL; c++) {
		char	*c_end = strchr(c->command, ' ');
  
		if (c_end != NULL) {
			int	len = c_end - c->command;
 
			if (strncasecmp(command, c->command, len) == 0)
				return (c);
		} else if (strcasecmp(command, c->command) == 0)
				return (c);
	}
  
	return (NULL);
} /* find_bthid_command */

/* Print commands in specified category */
static void
print_bthid_command(struct bthid_command *category)
{
	struct bthid_command	*c = NULL;
 
	for (c = category; c->command != NULL; c++)
		fprintf(stdout, "\t%s\n", c->command);
} /* print_bthid_command */

/* Usage */ 
static void
usage(void)
{
	fprintf(stderr,
"Usage: bthidcontrol options command\n" \
"Where options are:\n"
"	-a bdaddr	specify bdaddr\n" \
"	-c file		specify path to the bthidd config file\n" \
"	-H file		specify path to the bthidd HIDs file\n" \
"	-h		display usage and quit\n" \
"	-v		be verbose\n" \
"	command		one of the supported commands\n");
	exit(255);
} /* usage */