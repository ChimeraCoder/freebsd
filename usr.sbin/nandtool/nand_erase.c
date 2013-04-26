
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
#include <sys/types.h>
#include <sys/disk.h>
#include <libgeom.h>
#include <dev/nand/nand_dev.h>
#include "nandtool.h"

int nand_erase(struct cmd_param *params)
{
	struct chip_param_io chip_params;
	char *dev;
	int fd = -1, ret = 0;
	off_t pos, count;
	off_t start, nblocks, i;
	int block_size, mult;

	if (!(dev = param_get_string(params, "dev"))) {
		fprintf(stderr, "Please supply valid 'dev' parameter.\n");
		return (1);
	}

	if (param_has_value(params, "count"))
		count = param_get_intx(params, "count");
	else
		count = 1;

	if ((fd = g_open(dev, 1)) < 0) {
		perrorf("Cannot open %s", dev);
		return (1);
	}

	if (ioctl(fd, NAND_IO_GET_CHIP_PARAM, &chip_params) == -1) {
		perrorf("Cannot ioctl(NAND_IO_GET_CHIP_PARAM)");
		ret = 1;
		goto out;
	}

	block_size = chip_params.page_size * chip_params.pages_per_block;

	if (param_has_value(params, "page")) {
		pos = chip_params.page_size * param_get_intx(params, "page");
		mult = chip_params.page_size;
	} else if (param_has_value(params, "block")) {
		pos = block_size * param_get_intx(params, "block");
		mult = block_size;
	} else if (param_has_value(params, "pos")) {
		pos = param_get_intx(params, "pos");
		mult = 1;
	} else {
		/* Erase whole chip */
		if (ioctl(fd, DIOCGMEDIASIZE, &count) == -1) {
			ret = 1;
			goto out;
		}

		pos = 0;
		mult = 1;
	}

	if (pos % block_size) {
		fprintf(stderr, "Position must be block-size aligned!\n");
		ret = 1;
		goto out;
	}

	count *= mult;
	start = pos / block_size;
	nblocks = count / block_size;

	for (i = 0; i < nblocks; i++) {
		if (g_delete(fd, (start + i) * block_size, block_size) == -1) {
			perrorf("Cannot erase block %d - probably a bad block",
			    start + i);
			ret = 1;
		}
	}

out:
	g_close(fd);

	return (ret);
}