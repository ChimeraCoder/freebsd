
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
#include <sdp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sdpcontrol.h"

/* Prototypes */
static int                  do_sdp_command	(bdaddr_p, char const *, int,
						 int, char **);
static struct sdp_command * find_sdp_command	(char const *,
						 struct sdp_command *); 
static void                 print_sdp_command	(struct sdp_command *);
static void                 usage		(void);

/* Main */
int
main(int argc, char *argv[])
{
	char const	*control = SDP_LOCAL_PATH;
	int		 n, local;
	bdaddr_t	 bdaddr;

	memset(&bdaddr, 0, sizeof(bdaddr));
	local = 0;

	/* Process command line arguments */
	while ((n = getopt(argc, argv, "a:c:lh")) != -1) {
		switch (n) {
		case 'a': /* bdaddr */
			if (!bt_aton(optarg, &bdaddr)) {
				struct hostent  *he = NULL;

				if ((he = bt_gethostbyname(optarg)) == NULL)
					errx(1, "%s: %s", optarg, hstrerror(h_errno));
 
				memcpy(&bdaddr, he->h_addr, sizeof(bdaddr));
			}
			break;

		case 'c': /* control socket */
			control = optarg;
			break;

		case 'l': /* local sdpd */
			local = 1;
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

	return (do_sdp_command(&bdaddr, control, local, argc, argv));
}

/* Execute commands */
static int
do_sdp_command(bdaddr_p bdaddr, char const *control, int local,
		int argc, char **argv)
{
	char			*cmd = argv[0];
	struct sdp_command	*c = NULL;
	void			*xs = NULL;
	int			 e, help;

	help = 0;
	if (strcasecmp(cmd, "help") == 0) {
		argc --;
		argv ++;

		if (argc <= 0) {
			fprintf(stdout, "Supported commands:\n");
			print_sdp_command(sdp_commands);
			fprintf(stdout, "\nFor more information use " \
				"'help command'\n");

			return (OK);
		}

		help = 1;
		cmd = argv[0];
	}

	c = find_sdp_command(cmd, sdp_commands);
	if (c == NULL) {
		fprintf(stdout, "Unknown command: \"%s\"\n", cmd);
		return (ERROR);
	}

	if (!help) {
		if (!local) {
			if (memcmp(bdaddr, NG_HCI_BDADDR_ANY, sizeof(*bdaddr)) == 0)
				usage();

			xs = sdp_open(NG_HCI_BDADDR_ANY, bdaddr);
		} else
			xs = sdp_open_local(control);

		if (xs == NULL)
			errx(1, "Could not create SDP session object");
		if (sdp_error(xs) == 0)
			e = (c->handler)(xs, -- argc, ++ argv);
		else
			e = ERROR;
	} else
		e = USAGE;

	switch (e) {
	case OK:
	case FAILED:
		break;

	case ERROR:
		fprintf(stdout, "Could not execute command \"%s\". %s\n",
			cmd, strerror(sdp_error(xs)));
		break;

	case USAGE:
		fprintf(stdout, "Usage: %s\n%s\n", c->command, c->description);
		break;

	default: assert(0); break;
	}

	sdp_close(xs);

	return (e);
} /* do_sdp_command */

/* Try to find command in specified category */
static struct sdp_command *
find_sdp_command(char const *command, struct sdp_command *category)   
{
	struct sdp_command	*c = NULL;

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
} /* find_sdp_command */

/* Print commands in specified category */
static void
print_sdp_command(struct sdp_command *category)
{
	struct sdp_command	*c = NULL;

	for (c = category; c->command != NULL; c++)
		fprintf(stdout, "\t%s\n", c->command);
} /* print_sdp_command */

/* Usage */
static void
usage(void)
{
	fprintf(stderr,
"Usage: sdpcontrol options command\n" \
"Where options are:\n"
"	-a address	address to connect to\n" \
"	-c path		path to the control socket (default is %s)\n" \
"	-h		display usage and quit\n" \
"	-l		connect to the local SDP server via control socket\n" \
"	command		one of the supported commands\n", SDP_LOCAL_PATH);
	exit(255);
} /* usage */