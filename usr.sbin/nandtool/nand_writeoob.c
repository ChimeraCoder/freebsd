
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

int nand_write_oob(struct cmd_param *params)
{
	struct chip_param_io chip_params;
	struct nand_oob_rw req;
	char *dev, *in;
	int fd = -1, fd_in = -1, ret = 0;
	uint8_t *buf = NULL;
	int page;

	if (!(dev = param_get_string(params, "dev"))) {
		fprintf(stderr, "Please supply valid 'dev' parameter.\n");
		return (1);
	}

	if (!(in = param_get_string(params, "in"))) {
		fprintf(stderr, "Please supply valid 'in' parameter.\n");
		return (1);
	}

	if ((page = param_get_int(params, "page")) < 0) {
		fprintf(stderr, "Please supply valid 'page' parameter.\n");
		return (1);
	}

	if ((fd = g_open(dev, 1)) == -1) {
		perrorf("Cannot open %s", dev);
		return (1);
	}

	if ((fd_in = open(in, O_RDONLY)) == -1) {
		perrorf("Cannot open %s", in);
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

	if (read(fd_in, buf, chip_params.oob_size) == -1) {
		perrorf("Cannot read from %s", in);
		ret = 1;
		goto out;
	}

	req.page = page;
	req.len = chip_params.oob_size;
	req.data = buf;

	if (ioctl(fd, NAND_IO_OOB_PROG, &req) == -1) {
		perrorf("Cannot write OOB to %s", dev);
		ret = 1;
		goto out;
	}

out:
	g_close(fd);
	if (fd_in != -1)
		close(fd_in);
	if (buf)
		free(buf);

	return (ret);
}