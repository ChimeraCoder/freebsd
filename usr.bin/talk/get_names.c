
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

#ifndef lint
static const char sccsid[] = "@(#)get_names.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/param.h>

#include <err.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "talk.h"

extern	CTL_MSG msg;

static void
usage(void)
{
	fprintf(stderr, "usage: talk person [ttyname]\n");
	exit(1);
}

/*
 * Determine the local and remote user, tty, and machines
 */
void
get_names(int argc, char *argv[])
{
	char hostname[MAXHOSTNAMELEN];
	char *his_name, *my_name;
	const char *my_machine_name, *his_machine_name;
	const char *his_tty;
	char *cp;

	if (argc < 2 )
		usage();
	if (!isatty(0))
		errx(1, "standard input must be a tty, not a pipe or a file");
	if ((my_name = getlogin()) == NULL) {
		struct passwd *pw;

		if ((pw = getpwuid(getuid())) == NULL)
			errx(1, "you don't exist. Go away");
		my_name = pw->pw_name;
	}
	gethostname(hostname, sizeof (hostname));
	my_machine_name = hostname;
	/* check for, and strip out, the machine name of the target */
	cp = argv[1] + strcspn(argv[1], "@:!");
	if (*cp == '\0') {
		/* this is a local to local talk */
		his_name = argv[1];
		my_machine_name = his_machine_name = "localhost";
	} else {
		if (*cp++ == '@') {
			/* user@host */
			his_name = argv[1];
			his_machine_name = cp;
		} else {
			/* host!user or host:user */
			his_name = cp;
			his_machine_name = argv[1];
		}
		*--cp = '\0';
	}
	if (argc > 2)
		his_tty = argv[2];	/* tty name is arg 2 */
	else
		his_tty = "";
	get_addrs(my_machine_name, his_machine_name);
	/*
	 * Initialize the message template.
	 */
	msg.vers = TALK_VERSION;
	msg.addr.sa_family = htons(AF_INET);
	msg.ctl_addr.sa_family = htons(AF_INET);
	msg.id_num = htonl(0);
	strlcpy(msg.l_name, my_name, NAME_SIZE);
	strlcpy(msg.r_name, his_name, NAME_SIZE);
	strlcpy(msg.r_tty, his_tty, TTY_SIZE);
}