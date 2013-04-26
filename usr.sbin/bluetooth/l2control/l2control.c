
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

#include <assert.h>
#include <bluetooth.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "l2control.h"

/* Prototypes */
static int                    do_l2cap_command    (bdaddr_p, int, char **);
static struct l2cap_command * find_l2cap_command  (char const *, 
                                                   struct l2cap_command *);
static void                   print_l2cap_command (struct l2cap_command *);
static void                   usage               (void);

/* Main */

int	numeric_bdaddr = 0;

int
main(int argc, char *argv[])
{
	int		n;
	bdaddr_t	bdaddr;

	memset(&bdaddr, 0, sizeof(bdaddr));

	/* Process command line arguments */
	while ((n = getopt(argc, argv, "a:nh")) != -1) {
		switch (n) {
		case 'a':
			if (!bt_aton(optarg, &bdaddr)) {
				struct hostent	*he = NULL;

				if ((he = bt_gethostbyname(optarg)) == NULL)
					errx(1, "%s: %s", optarg, hstrerror(h_errno));

				memcpy(&bdaddr, he->h_addr, sizeof(bdaddr));
			}
			break;

		case 'n':
			numeric_bdaddr = 1;
			break;

		case 'h':
		default:
			usage();
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (*argv == NULL)
		usage();

	return (do_l2cap_command(&bdaddr, argc, argv));
} /* main */

/* Execute commands */
static int
do_l2cap_command(bdaddr_p bdaddr, int argc, char **argv)
{
	char			*cmd = argv[0];
	struct l2cap_command	*c = NULL;
	struct sockaddr_l2cap	 sa;
	int			 s, e, help;

	help = 0;
	if (strcasecmp(cmd, "help") == 0) {
		argc --;
		argv ++;

		if (argc <= 0) {
			fprintf(stdout, "Supported commands:\n");
			print_l2cap_command(l2cap_commands);
			fprintf(stdout, "\nFor more information use " \
				"'help command'\n");

			return (OK);
		}

		help = 1;
		cmd = argv[0];
	}

	c = find_l2cap_command(cmd, l2cap_commands);
	if (c == NULL) {
		fprintf(stdout, "Unknown command: \"%s\"\n", cmd);
		return (ERROR);
	}

	if (!help) {
		if (memcmp(bdaddr, NG_HCI_BDADDR_ANY, sizeof(*bdaddr)) == 0)
			usage();

		memset(&sa, 0, sizeof(sa));
		sa.l2cap_len = sizeof(sa);
		sa.l2cap_family = AF_BLUETOOTH;
		memcpy(&sa.l2cap_bdaddr, bdaddr, sizeof(sa.l2cap_bdaddr));

		s = socket(PF_BLUETOOTH, SOCK_RAW, BLUETOOTH_PROTO_L2CAP);
		if (s < 0)
			err(1, "Could not create socket");
	
		if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0)
			err(2,
"Could not bind socket, bdaddr=%s", bt_ntoa(&sa.l2cap_bdaddr, NULL));

		e = 0x0ffff;
		if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &e, sizeof(e)) < 0)
			err(3, "Coult not setsockopt(RCVBUF, %d)", e);

		e = (c->handler)(s, -- argc, ++ argv);

		close(s);
	} else
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
} /* do_l2cap_command */

/* Try to find command in specified category */
static struct l2cap_command *
find_l2cap_command(char const *command, struct l2cap_command *category)
{
	struct l2cap_command	*c = NULL;

	for (c = category; c->command != NULL; c++) {
		char 	*c_end = strchr(c->command, ' ');

		if (c_end != NULL) {
			int	len = c_end - c->command;

			if (strncasecmp(command, c->command, len) == 0)
				return (c);
		} else if (strcasecmp(command, c->command) == 0)
				return (c);
	}

	return (NULL);
} /* find_l2cap_command */

/* Print commands in specified category */
static void
print_l2cap_command(struct l2cap_command *category)
{
	struct l2cap_command	*c = NULL;

	for (c = category; c->command != NULL; c++)
		fprintf(stdout, "\t%s\n", c->command);
} /* print_l2cap_command */

/* Usage */
static void
usage(void)
{
	fprintf(stderr, "Usage: l2control [-hn] -a local cmd [params ..]\n");
	fprintf(stderr, "Where:\n");
	fprintf(stderr, "  -a local   Specify local device to connect to\n");
	fprintf(stderr, "  -h         Display this message\n");
	fprintf(stderr, "  -n         Show addresses as numbers\n");
	fprintf(stderr, "  cmd        Supported command " \
		"(see l2control help)\n");
	fprintf(stderr, "  params     Optional command parameters\n");
	exit(255);
} /* usage */