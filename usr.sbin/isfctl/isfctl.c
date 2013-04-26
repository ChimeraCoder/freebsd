
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
#include <sys/ioctl.h>

#include <err.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* XXXBED: should install and include sys/dev/isf.h */
struct isf_range {
	off_t	ir_off;		/* Offset of range to delete (set to 0xFF) */
	size_t	ir_size;	/* Size of range */
};

#define ISF_ERASE	_IOW('I', 1, struct isf_range)

#define ISF_ERASE_BLOCK (128 * 1024)

static enum {UNSET, ERASE} action = UNSET;

static void
usage(void)
{
	fprintf(stderr, "usage: isfctl <device> erase <offset> <size>\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	struct isf_range	ir;
	int			fd, i;
	char			*p, *dev;
	
	if (argc < 2)
		usage();
	argc--; argv++;
	
	if (*argv[0] == '/')
		dev = argv[0];
	else
		asprintf(&dev, "/dev/%s", argv[0]);
	argc--; argv++;
	fd = open(dev, O_RDWR);
	if (fd < 0)
		err(1, "unable to open device -- %s", dev);

	if (strcmp(argv[0], "erase") == 0) {
		if (argc != 3)
			usage();
		action = ERASE;
		ir.ir_off = strtol(argv[1], &p, 0);
		if (*p)
			errx(1, "invalid offset -- %s", argv[2]);
		ir.ir_size = strtol(argv[2], &p, 0);
		if (*p)
			errx(1, "invalid size -- %s", argv[3]);
		/*
		 * If the user requests to delete less than 32K of space
		 * then assume that they want to delete a number of 128K
		 * blocks.
		 */
		if (ir.ir_size < 32 * 1024)
			ir.ir_size *= 128 * 1024;
	}

	switch (action) {
	case ERASE:
		i = ioctl(fd, ISF_ERASE, &ir);
		if (i < 0)
			err(1, "ioctl(%s, %jx, %zx)", dev,
			    (intmax_t)ir.ir_off, ir.ir_size);
		break;
	default:
		usage();
	}

	close(fd);
	return (0);
}