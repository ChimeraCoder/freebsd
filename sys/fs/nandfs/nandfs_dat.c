
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/namei.h>
#include <sys/sysctl.h>
#include <sys/vnode.h>
#include <sys/buf.h>
#include <sys/bio.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>

#include <fs/nandfs/nandfs_mount.h>
#include <fs/nandfs/nandfs.h>
#include <fs/nandfs/nandfs_subr.h>

int
nandfs_vblock_alloc(struct nandfs_device *nandfsdev, nandfs_daddr_t *vblock)
{
	struct nandfs_node *dat;
	struct nandfs_mdt *mdt;
	struct nandfs_alloc_request req;
	struct nandfs_dat_entry *dat_entry;
	uint64_t start;
	uint32_t entry;
	int locked, error;

	dat = nandfsdev->nd_dat_node;
	mdt = &nandfsdev->nd_dat_mdt;
	start = nandfsdev->nd_last_cno + 1;

	locked = NANDFS_VOP_ISLOCKED(NTOV(dat));
	if (!locked)
		VOP_LOCK(NTOV(dat), LK_EXCLUSIVE);
	req.entrynum = 0;

	/* Alloc vblock number */
	error = nandfs_find_free_entry(mdt, dat, &req);
	if (error) {
		nandfs_error("%s: cannot find free vblk entry\n",
		    __func__);
		if (!locked)
			VOP_UNLOCK(NTOV(dat), 0);
		return (error);
	}

	/* Read/create buffer */
	error = nandfs_get_entry_block(mdt, dat, &req, &entry, 1);
	if (error) {
		nandfs_error("%s: cannot get free vblk entry\n",
		    __func__);
		nandfs_abort_entry(&req);
		if (!locked)
			VOP_UNLOCK(NTOV(dat), 0);
		return (error);
	}

	/* Fill out vblock data */
	dat_entry = (struct nandfs_dat_entry *) req.bp_entry->b_data;
	dat_entry[entry].de_start = start;
	dat_entry[entry].de_end = UINTMAX_MAX;
	dat_entry[entry].de_blocknr = 0;

	/* Commit allocation */
	error = nandfs_alloc_entry(mdt, &req);
	if (error) {
		nandfs_error("%s: cannot get free vblk entry\n",
		    __func__);
		if (!locked)
			VOP_UNLOCK(NTOV(dat), 0);
		return (error);
	}

	/* Return allocated vblock */
	*vblock = req.entrynum;
	DPRINTF(DAT, ("%s: allocated vblock %#jx\n",
	    __func__, (uintmax_t)*vblock));

	if (!locked)
		VOP_UNLOCK(NTOV(dat), 0);
	return (error);
}

int
nandfs_vblock_assign(struct nandfs_device *nandfsdev, nandfs_daddr_t vblock,
    nandfs_lbn_t block)
{
	struct nandfs_node *dat;
	struct nandfs_mdt *mdt;
	struct nandfs_alloc_request req;
	struct nandfs_dat_entry *dat_entry;
	uint32_t entry;
	int locked, error;

	dat = nandfsdev->nd_dat_node;
	mdt = &nandfsdev->nd_dat_mdt;

	locked = NANDFS_VOP_ISLOCKED(NTOV(dat));
	if (!locked)
		VOP_LOCK(NTOV(dat), LK_EXCLUSIVE);
	req.entrynum = vblock;

	error = nandfs_get_entry_block(mdt, dat, &req, &entry, 0);
	if (!error) {
		dat_entry = (struct nandfs_dat_entry *) req.bp_entry->b_data;
		dat_entry[entry].de_blocknr = block;

		DPRINTF(DAT, ("%s: assing vblock %jx->%jx\n",
		    __func__, (uintmax_t)vblock, (uintmax_t)block));

		/*
		 * It is mostly called from syncer() so
		 * we want to force making buf dirty
		 */
		error = nandfs_dirty_buf(req.bp_entry, 1);
	}

	if (!locked)
		VOP_UNLOCK(NTOV(dat), 0);

	return (error);
}

int
nandfs_vblock_end(struct nandfs_device *nandfsdev, nandfs_daddr_t vblock)
{
	struct nandfs_node *dat;
	struct nandfs_mdt *mdt;
	struct nandfs_alloc_request req;
	struct nandfs_dat_entry *dat_entry;
	uint64_t end;
	uint32_t entry;
	int locked, error;

	dat = nandfsdev->nd_dat_node;
	mdt = &nandfsdev->nd_dat_mdt;
	end = nandfsdev->nd_last_cno;

	locked = NANDFS_VOP_ISLOCKED(NTOV(dat));
	if (!locked)
		VOP_LOCK(NTOV(dat), LK_EXCLUSIVE);
	req.entrynum = vblock;

	error = nandfs_get_entry_block(mdt, dat, &req, &entry, 0);
	if (!error) {
		dat_entry = (struct nandfs_dat_entry *) req.bp_entry->b_data;
		dat_entry[entry].de_end = end;
		DPRINTF(DAT, ("%s: end vblock %#jx at checkpoint %#jx\n",
		    __func__, (uintmax_t)vblock, (uintmax_t)end));

		/*
		 * It is mostly called from syncer() so
		 * we want to force making buf dirty
		 */
		error = nandfs_dirty_buf(req.bp_entry, 1);
	}

	if (!locked)
		VOP_UNLOCK(NTOV(dat), 0);

	return (error);
}

int
nandfs_vblock_free(struct nandfs_device *nandfsdev, nandfs_daddr_t vblock)
{
	struct nandfs_node *dat;
	struct nandfs_mdt *mdt;
	struct nandfs_alloc_request req;
	int error;

	dat = nandfsdev->nd_dat_node;
	mdt = &nandfsdev->nd_dat_mdt;

	VOP_LOCK(NTOV(dat), LK_EXCLUSIVE);
	req.entrynum = vblock;

	error = nandfs_find_entry(mdt, dat, &req);
	if (!error) {
		DPRINTF(DAT, ("%s: vblk %#jx\n", __func__, (uintmax_t)vblock));
		nandfs_free_entry(mdt, &req);
	}

	VOP_UNLOCK(NTOV(dat), 0);
	return (error);
}

int
nandfs_get_dat_vinfo_ioctl(struct nandfs_device *nandfsdev, struct nandfs_argv *nargv)
{
	struct nandfs_vinfo *vinfo;
	size_t size;
	int error;

	if (nargv->nv_nmembs > NANDFS_VINFO_MAX)
		return (EINVAL);

	size = sizeof(struct nandfs_vinfo) * nargv->nv_nmembs;
	vinfo = malloc(size, M_NANDFSTEMP, M_WAITOK|M_ZERO);

	error = copyin((void *)(uintptr_t)nargv->nv_base, vinfo, size);
	if (error) {
		free(vinfo, M_NANDFSTEMP);
		return (error);
	}

	error = nandfs_get_dat_vinfo(nandfsdev, vinfo, nargv->nv_nmembs);
	if (error == 0)
		error =	copyout(vinfo, (void *)(uintptr_t)nargv->nv_base, size);
	free(vinfo, M_NANDFSTEMP);
	return (error);
}

int
nandfs_get_dat_vinfo(struct nandfs_device *nandfsdev, struct nandfs_vinfo *vinfo,
    uint32_t nmembs)
{
	struct nandfs_node *dat;
	struct nandfs_mdt *mdt;
	struct nandfs_alloc_request req;
	struct nandfs_dat_entry *dat_entry;
	uint32_t i, idx;
	int error = 0;

	dat = nandfsdev->nd_dat_node;
	mdt = &nandfsdev->nd_dat_mdt;

	DPRINTF(DAT, ("%s: nmembs %#x\n", __func__, nmembs));

	VOP_LOCK(NTOV(dat), LK_EXCLUSIVE);

	for (i = 0; i < nmembs; i++) {
		req.entrynum = vinfo[i].nvi_vblocknr;

		error = nandfs_get_entry_block(mdt, dat,&req, &idx, 0);
		if (error)
			break;

		dat_entry = ((struct nandfs_dat_entry *) req.bp_entry->b_data);
		vinfo[i].nvi_start = dat_entry[idx].de_start;
		vinfo[i].nvi_end = dat_entry[idx].de_end;
		vinfo[i].nvi_blocknr = dat_entry[idx].de_blocknr;

		DPRINTF(DAT, ("%s: vinfo: %jx[%jx-%jx]->%jx\n",
		    __func__, vinfo[i].nvi_vblocknr, vinfo[i].nvi_start,
		    vinfo[i].nvi_end, vinfo[i].nvi_blocknr));

		brelse(req.bp_entry);
	}

	VOP_UNLOCK(NTOV(dat), 0);
	return (error);
}

int
nandfs_get_dat_bdescs_ioctl(struct nandfs_device *nffsdev,
    struct nandfs_argv *nargv)
{
	struct nandfs_bdesc *bd;
	size_t size;
	int error;

	size = nargv->nv_nmembs * sizeof(struct nandfs_bdesc);
	bd = malloc(size, M_NANDFSTEMP, M_WAITOK);
	error = copyin((void *)(uintptr_t)nargv->nv_base, bd, size);
	if (error) {
		free(bd, M_NANDFSTEMP);
		return (error);
	}

	error = nandfs_get_dat_bdescs(nffsdev, bd, nargv->nv_nmembs);

	if (error == 0)
		error =	copyout(bd, (void *)(uintptr_t)nargv->nv_base, size);

	free(bd, M_NANDFSTEMP);
	return (error);
}

int
nandfs_get_dat_bdescs(struct nandfs_device *nffsdev, struct nandfs_bdesc *bd,
    uint32_t nmembs)
{
	struct nandfs_node *dat_node;
	uint64_t map;
	uint32_t i;
	int error = 0;

	dat_node = nffsdev->nd_dat_node;

	VOP_LOCK(NTOV(dat_node), LK_EXCLUSIVE);

	for (i = 0; i < nmembs; i++) {
		DPRINTF(CLEAN,
		    ("%s: bd ino:%#jx oblk:%#jx blocknr:%#jx off:%#jx\n",
		    __func__,  (uintmax_t)bd[i].bd_ino,
		    (uintmax_t)bd[i].bd_oblocknr, (uintmax_t)bd[i].bd_blocknr,
		    (uintmax_t)bd[i].bd_offset));

		error = nandfs_bmap_lookup(dat_node, bd[i].bd_offset, &map);
		if (error)
			break;
		bd[i].bd_blocknr = map;
	}

	VOP_UNLOCK(NTOV(dat_node), 0);
	return (error);
}