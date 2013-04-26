
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
#include <sys/types.h>
#include <sys/ioccom.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>

#define CDEV_IOCTL1     _IOR('C', 1, u_int)
#define CDEV_DEVICE	"cdev"

static char writestr[] = "Hello kernel!";
static char buf[512+1];

int
main(int argc __unused, char *argv[] __unused)
{
    int kernel_fd;
    int one;
    int len;

    if ((kernel_fd = open("/dev/" CDEV_DEVICE, O_RDWR)) == -1) {
	perror("/dev/" CDEV_DEVICE);
	exit(1);
    }

    /* Send ioctl */
    if (ioctl(kernel_fd, CDEV_IOCTL1, &one) == -1) {
	perror("CDEV_IOCTL1");
    } else {
	printf( "Sent ioctl CDEV_IOCTL1 to device %s%s\n", _PATH_DEV, CDEV_DEVICE);
    }

    len = strlen(writestr) + 1;

    /* Write operation */
    if (write(kernel_fd, writestr, len) == -1) {
	perror("write()");
    } else {
	printf("Written \"%s\" string to device /dev/" CDEV_DEVICE "\n", writestr);
    }

    /* Read operation */
    if (read(kernel_fd, buf, len) == -1) {
	perror("read()");
    } else {
	printf("Read \"%s\" string from device /dev/" CDEV_DEVICE "\n", buf);
    }

    exit(0);
}