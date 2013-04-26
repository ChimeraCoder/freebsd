
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
#include <sys/types.h>
#include <sys/disk.h>
#include <dev/nand/nand_dev.h>
#include "nandtool.h"

int nand_read_oob(struct cmd_param *params)
{
	struct chip_param_io chip_params;
	struct nand_oob_rw req;
	char *dev, *out;
	int fd = -1, fd_out = -1, ret = 0;
	int page;
	uint8_t *buf = NULL;

	if ((page = param_get_int(params, "page")) < 0) {
		fprintf(stderr, "You must supply valid 'page' argument.\n");
		return (1);
	}

	if (!(dev = param_get_string(params, "dev"))) {
		fprintf(stderr, "You must supply 'dev' argument.\n");
		return (1);
	}

	if ((out = param_get_string(params, "out"))) {
		if ((fd_out = open(out, O_WRONLY | O_CREAT)) == -1) {
			perrorf("Cannot open %s", out);
			ret = 1;
			goto out;
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

	buf = malloc(chip_params.oob_size);
	if (buf == NULL) {
		perrorf("Cannot allocate %d bytes\n", chip_params.oob_size);
		ret = 1;
		goto out;
	}

	req.page = page;
	req.len = chip_params.oob_size;
	req.data = buf;

	if (ioctl(fd, NAND_IO_OOB_READ, &req) == -1) {
		perrorf("Cannot read OOB from %s", dev);
		ret = 1;
		goto out;
	}

	if (fd_out != -1)
		write(fd_out, buf, chip_params.oob_size);
	else
		hexdump(buf, chip_params.oob_size);

out:
	close(fd_out);

	if (fd != -1)
		g_close(fd);
	if (buf)
		free(buf);

	return (ret);
}