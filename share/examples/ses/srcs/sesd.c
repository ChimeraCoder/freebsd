
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
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cam/scsi/scsi_all.h>
#include <cam/scsi/scsi_enc.h>

#define	ALLSTAT (SES_ENCSTAT_UNRECOV | SES_ENCSTAT_CRITICAL | \
	SES_ENCSTAT_NONCRITICAL | SES_ENCSTAT_INFO)

/*
 * Monitor named SES devices and note (via syslog) any changes in status.
 */

int
main(int a, char **v)
{
	static const char *usage =
	    "usage: %s [ -c ] [ -d ] [ -t pollinterval ] device [ device ]\n";
	int fd, polltime, dev, nodaemon, clear, c;
	encioc_enc_status_t stat, nstat, *carray;

	if (a < 2) {
		fprintf(stderr, usage, *v);
		return (1);
	}

	nodaemon = 0;
	polltime = 30;
	clear = 0;
	while ((c = getopt(a, v, "cdt:")) != -1) {
		switch (c) {
		case 'c':
			clear = 1;
			break;
		case 'd':
			nodaemon = 1;
			break;
		case 't':
			polltime = atoi(optarg);
			break;
		default:
			fprintf(stderr, usage, *v);
			return (1);
		}
	}

	carray = malloc(a);
	if (carray == NULL) {
		perror("malloc");
		return (1);
	}
	for (dev = optind; dev < a; dev++)
		carray[dev] = (encioc_enc_status_t) -1;

	/*
	 * Check to make sure we can open all devices
	 */
	for (dev = optind; dev < a; dev++) {
		fd = open(v[dev], O_RDWR);
		if (fd < 0) {
			perror(v[dev]);
			return (1);
		}
		if (ioctl(fd, ENCIOC_INIT, NULL) < 0) {
			fprintf(stderr, "%s: ENCIOC_INIT fails- %s\n",
			    v[dev], strerror(errno));
			return (1);
		}
		(void) close(fd);
	}
	if (nodaemon == 0) {
		if (daemon(0, 0) < 0) {
			perror("daemon");
			return (1);
		}
		openlog("sesd", LOG_CONS, LOG_USER);
	} else {
		openlog("sesd", LOG_CONS|LOG_PERROR, LOG_USER);
	}

	for (;;) {
		for (dev = optind; dev < a; dev++) {
			fd = open(v[dev], O_RDWR);
			if (fd < 0) {
				syslog(LOG_ERR, "%s: %m", v[dev]);
				continue;
			}

			/*
			 * Get the actual current enclosure status.
			 */
			if (ioctl(fd, ENCIOC_GETENCSTAT, (caddr_t) &stat) < 0) {
				syslog(LOG_ERR,
				    "%s: ENCIOC_GETENCSTAT- %m", v[dev]);
				(void) close(fd);
				continue;
			}
			if (stat != 0 && clear) {
				nstat = 0;
				if (ioctl(fd, ENCIOC_SETENCSTAT,
				    (caddr_t) &nstat) < 0) {
					syslog(LOG_ERR,
					    "%s: ENCIOC_SETENCSTAT- %m", v[dev]);
				}
			}
			(void) close(fd);

			if (stat == carray[dev])
				continue;

			carray[dev] = stat;
			if ((stat & ALLSTAT) == 0) {
				syslog(LOG_NOTICE,
				    "%s: Enclosure Status OK", v[dev]);
			}
			if (stat & SES_ENCSTAT_INFO) {
				syslog(LOG_NOTICE,
				    "%s: Enclosure Has Information", v[dev]);
			}
			if (stat & SES_ENCSTAT_NONCRITICAL) {
				syslog(LOG_WARNING,
				    "%s: Enclosure Non-Critical", v[dev]);
			}
			if (stat & SES_ENCSTAT_CRITICAL) {
				syslog(LOG_CRIT,
				    "%s: Enclosure Critical", v[dev]);
			}
			if (stat & SES_ENCSTAT_UNRECOV) {
				syslog(LOG_ALERT,
				    "%s: Enclosure Unrecoverable", v[dev]);
			}
		}
		sleep(polltime);
	}
	/* NOTREACHED */
}