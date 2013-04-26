
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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/*
 * From the driver itself
 */
#include <plugins/visibility_ioctl.h>

static int dev = -1;

static void
toggle_medium(int op)
{
	if (ioctl(dev, VISIOCTLOPEN, &op) < 0) {
		printf("error opening/closing medium\n");
	}
}

static void
link_op(struct link *l)
{
	if (ioctl(dev, VISIOCTLLINK, l) < 0) {
		printf("error making a link operation\n");
	}
}

static void
usage(const char *argv[])
{
	printf("usage: %s [o | c | [ [a|d]  wtap_id1  wtap_id2]]\n",
	    argv[0]);
}

int
main(int argc, const char* argv[])
{
	struct link l;
	char cmd;

	if (argc < 2) {
		usage(argv);
		exit(1);
	}

	dev = open("/dev/visctl", O_RDONLY);
		if (dev < 0) {
			printf("error opening visctl cdev\n");
			exit(1);
	}

	cmd = (char)*argv[1];

	switch (cmd) {
	case 'o':
		toggle_medium(1);
		break;
	case 'c':
		toggle_medium(0);
		break;
	case 'a':
		if (argc < 4) {
			usage(argv);
			exit(1);
		}
		l.op = 1;
		l.id1 = atoi(argv[2]);
		l.id2 = atoi(argv[3]);
		link_op(&l);
		break;
	case 'd':
		if (argc < 4) {
			usage(argv);
			exit(1);
		}
		l.op = 0;
		l.id1 = atoi(argv[2]);
		l.id2 = atoi(argv[3]);
		link_op(&l);
		break;
	default:
		printf("wtap ioctl: unkown command '%c'\n", *argv[1]);
		exit(1);
	}
	exit(0);
}