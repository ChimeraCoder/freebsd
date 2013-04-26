
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

#include <dev/ppbus/lptio.h>

#include <err.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_DEVICE	"/dev/lpt0.ctl"
#define IRQ_UNSPECIFIED	-1
#define DO_POLL		0
#define USE_IRQ		1
#define USE_EXT_MODE	2
#define USE_STD_MODE	3

static void usage(void)
{
	fprintf(stderr,
		"usage: lptcontrol -e | -i | -p | -s [[-d] controldevice]\n");
	exit(1);
}

int main (int argc, char **argv)
{
	const char *device;
	int fd;
	int irq_status;
	int opt;

	device = DEFAULT_DEVICE;
	irq_status = IRQ_UNSPECIFIED;
	while ((opt = getopt(argc, argv, "d:eips")) != -1)
		switch (opt) {
		case 'd':
			device = optarg;
			break;
		case 'e':
			irq_status = USE_EXT_MODE;
			break;
		case 'i':
			irq_status = USE_IRQ;
			break;
		case 'p':
			irq_status = DO_POLL;
			break;
		case 's':
			irq_status = USE_STD_MODE;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;
	/* POLA: DTRT if -d was forgotten, but device name was specified. */
	if (argc == 1) {
		device = argv[0];
		--argc;
	}

	if (irq_status == IRQ_UNSPECIFIED || argc != 0)
		usage();

	if ((fd = open(device, O_WRONLY)) < 0)
		err(1, "open");
	if (ioctl(fd, LPT_IRQ, &irq_status) < 0)
		err(1, "ioctl");
	close(fd);

	return(0);
}