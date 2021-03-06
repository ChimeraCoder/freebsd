
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
#include <sys/event.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

static void
usage(void)
{

	fprintf(stderr, "usage: pwait [-v] pid ...\n");
	exit(EX_USAGE);
}

/*
 * pwait - wait for processes to terminate
 */
int
main(int argc, char *argv[])
{
	int kq;
	struct kevent *e;
	int verbose = 0;
	int opt, nleft, n, i, duplicate, status;
	long pid;
	char *s, *end;

	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch (opt) {
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0)
		usage();

	kq = kqueue();
	if (kq == -1)
		err(1, "kqueue");

	e = malloc(argc * sizeof(struct kevent));
	if (e == NULL)
		err(1, "malloc");
	nleft = 0;
	for (n = 0; n < argc; n++) {
		s = argv[n];
		if (!strncmp(s, "/proc/", 6)) /* Undocumented Solaris compat */
			s += 6;
		errno = 0;
		pid = strtol(s, &end, 10);
		if (pid < 0 || *end != '\0' || errno != 0) {
			warnx("%s: bad process id", s);
			continue;
		}
		duplicate = 0;
		for (i = 0; i < nleft; i++)
			if (e[i].ident == (uintptr_t)pid)
				duplicate = 1;
		if (!duplicate) {
			EV_SET(e + nleft, pid, EVFILT_PROC, EV_ADD, NOTE_EXIT,
			    0, NULL);
			if (kevent(kq, e + nleft, 1, NULL, 0, NULL) == -1)
				warn("%ld", pid);
			else
				nleft++;
		}
	}

	while (nleft > 0) {
		n = kevent(kq, NULL, 0, e, nleft, NULL);
		if (n == -1)
			err(1, "kevent");
		if (verbose)
			for (i = 0; i < n; i++) {
				status = e[i].data;
				if (WIFEXITED(status))
					printf("%ld: exited with status %d.\n",
					    (long)e[i].ident,
					    WEXITSTATUS(status));
				else if (WIFSIGNALED(status))
					printf("%ld: killed by signal %d.\n",
					    (long)e[i].ident,
					    WTERMSIG(status));
				else
					printf("%ld: terminated.\n",
					    (long)e[i].ident);
			}
		nleft -= n;
	}

	exit(EX_OK);
}