
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

#include <sys/select.h>
#include <bluetooth.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <sdp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "server.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include "profile.h"
#include "provider.h"

#define	SDPD			"sdpd"

static int32_t 	drop_root	(char const *user, char const *group);
static void	sighandler	(int32_t s);
static void	usage		(void);

static int32_t	done;

/*
 * Bluetooth Service Discovery Procotol (SDP) daemon
 */

int
main(int argc, char *argv[])
{
	server_t		 server;
	char const		*control = SDP_LOCAL_PATH;
	char const		*user = "nobody", *group = "nobody";
	int32_t			 detach = 1, opt;
	struct sigaction	 sa;

	while ((opt = getopt(argc, argv, "c:dg:hu:")) != -1) {
		switch (opt) {
		case 'c': /* control */
			control = optarg;
			break;

		case 'd': /* do not detach */
			detach = 0;
			break;

		case 'g': /* group */
			group = optarg;
			break;

		case 'u': /* user */
			user = optarg;
			break;

		case 'h':
		default:
			usage();
			/* NOT REACHED */
		}
	}

	log_open(SDPD, !detach);

	/* Become daemon if required */
	if (detach && daemon(0, 0) < 0) {
		log_crit("Could not become daemon. %s (%d)",
			strerror(errno), errno);
		exit(1);
	}

	/* Set signal handlers */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sighandler;

	if (sigaction(SIGTERM, &sa, NULL) < 0 ||
	    sigaction(SIGHUP,  &sa, NULL) < 0 ||
	    sigaction(SIGINT,  &sa, NULL) < 0) {
		log_crit("Could not install signal handlers. %s (%d)",
			strerror(errno), errno); 
		exit(1);
	}

	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) < 0) {
		log_crit("Could not install signal handlers. %s (%d)",
			strerror(errno), errno); 
		exit(1);
	}

	/* Initialize server */
	if (server_init(&server, control) < 0)
		exit(1);

	if ((user != NULL || group != NULL) && drop_root(user, group) < 0)
		exit(1);

	for (done = 0; !done; ) {
		if (server_do(&server) != 0)
			done ++;
	}

	server_shutdown(&server);
	log_close();

	return (0);
}

/*
 * Drop root
 */

static int32_t
drop_root(char const *user, char const *group)
{
	int	 uid, gid;
	char	*ep;

	if ((uid = getuid()) != 0) {
		log_notice("Cannot set uid/gid. Not a superuser");
		return (0); /* dont do anything unless root */
	}

	gid = getgid();

	if (user != NULL) {
		uid = strtol(user, &ep, 10);
		if (*ep != '\0') {
			struct passwd	*pwd = getpwnam(user);

			if (pwd == NULL) {
				log_err("Could not find passwd entry for " \
					"user %s", user);
				return (-1);
			}

			uid = pwd->pw_uid;
		}
	}

	if (group != NULL) {
		gid = strtol(group, &ep, 10);
		if (*ep != '\0') {
			struct group	*grp = getgrnam(group);

			if (grp == NULL) {
				log_err("Could not find group entry for " \
					"group %s", group);
				return (-1);
			}

			gid = grp->gr_gid;
		}
	}

	if (setgid(gid) < 0) {
		log_err("Could not setgid(%s). %s (%d)",
			group, strerror(errno), errno);
		return (-1);
	}

	if (setuid(uid) < 0) {
		log_err("Could not setuid(%s). %s (%d)",
			user, strerror(errno), errno);
		return (-1);
	}

	return (0);
}

/*
 * Signal handler
 */

static void
sighandler(int32_t s)
{
	log_notice("Got signal %d. Total number of signals received %d",
		s, ++ done);
}

/*
 * Display usage information and quit
 */

static void
usage(void)
{
	fprintf(stderr,
"Usage: %s [options]\n" \
"Where options are:\n" \
"	-c	specify control socket name (default %s)\n" \
"	-d	do not detach (run in foreground)\n" \
"	-g grp	specify group\n" \
"	-h	display usage and exit\n" \
"	-u usr	specify user\n",
		SDPD, SDP_LOCAL_PATH);
	exit(255);
}