
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
#include <sys/ioctl.h>
#include <cam/scsi/scsi_all.h>
#include <cam/scsi/scsi_enc.h>

int
main(int a, char **v)
{
	int fd;
	int i;
	encioc_elm_status_t obj;
	long cvt;
	char *x;

	if (a != 7) {
usage:
		fprintf(stderr,
		    "usage: %s device objectid stat0 stat1 stat2 stat3\n", *v);
		return (1);
	}
	fd = open(v[1], O_RDWR);
	if (fd < 0) {
		perror(v[1]);
		return (1);
	}
	x = v[2];
	cvt = strtol(v[2], &x, 0);
	if (x == v[2]) {
		goto usage;
	}
	obj.elm_idx = cvt;
	for (i = 0; i < 4; i++) {
		x = v[3 + i];
		cvt = strtol(v[3 + i],  &x, 0);
		if (x == v[3 + i]) {
			goto usage;
		}
		obj.cstat[i] = cvt;
	}
	if (ioctl(fd, ENCIOC_SETELMSTAT, (caddr_t) &obj) < 0) {
		perror("ENCIOC_SETELMSTAT");
	}
	(void) close(fd);
	return (0);
}