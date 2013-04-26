
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

#include <sys/param.h>
#include <sys/rtprio.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int parseint(const char *, const char *);
static void usage(void);

int
main(int argc, char *argv[])
{
	struct rtprio rtp;
	const char *progname;
	pid_t proc = 0;

	progname = getprogname();

	if (strcmp(progname, "rtprio") == 0)
		rtp.type = RTP_PRIO_REALTIME;
	else if (strcmp(progname, "idprio") == 0)
		rtp.type = RTP_PRIO_IDLE;
	else
		errx(1, "invalid progname");

	switch (argc) {
	case 2:
		proc = parseint(argv[1], "pid");
		proc = abs(proc);
		/* FALLTHROUGH */
	case 1:
		if (rtprio(RTP_LOOKUP, proc, &rtp) != 0)
			err(1, "RTP_LOOKUP");
		switch (rtp.type) {
		case RTP_PRIO_REALTIME:
		case RTP_PRIO_FIFO:
			warnx("realtime priority %d", rtp.prio);
			break;
		case RTP_PRIO_NORMAL:
			warnx("normal priority");
			break;
		case RTP_PRIO_IDLE:
			warnx("idle priority %d", rtp.prio);
			break;
		default:
			errx(1, "invalid priority type %d", rtp.type);
			break;
		}
		exit(0);
	default:
		if (argv[1][0] == '-' || isdigit(argv[1][0])) {
			if (argv[1][0] == '-') {
				if (strcmp(argv[1], "-t") == 0) {
					rtp.type = RTP_PRIO_NORMAL;
					rtp.prio = 0;
				} else {
					usage();
					break;
				}
			} else
				rtp.prio = parseint(argv[1], "priority");
		} else {
			usage();
			break;
		}

		if (argv[2][0] == '-') {
			proc = parseint(argv[2], "pid");
			proc = abs(proc);
		}

		if (rtprio(RTP_SET, proc, &rtp) != 0)
			err(1, "RTP_SET");

		if (proc == 0) {
			execvp(argv[2], &argv[2]);
			err(1, "execvp: %s", argv[2]);
		}
		exit(0);
	}
	/* NOTREACHED */
}

static int
parseint(const char *str, const char *errname)
{
	char *endp;
	long res;

	errno = 0;
	res = strtol(str, &endp, 10);
	if (errno != 0 || endp == str || *endp != '\0')
		errx(1, "%s must be a number", errname);
	if (res >= INT_MAX)
		errx(1, "Integer overflow parsing %s", errname);
	return (res);
}

static void
usage(void)
{

	(void) fprintf(stderr, "%s\n%s\n%s\n%s\n%s\n%s\n",
		"usage: [id|rt]prio",
		"       [id|rt]prio [-]pid",
		"       [id|rt]prio priority command [args]",
		"       [id|rt]prio priority -pid",
		"       [id|rt]prio -t command [args]",
		"       [id|rt]prio -t -pid");
	exit(1);
}