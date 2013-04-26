
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
#include <sys/namei.h>
#include <sys/kernel.h>
#include <sys/stat.h>
#include <sys/buf.h>
#include <sys/bio.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/signalvar.h>
#include <sys/malloc.h>
#include <sys/dirent.h>
#include <sys/lockf.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>

#include "nandfs_mount.h"
#include "nandfs.h"
#include "nandfs_subr.h"

int
nandfs_add_dirent(struct vnode *dvp, uint64_t ino, char *nameptr, long namelen,
    uint8_t type)
{
	struct nandfs_node *dir_node = VTON(dvp);
	struct nandfs_dir_entry *dirent, *pdirent;
	uint32_t blocksize = dir_node->nn_nandfsdev->nd_blocksize;
	uint64_t filesize = dir_node->nn_inode.i_size;
	uint64_t inode_blks = dir_node->nn_inode.i_blocks;
	uint32_t off, rest;
	uint8_t *pos;
	struct buf *bp;
	int error;

	pdirent = NULL;
	bp = NULL;
	if (inode_blks) {
		error = nandfs_bread(dir_node, inode_blks - 1, NOCRED, 0, &bp);
		if (error) {
			brelse(bp);
			return (error);
		}

		pos = bp->b_data;
		off = 0;
		while (off < blocksize) {
			pdirent = (struct nandfs_dir_entry *) (pos + off);
			if (!pdirent->rec_len) {
				pdirent = NULL;
				break;
			}
			off += pdirent->rec_len;
		}

		if (pdirent)
			rest = pdirent->rec_len -
			    NANDFS_DIR_REC_LEN(pdirent->name_len);
		else
			rest = blocksize;

		if (rest < NANDFS_DIR_REC_LEN(namelen)) {
			/* Do not update pdirent as new block is created */
			pdirent = NULL;
			brelse(bp);
			/* Set to NULL to create new */
			bp = NULL;
			filesize += rest;
		}
	}

	/* If no bp found create new */
	if (!bp) {
		error = nandfs_bcreate(dir_node, inode_blks, NOCRED, 0, &bp);
		if (error)
			return (error);
		off = 0;
		pos = bp->b_data;
	}

	/* Modify pdirent if exists */
	if (pdirent) {
		DPRINTF(LOOKUP, ("modify pdirent %p\n", pdirent));
		/* modify last de */
		off -= pdirent->rec_len;
		pdirent->rec_len =
		    NANDFS_DIR_REC_LEN(pdirent->name_len);
		off += pdirent->rec_len;
	}

	/* Create new dirent */
	dirent = (struct nandfs_dir_entry *) (pos + off);
	dirent->rec_len = blocksize - off;
	dirent->inode = ino;
	dirent->name_len = namelen;
	memset(dirent->name, 0, NANDFS_DIR_NAME_LEN(namelen));
	memcpy(dirent->name, nameptr, namelen);
	dirent->file_type = type;

	filesize += NANDFS_DIR_REC_LEN(dirent->name_len);

	DPRINTF(LOOKUP, ("create dir_entry '%.*s' at %p with size %x "
	    "new filesize: %jx\n",
	    (int)namelen, dirent->name, dirent, dirent->rec_len,
	    (uintmax_t)filesize));

	error = nandfs_dirty_buf(bp, 0);
	if (error)
		return (error);

	dir_node->nn_inode.i_size = filesize;
	dir_node->nn_flags |= IN_CHANGE | IN_UPDATE;
	vnode_pager_setsize(dvp, filesize);

	return (0);
}

int
nandfs_remove_dirent(struct vnode *dvp, struct nandfs_node *node,
    struct componentname *cnp)
{
	struct nandfs_node *dir_node;
	struct nandfs_dir_entry *dirent, *pdirent;
	struct buf *bp;
	uint64_t filesize, blocknr, ino, offset;
	uint32_t blocksize, limit, off;
	uint16_t newsize;
	uint8_t *pos;
	int error, found;

	dir_node = VTON(dvp);
	filesize = dir_node->nn_inode.i_size;
	if (!filesize)
		return (0);

	if (node) {
		offset = node->nn_diroff;
		ino = node->nn_ino;
	} else {
		offset = dir_node->nn_diroff;
		ino = NANDFS_WHT_INO;
	}

	dirent = pdirent = NULL;
	blocksize = dir_node->nn_nandfsdev->nd_blocksize;
	blocknr = offset / blocksize;

	DPRINTF(LOOKUP, ("rm direntry dvp %p node %p ino %#jx at off %#jx\n",
	    dvp, node, (uintmax_t)ino, (uintmax_t)offset));

	error = nandfs_bread(dir_node, blocknr, NOCRED, 0, &bp);
	if (error) {
		brelse(bp);
		return (error);
	}

	pos = bp->b_data;
	off = 0;
	found = 0;
	limit = offset % blocksize;
	pdirent = (struct nandfs_dir_entry *) bp->b_data;
	while (off <= limit) {
		dirent = (struct nandfs_dir_entry *) (pos + off);

		if ((off == limit) &&
		    (dirent->inode == ino)) {
			found = 1;
			break;
		}
		if (dirent->inode != 0)
			pdirent = dirent;
		off += dirent->rec_len;
	}

	if (!found) {
		nandfs_error("cannot find entry to remove");
		brelse(bp);
		return (error);
	}
	DPRINTF(LOOKUP,
	    ("rm dirent ino %#jx at %#x with size %#x\n",
	    (uintmax_t)dirent->inode, off, dirent->rec_len));

	newsize = (uintptr_t)dirent - (uintptr_t)pdirent;
	newsize += dirent->rec_len;
	pdirent->rec_len = newsize;
	dirent->inode = 0;
	error = nandfs_dirty_buf(bp, 0);
	if (error)
		return (error);

	dir_node->nn_flags |= IN_CHANGE | IN_UPDATE;
	/* If last one modify filesize */
	if ((offset + NANDFS_DIR_REC_LEN(dirent->name_len)) == filesize) {
		filesize = blocknr * blocksize +
		    ((uintptr_t)pdirent - (uintptr_t)pos) +
		    NANDFS_DIR_REC_LEN(pdirent->name_len);
		dir_node->nn_inode.i_size = filesize;
	}

	return (0);
}

int
nandfs_update_parent_dir(struct vnode *dvp, uint64_t newparent)
{
	struct nandfs_dir_entry *dirent;
	struct nandfs_node *dir_node;
	struct buf *bp;
	int error;

	dir_node = VTON(dvp);
	error = nandfs_bread(dir_node, 0, NOCRED, 0, &bp);
	if (error) {
		brelse(bp);
		return (error);
	}
	dirent = (struct nandfs_dir_entry *)bp->b_data;
	dirent->inode = newparent;
	error = nandfs_dirty_buf(bp, 0);
	if (error)
		return (error);

	return (0);
}

int
nandfs_update_dirent(struct vnode *dvp, struct nandfs_node *fnode,
    struct nandfs_node *tnode)
{
	struct nandfs_node *dir_node;
	struct nandfs_dir_entry *dirent;
	struct buf *bp;
	uint64_t file_size, blocknr;
	uint32_t blocksize, off;
	uint8_t *pos;
	int error;

	dir_node = VTON(dvp);
	file_size = dir_node->nn_inode.i_size;
	if (!file_size)
		return (0);

	DPRINTF(LOOKUP,
	    ("chg direntry dvp %p ino %#jx  to in %#jx at off %#jx\n",
	    dvp, (uintmax_t)tnode->nn_ino, (uintmax_t)fnode->nn_ino,
	    (uintmax_t)tnode->nn_diroff));

	blocksize = dir_node->nn_nandfsdev->nd_blocksize;
	blocknr = tnode->nn_diroff / blocksize;
	off = tnode->nn_diroff % blocksize;
	error = nandfs_bread(dir_node, blocknr, NOCRED, 0, &bp);
	if (error) {
		brelse(bp);
		return (error);
	}

	pos = bp->b_data;
	dirent = (struct nandfs_dir_entry *) (pos + off);
	KASSERT((dirent->inode == tnode->nn_ino),
	    ("direntry mismatch"));

	dirent->inode = fnode->nn_ino;
	error = nandfs_dirty_buf(bp, 0);
	if (error)
		return (error);

	return (0);
}

int
nandfs_init_dir(struct vnode *dvp, uint64_t ino, uint64_t parent_ino)
{

	if (nandfs_add_dirent(dvp, parent_ino, "..", 2, DT_DIR) ||
	    nandfs_add_dirent(dvp, ino, ".", 1, DT_DIR)) {
		nandfs_error("%s: cannot initialize dir ino:%jd(pino:%jd)\n",
		    __func__, ino, parent_ino);
		return (-1);
	}
	return (0);
}