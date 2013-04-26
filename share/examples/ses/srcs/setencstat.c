
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
	long val;
	encioc_enc_status_t stat;

	if (a != 3) {
		fprintf(stderr, "usage: %s device enclosure_status\n", *v);
		return (1);
	}
	fd = open(v[1], O_RDWR);
	if (fd < 0) {
		perror(v[1]);
		return (1);
	}
	
	val =  strtol(v[2], NULL, 0);
	stat = (encioc_enc_status_t)val;
	if (ioctl(fd, ENCIOC_SETENCSTAT, (caddr_t) &stat) < 0) {
		perror("ENCIOC_SETENCSTAT");
	}
	(void) close(fd);
	return (0);
}