
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgeom.h>
#include <sys/disk.h>
#include <dev/nand/nand_dev.h>
#include "nandtool.h"

int nand_read(struct cmd_param *params)
{
	struct chip_param_io chip_params;
	int fd = -1, out_fd = -1, done = 0, ret = 0;
	char *dev, *out;
	int pos, count, mult, block_size;
	uint8_t *buf = NULL;

	if (!(dev = param_get_string(params, "dev"))) {
		fprintf(stderr, "You must specify 'dev' parameter\n");
		return (1);
	}

	if ((out = param_get_string(params, "out"))) {
		out_fd = open(out, O_WRONLY|O_CREAT);
		if (out_fd == -1) {
			perrorf("Cannot open %s for writing", out);
			return (1);
		}
	}

	if ((fd = g_open(dev, 1)) == -1) {
		perrorf("Cannot open %s", dev);
		ret = 1;
		goto out;
	}

	if (ioctl(fd, NAND_IO_GET_CHIP_PARAM, &chip_params) == -1) {
		perrorf("Cannot ioctl(NAND_IO_GET_CHIP_PARAM)");
		ret = 1;
		goto out;
	}

	block_size = chip_params.page_size * chip_params.pages_per_block;

	if (param_has_value(params, "page")) {
		pos = chip_params.page_size * param_get_int(params, "page");
		mult = chip_params.page_size;
	} else if (param_has_value(params, "block")) {
		pos = block_size * param_get_int(params, "block");
		mult = block_size;
	} else if (param_has_value(params, "pos")) {
		pos = param_get_int(params, "pos");
		mult = 1;
		if (pos % chip_params.page_size) {
			fprintf(stderr, "Position must be page-size aligned!\n");
			ret = 1;
			goto out;
		}
	} else {
		fprintf(stderr, "You must specify one of: 'block', 'page',"
		    "'pos' arguments\n");
		ret = 1;
		goto out;
	}

	if (!(param_has_value(params, "count")))
		count = mult;
	else
		count = param_get_int(params, "count") * mult;

	if (!(buf = malloc(chip_params.page_size))) {
		perrorf("Cannot allocate buffer [size %x]",
		    chip_params.page_size);
		ret = 1;
		goto out;
	}

	lseek(fd, pos, SEEK_SET);

	while (done < count) {
		if ((ret = read(fd, buf, chip_params.page_size)) !=
		    (int32_t)chip_params.page_size) {
			perrorf("read error (read %d bytes)", ret);
			goto out;
		}

		if (out_fd != -1) {
			done += ret;
			if ((ret = write(out_fd, buf, chip_params.page_size)) !=
			    (int32_t)chip_params.page_size) {
				perrorf("write error (written %d bytes)", ret);
				ret = 1;
				goto out;
			}
		} else {
			hexdumpoffset(buf, chip_params.page_size, done);
			done += ret;
		}
	}

out:
	g_close(fd);
	if (out_fd != -1)
		close(out_fd);
	if (buf)
		free(buf);

	return (ret);
}