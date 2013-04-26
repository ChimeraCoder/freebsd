
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

#include "nvme_private.h"

int
nvme_ns_cmd_read(struct nvme_namespace *ns, void *payload, uint64_t lba,
    uint32_t lba_count, nvme_cb_fn_t cb_fn, void *cb_arg)
{
	struct nvme_request	*req;
	struct nvme_command	*cmd;

	req = nvme_allocate_request_vaddr(payload, lba_count*512, cb_fn, cb_arg);

	if (req == NULL)
		return (ENOMEM);
	cmd = &req->cmd;
	cmd->opc = NVME_OPC_READ;
	cmd->nsid = ns->id;

	/* TODO: create a read command data structure */
	*(uint64_t *)&cmd->cdw10 = lba;
	cmd->cdw12 = lba_count-1;

	nvme_ctrlr_submit_io_request(ns->ctrlr, req);

	return (0);
}

int
nvme_ns_cmd_read_bio(struct nvme_namespace *ns, struct bio *bp,
    nvme_cb_fn_t cb_fn, void *cb_arg)
{
	struct nvme_request	*req;
	struct nvme_command	*cmd;
	uint64_t		lba;
	uint64_t		lba_count;

	req = nvme_allocate_request_bio(bp, cb_fn, cb_arg);

	if (req == NULL)
		return (ENOMEM);
	cmd = &req->cmd;
	cmd->opc = NVME_OPC_READ;
	cmd->nsid = ns->id;

	lba = bp->bio_offset / nvme_ns_get_sector_size(ns);
	lba_count = bp->bio_bcount / nvme_ns_get_sector_size(ns);

	/* TODO: create a read command data structure */
	*(uint64_t *)&cmd->cdw10 = lba;
	cmd->cdw12 = lba_count-1;

	nvme_ctrlr_submit_io_request(ns->ctrlr, req);

	return (0);
}

int
nvme_ns_cmd_write(struct nvme_namespace *ns, void *payload, uint64_t lba,
    uint32_t lba_count, nvme_cb_fn_t cb_fn, void *cb_arg)
{
	struct nvme_request	*req;
	struct nvme_command	*cmd;

	req = nvme_allocate_request_vaddr(payload, lba_count*512, cb_fn,
	    cb_arg);

	if (req == NULL)
		return (ENOMEM);

	cmd = &req->cmd;
	cmd->opc = NVME_OPC_WRITE;
	cmd->nsid = ns->id;

	/* TODO: create a write command data structure */
	*(uint64_t *)&cmd->cdw10 = lba;
	cmd->cdw12 = lba_count-1;

	nvme_ctrlr_submit_io_request(ns->ctrlr, req);

	return (0);
}

int
nvme_ns_cmd_write_bio(struct nvme_namespace *ns, struct bio *bp,
    nvme_cb_fn_t cb_fn, void *cb_arg)
{
	struct nvme_request	*req;
	struct nvme_command	*cmd;
	uint64_t		lba;
	uint64_t		lba_count;

	req = nvme_allocate_request_bio(bp, cb_fn, cb_arg);

	if (req == NULL)
		return (ENOMEM);
	cmd = &req->cmd;
	cmd->opc = NVME_OPC_WRITE;
	cmd->nsid = ns->id;

	lba = bp->bio_offset / nvme_ns_get_sector_size(ns);
	lba_count = bp->bio_bcount / nvme_ns_get_sector_size(ns);

	/* TODO: create a write command data structure */
	*(uint64_t *)&cmd->cdw10 = lba;
	cmd->cdw12 = lba_count-1;

	nvme_ctrlr_submit_io_request(ns->ctrlr, req);

	return (0);
}

int
nvme_ns_cmd_deallocate(struct nvme_namespace *ns, void *payload,
    uint8_t num_ranges, nvme_cb_fn_t cb_fn, void *cb_arg)
{
	struct nvme_request	*req;
	struct nvme_command	*cmd;

	req = nvme_allocate_request_vaddr(payload,
	    num_ranges * sizeof(struct nvme_dsm_range), cb_fn, cb_arg);

	if (req == NULL)
		return (ENOMEM);

	cmd = &req->cmd;
	cmd->opc = NVME_OPC_DATASET_MANAGEMENT;
	cmd->nsid = ns->id;

	/* TODO: create a delete command data structure */
	cmd->cdw10 = num_ranges - 1;
	cmd->cdw11 = NVME_DSM_ATTR_DEALLOCATE;

	nvme_ctrlr_submit_io_request(ns->ctrlr, req);

	return (0);
}

int
nvme_ns_cmd_flush(struct nvme_namespace *ns, nvme_cb_fn_t cb_fn, void *cb_arg)
{
	struct nvme_request	*req;
	struct nvme_command	*cmd;

	req = nvme_allocate_request_null(cb_fn, cb_arg);

	if (req == NULL)
		return (ENOMEM);

	cmd = &req->cmd;
	cmd->opc = NVME_OPC_FLUSH;
	cmd->nsid = ns->id;

	nvme_ctrlr_submit_io_request(ns->ctrlr, req);

	return (0);
}