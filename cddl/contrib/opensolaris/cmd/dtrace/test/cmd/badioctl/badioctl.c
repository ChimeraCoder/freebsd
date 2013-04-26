
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

/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/varargs.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define	DTRACEIOC	(('d' << 24) | ('t' << 16) | ('r' << 8))
#define	DTRACEIOC_MAX	17

void
fatal(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	fprintf(stderr, "%s: ", "badioctl");
	vfprintf(stderr, fmt, ap);

	if (fmt[strlen(fmt) - 1] != '\n')
		fprintf(stderr, ": %s\n", strerror(errno));

	exit(1);
}

void
badioctl(pid_t parent)
{
	int fd = -1, random, ps = sysconf(_SC_PAGESIZE);
	int i = 0, seconds;
	caddr_t addr;
	hrtime_t now, last = 0, end;

	if ((random = open("/dev/random", O_RDONLY)) == -1)
		fatal("couldn't open /dev/random");

	if ((addr = mmap(0, ps, PROT_READ | PROT_WRITE,
	    MAP_ANON | MAP_PRIVATE, -1, 0)) == (caddr_t)-1)
		fatal("mmap");

	for (;;) {
		unsigned int ioc;

		if ((now = gethrtime()) - last > NANOSEC) {
			if (kill(parent, 0) == -1 && errno == ESRCH) {
				/*
				 * Our parent died.  We will kill ourselves in
				 * sympathy.
				 */
				exit(0);
			}

			/*
			 * Once a second, we'll reopen the device.
			 */
			if (fd != -1)
				close(fd);

			fd = open("/devices/pseudo/dtrace@0:dtrace", O_RDONLY);

			if (fd == -1)
				fatal("couldn't open DTrace pseudo device");

			last = now;
		}


		if ((i++ % 1000) == 0) {
			/*
			 * Every thousand iterations, change our random gunk.
			 */
			read(random, addr, ps);
		}

		read(random, &ioc, sizeof (ioc));
		ioc %= DTRACEIOC_MAX;
		ioc++;
		ioctl(fd, DTRACEIOC | ioc, addr);
	}
}

int
main()
{
	pid_t child, parent = getpid();
	int status;

	for (;;) {
		if ((child = fork()) == 0)
			badioctl(parent);

		while (waitpid(child, &status, WEXITED) != child)
			continue;

		if (WIFEXITED(status)) {
			/*
			 * Our child exited by design -- we'll exit with
			 * the same status code.
			 */
			exit(WEXITSTATUS(status));
		}

		/*
		 * Our child died on a signal.  Respawn it.
		 */
		printf("badioctl: child died on signal %d; respawning.\n",
		    WTERMSIG(status));
		fflush(stdout);
	}

	/* NOTREACHED */
	return (0);
}