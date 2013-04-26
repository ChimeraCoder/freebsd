
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

#include <stand.h>

#include "bootstrap.h"
#include "lv1call.h"
#include "ps3bus.h"
#include "ps3repo.h"
#include "ps3stor.h"

int ps3stor_setup(struct ps3_stordev *sd, int type)
{
	unsigned int i;
	int err;

	sd->sd_type = type;

	err = ps3repo_find_bus_by_type(PS3_BUS_TYPE_STOR, &sd->sd_busidx);
	if (err)
		goto out;

	err = ps3repo_read_bus_id(sd->sd_busidx, &sd->sd_busid);
	if (err)
		goto out;

	err = ps3repo_find_bus_dev_by_type(sd->sd_busidx, type, &sd->sd_devidx);
	if (err)
		goto out;

	err = ps3repo_read_bus_dev_id(sd->sd_busidx, sd->sd_devidx,
	    &sd->sd_devid);
	if (err)
		goto out;

	err = ps3repo_read_bus_dev_blk_size(sd->sd_busidx, sd->sd_devidx,
	    &sd->sd_blksize);
	if (err)
		goto out;

	err = ps3repo_read_bus_dev_nblocks(sd->sd_busidx, sd->sd_devidx,
	    &sd->sd_nblocks);
	if (err)
		goto out;

	err = ps3repo_read_bus_dev_nregs(sd->sd_busidx, sd->sd_devidx,
	    &sd->sd_nregs);
	if (err)
		goto out;

	for (i = 0; i < sd->sd_nregs; i++) {
		err = ps3repo_read_bus_dev_reg_id(sd->sd_busidx, sd->sd_devidx,
		    i, &sd->sd_regs[i].sr_id);
		if (err)
			goto out;

		err = ps3repo_read_bus_dev_reg_start(sd->sd_busidx,
		    sd->sd_devidx, i, &sd->sd_regs[i].sr_start);
		if (err)
			goto out;

		err = ps3repo_read_bus_dev_reg_size(sd->sd_busidx,
		    sd->sd_devidx, i, &sd->sd_regs[i].sr_size);
		if (err)
			goto out;
	}

	if (!sd->sd_nregs) {
		err = ENODEV;
		goto out;
	}

	err = lv1_open_device(sd->sd_busid, sd->sd_devid, 0);
	if (err)
		goto out;

	err = lv1_setup_dma(sd->sd_busid, sd->sd_devid, &sd->sd_dmabase);
	if (err)
		goto close_dev;

	return 0;

close_dev:

	lv1_close_device(sd->sd_busid, sd->sd_devid);

out:

	return err;
}

static char dma_buf[2048] __aligned(2048);

int ps3stor_read_sectors(struct ps3_stordev *sd, int regidx,
        uint64_t start_sector, uint64_t sector_count, uint64_t flags, char *buf)
{
#define MIN(a, b)			((a) <= (b) ? (a) : (b))
#define BOUNCE_SECTORS			(sizeof(dma_buf) / sd->sd_blksize)
#define ASYNC_STATUS_POLL_PERIOD	100 /* microseconds */

	struct ps3_storreg *reg = &sd->sd_regs[regidx];
	uint64_t nleft, nread, nsectors;
	uint64_t tag, status;
	unsigned int timeout;
	int err = 0;

	nleft = sector_count;
	nread = 0;

	while (nleft) {
		nsectors = MIN(nleft, BOUNCE_SECTORS);

		err = lv1_storage_read(sd->sd_devid, reg->sr_id,
		    start_sector + nread, nsectors, flags, (uint32_t)dma_buf,
		    &tag);
		if (err)
			return err;

		timeout = 5000000; /* microseconds */

		while (1) {
			if (timeout < ASYNC_STATUS_POLL_PERIOD)
				return ETIMEDOUT;

			err = lv1_storage_check_async_status(sd->sd_devid, tag,
			    &status);
			if (!err && !status)
				break;

			delay(ASYNC_STATUS_POLL_PERIOD);
			timeout -= ASYNC_STATUS_POLL_PERIOD;
		}

		if (status != 0)
			return EIO;

		memcpy(buf + nread * sd->sd_blksize, (u_char *)dma_buf,
		    nsectors * sd->sd_blksize);
		nread += nsectors;
		nleft -= nsectors;
	}

	return err;

#undef MIN
#undef BOUNCE_SECTORS
#undef ASYNC_STATUS_POLL_PERIOD
}

void ps3stor_print(struct ps3_stordev *sd)
{
}