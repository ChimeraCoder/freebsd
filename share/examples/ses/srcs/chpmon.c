
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "ses.h"

/*
 * Continuously monitor all named SES devices
 * and turn all but INFO enclosure status
 * values into CRITICAL enclosure status.
 */
#define	BADSTAT	\
	(SES_ENCSTAT_UNRECOV|SES_ENCSTAT_CRITICAL|SES_ENCSTAT_NONCRITICAL)
int
main(int a, char **v)
{
	int fd, delay, dev;
	ses_encstat stat, *carray;

	if (a < 3) {
		fprintf(stderr, "usage: %s polling-interval device "
		    "[ device ... ]\n", *v);
		return (1);
	}
	delay = atoi(v[1]);
	carray = malloc(a);
	if (carray == NULL) {
		perror("malloc");
		return (1);
	}
	bzero((void *)carray, a);

	for (;;) {
		for (dev = 2; dev < a; dev++) {
			fd = open(v[dev], O_RDWR);
			if (fd < 0) {
				perror(v[dev]);
				continue;
			}
			/*
			 * First clear any enclosure status, in case it is
			 * a latched status.
			 */
			stat = 0;
			if (ioctl(fd, SESIOC_SETENCSTAT, (caddr_t) &stat) < 0) {
				fprintf(stderr, "%s: SESIOC_SETENCSTAT1: %s\n",
				    v[dev], strerror(errno));
				(void) close(fd);
				continue;
			}
			/*
			 * Now get the actual current enclosure status.
			 */
			if (ioctl(fd, SESIOC_GETENCSTAT, (caddr_t) &stat) < 0) {
				fprintf(stderr, "%s: SESIOC_GETENCSTAT: %s\n",
				    v[dev], strerror(errno));
				(void) close(fd);
				continue;
			}

			if ((stat & BADSTAT) == 0) {
				if (carray[dev]) {
					fprintf(stdout, "%s: Clearing CRITICAL "
					    "condition\n", v[dev]);
					carray[dev] = 0;
				}
				(void) close(fd);
				continue;
			}
			carray[dev] = 1;
			fprintf(stdout, "%s: Setting CRITICAL from:", v[dev]);
			if (stat & SES_ENCSTAT_UNRECOV)
				fprintf(stdout, " UNRECOVERABLE");
		
			if (stat & SES_ENCSTAT_CRITICAL)
				fprintf(stdout, " CRITICAL");
		
			if (stat & SES_ENCSTAT_NONCRITICAL)
				fprintf(stdout, " NONCRITICAL");
			putchar('\n');
			stat = SES_ENCSTAT_CRITICAL;
			if (ioctl(fd, SESIOC_SETENCSTAT, (caddr_t) &stat) < 0) {
				fprintf(stderr, "%s: SESIOC_SETENCSTAT 2: %s\n",
				    v[dev], strerror(errno));
			}
			(void) close(fd);
		}
		sleep(delay);
	}
	/* NOTREACHED */
}