
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
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

#include </sys/dev/mfi/mfireg.h>
#include </sys/dev/mfi/mfi_ioctl.h>

#include <errno.h>
#include <stdio.h>

/*
 * Simple program to print out the queue stats on the given queue index.
 * See /sys/sys/mfi_ioctl.h for the definitions of each queue index.
 */

void
usage(void)
{
	printf("Usage: mfi_checkq <queue_number>\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	union mfi_statrequest sr;
	int fd, retval, queue;

	if (argc != 2)
		usage();

	fd = open("/dev/mfi0", O_RDWR);
	if (fd == -1) {
		printf("couldn't open mfi0: %s\n", strerror(errno));
		return (-1);
	}

	queue = atoi(argv[1]);
	printf("Getting stats for queue %d\n", queue);
	bzero(&sr, sizeof(union mfi_statrequest));
	sr.ms_item = queue;
	retval = ioctl(fd, MFIIO_STATS, &sr);
	if (retval == -1) {
		printf("error on ioctl: %s\n", strerror(errno));
		return (-1);
	}

	printf("length= %d, max= %d\n",sr.ms_qstat.q_length, sr.ms_qstat.q_max);

	close(fd);
	return 0;
}