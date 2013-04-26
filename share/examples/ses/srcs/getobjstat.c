
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
#include <cam/scsi/scsi_ses.h>

int
main(int a, char **v)
{
	int fd;
	int i;
	ses_objstat obj;
	long cvt;
	char *x;

	if (a != 3) {
usage:
		fprintf(stderr, "usage: %s device objectid\n", *v);
		return (1);
	}
	fd = open(v[1], O_RDONLY);
	if (fd < 0) {
		perror(v[1]);
		return (1);
	}
	x = v[2];
	cvt = strtol(v[2], &x, 0);
	if (x == v[2]) {
		goto usage;
	}
	obj.obj_id = cvt;
	if (ioctl(fd, SESIOC_GETOBJSTAT, (caddr_t) &obj) < 0) {
		perror("SESIOC_GETOBJSTAT");
		return (1);
	}
	fprintf(stdout, "Object 0x%x: 0x%x 0x%x 0x%x 0x%x\n", obj.obj_id,
	    obj.cstat[0], obj.cstat[1], obj.cstat[2], obj.cstat[3]);
	(void) close(fd);
	return (0);
}