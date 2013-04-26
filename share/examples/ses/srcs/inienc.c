
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

	while (*++v) {
		fd = open(*v, O_RDWR);
		if (fd < 0) {
			perror(*v);
			continue;
		}
		if (ioctl(fd, SESIOC_INIT, NULL) < 0) {
			perror("SESIOC_GETNOBJ");
		}
		(void) close(fd);
	}
	return (0);
}