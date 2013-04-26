
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

int nand_write(struct cmd_param *params)
{
	struct chip_param_io chip_params;
	char *dev, *file;
	int in_fd = -1, ret = 0, done = 0;
	int fd, block_size, mult, pos, count;
	uint8_t *buf = NULL;

	if (!(dev = param_get_string(params, "dev"))) {
		fprintf(stderr, "Please supply 'dev' argument.\n");
		return (1);
	}

	if (!(file = param_get_string(params, "in"))) {
		fprintf(stderr, "Please supply 'in' argument.\n");
		return (1);
	}

	if ((fd = g_open(dev, 1)) == -1) {
		perrorf("Cannot open %s", dev);
		return (1);
	}

	if ((in_fd = open(file, O_RDONLY)) == -1) {
		perrorf("Cannot open file %s", file);
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
			fprintf(stderr, "Position must be page-size "
			    "aligned!\n");
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
		if ((ret = read(in_fd, buf, chip_params.page_size)) !=
		    (int32_t)chip_params.page_size) {
			if (ret > 0) {
				/* End of file ahead, truncate here */
				break;
			} else {
				perrorf("Cannot read from %s", file);
				ret = 1;
				goto out;
			}
		}

		if ((ret = write(fd, buf, chip_params.page_size)) !=
		    (int32_t)chip_params.page_size) {
			ret = 1;
			goto out;
		}

		done += ret;
	}

out:
	g_close(fd);
	if (in_fd != -1)
		close(in_fd);
	if (buf)
		free(buf);

	return (ret);
}