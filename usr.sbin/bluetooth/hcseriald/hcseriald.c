
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

#include <sys/types.h>
#include <sys/ioctl.h>

#include <netgraph/ng_message.h>
#include <netgraph.h>
#include <netgraph/bluetooth/include/ng_h4.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <syslog.h>
#include <termios.h>
#include <unistd.h>

/* Prototypes */
static int	open_device	(char const *, speed_t, char const *);
static void	sighandler	(int);
static void	usage		();

static char const * const	hcseriald = "hcseriald";
static int			done = 0;

int
main(int argc, char *argv[])
{
	char			*device = NULL, *name = NULL;
	speed_t			 speed = 115200;
	int			 n, detach = 1;
	char			 p[FILENAME_MAX];
	FILE			*f = NULL;
	struct sigaction	 sa;

	/* Process command line arguments */
	while ((n = getopt(argc, argv, "df:n:s:h")) != -1) {
		switch (n) {
		case 'd':
			detach = 0;
			break;

		case 'f':
			device = optarg;
			break;

		case 'n':
			name = optarg;
			break;

		case 's':
			speed = atoi(optarg);
			if (speed < 0)
				usage(argv[0]);
			break;

		case 'h':
		default:
			usage(argv[0]);
			break;
		}
	}

	if (device == NULL || name == NULL)
		usage(argv[0]);

	openlog(hcseriald, LOG_PID | LOG_NDELAY, LOG_USER);

	/* Open device */
	n = open_device(device, speed, name);

	if (detach && daemon(0, 0) < 0) {
		syslog(LOG_ERR, "Could not daemon(0, 0). %s (%d)",
			strerror(errno), errno);
		exit(1);
	}

	/* Write PID file */
	snprintf(p, sizeof(p), "/var/run/%s.%s.pid", hcseriald, name);
	f = fopen(p, "w");
	if (f == NULL) {
		syslog(LOG_ERR, "Could not fopen(%s). %s (%d)",
			p, strerror(errno), errno);
		exit(1);
	}
	fprintf(f, "%d", getpid());
	fclose(f);

	/* Install signal handler */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sighandler;

	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		syslog(LOG_ERR, "Could not sigaction(SIGTERM). %s (%d)",
			strerror(errno), errno);
		exit(1);
	}

	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		syslog(LOG_ERR, "Could not sigaction(SIGHUP). %s (%d)",
			strerror(errno), errno);
		exit(1);
	}

	if (sigaction(SIGINT, &sa, NULL) < 0) {
		syslog(LOG_ERR, "Could not sigaction(SIGINT). %s (%d)",
			strerror(errno), errno);
		exit(1);
	}

	/* Keep running */
	while (!done)
		select(0, NULL, NULL, NULL, NULL);

	/* Remove PID file and close device */
	unlink(p);
	close(n);
	closelog();

	return (0);
} /* main */

/* Open terminal, set settings, push H4 line discipline and set node name */
static int
open_device(char const *device, speed_t speed, char const *name)
{
	int		fd, disc, cs, ds;
	struct termios	t;
	struct nodeinfo	ni;
	struct ngm_name	n;
	char		p[NG_NODESIZ];

	/* Open terminal device and setup H4 line discipline */
	fd = open(device, O_RDWR|O_NOCTTY);
	if (fd < 0) {
		syslog(LOG_ERR, "Could not open(%s). %s (%d)", 
			device, strerror(errno), errno);
		exit(1);
	}

	tcflush(fd, TCIOFLUSH);

	if (tcgetattr(fd, &t) < 0) {
		syslog(LOG_ERR, "Could not tcgetattr(%s). %s (%d)",
			device, strerror(errno), errno);
		exit(1);
	}

	cfmakeraw(&t);

	t.c_cflag |= CLOCAL;	/* clocal */
	t.c_cflag &= ~CSIZE;	/* cs8 */
	t.c_cflag |= CS8; 	/* cs8 */
	t.c_cflag &= ~PARENB;	/* -parenb */
	t.c_cflag &= ~CSTOPB;	/* -cstopb */
	t.c_cflag |= CRTSCTS;	/* crtscts */

	if (tcsetattr(fd, TCSANOW, &t) < 0) {
		syslog(LOG_ERR, "Could not tcsetattr(%s). %s (%d)",
			device, strerror(errno), errno);
		exit(1);
	}

	tcflush(fd, TCIOFLUSH);

	if (cfsetspeed(&t, speed) < 0) {
		syslog(LOG_ERR, "Could not cfsetspeed(%s). %s (%d)",
			device, strerror(errno), errno);
		exit(1);
	}

	if (tcsetattr(fd, TCSANOW, &t) < 0) {
		syslog(LOG_ERR, "Could not tcsetattr(%s). %s (%d)",
			device, strerror(errno), errno);
		exit(1);
	}

	disc = H4DISC;
	if (ioctl(fd, TIOCSETD, &disc) < 0) {
		syslog(LOG_ERR, "Could not ioctl(%s, TIOCSETD, %d). %s (%d)",
			device, disc, strerror(errno), errno);
		exit(1);
	}

	/* Get default name of the Netgraph node */
	memset(&ni, 0, sizeof(ni));
	if (ioctl(fd, NGIOCGINFO, &ni) < 0) {
		syslog(LOG_ERR, "Could not ioctl(%d, NGIOGINFO). %s (%d)",
			fd, strerror(errno), errno);
		exit(1);
	}

	/* Assign new name to the Netgraph node */
	snprintf(p, sizeof(p), "%s:", ni.name);
	snprintf(n.name, sizeof(n.name), "%s", name);

	if (NgMkSockNode(NULL, &cs, &ds) < 0) {
		syslog(LOG_ERR, "Could not NgMkSockNode(). %s (%d)",
			strerror(errno), errno);
		exit(1);
	}

	if (NgSendMsg(cs, p, NGM_GENERIC_COOKIE, NGM_NAME, &n, sizeof(n)) < 0) {
		syslog(LOG_ERR, "Could not NgSendMsg(%d, %s, NGM_NAME, %s). " \
			"%s (%d)", cs, p, n.name, strerror(errno), errno);
		exit(1);
	}

	close(cs);
	close(ds);

	return (fd);
} /* open_device */

/* Signal handler */
static void
sighandler(int s)
{
	done = 1;
} /* sighandler */

/* Usage */
static void
usage(void)
{
	fprintf(stderr, "Usage: %s -f device -n node_name [-s speed -d -h]\n" \
			"Where:\n" \
			"\t-f device    tty device name, ex. /dev/cuau1\n" \
			"\t-n node_name set Netgraph node name to node_name\n" \
			"\t-s speed     set tty speed, ex. 115200\n" \
			"\t-d           run in foreground\n" \
			"\t-h           display this message\n",
			hcseriald);
	exit(255);
} /* usage */