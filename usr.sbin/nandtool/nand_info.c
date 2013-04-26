
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
#include <stdint.h>
#include <string.h>
#include <libgeom.h>
#include <sys/disk.h>
#include <dev/nand/nand_dev.h>
#include "nandtool.h"

int nand_info(struct cmd_param *params)
{
	struct chip_param_io chip_params;
	int fd = -1, ret = 0;
	int block_size;
	off_t chip_size, media_size;
	const char *dev;

	if ((dev = param_get_string(params, "dev")) == NULL) {
		fprintf(stderr, "Please supply 'dev' parameter, eg. "
		    "'dev=/dev/gnand0'\n");
		return (1);
	}

	if ((fd = g_open(dev, 1)) == -1) {
		perrorf("Cannot open %s", dev);
		return (1);
	}

	if (ioctl(fd, NAND_IO_GET_CHIP_PARAM, &chip_params) == -1) {
		perrorf("Cannot ioctl(NAND_IO_GET_CHIP_PARAM)");
		ret = 1;
		goto out;
	}

	if (ioctl(fd, DIOCGMEDIASIZE, &media_size) == -1) {
		perrorf("Cannot ioctl(DIOCGMEDIASIZE)");
		ret = 1;
		goto out;
	}

	block_size = chip_params.page_size * chip_params.pages_per_block;
	chip_size = block_size * chip_params.blocks;

	printf("Device:\t\t\t%s\n", dev);
	printf("Page size:\t\t%d bytes\n", chip_params.page_size);
	printf("Block size:\t\t%d bytes (%d KB)\n", block_size,
	    block_size / 1024);
	printf("OOB size per page:\t%d bytes\n", chip_params.oob_size);
	printf("Chip size:\t\t%jd MB\n", (uintmax_t)(chip_size / 1024 / 1024));
	printf("Slice size:\t\t%jd MB\n",
	    (uintmax_t)(media_size / 1024 / 1024));

out:
	g_close(fd);

	return (ret);
}