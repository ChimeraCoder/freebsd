
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

#include "opt_quota.h"
#include "opt_ufs.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/lock.h>
#include <sys/mount.h>
#include <sys/malloc.h>
#include <sys/mutex.h>

#include <ufs/ufs/extattr.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/ufs_extern.h>
#ifdef UFS_DIRHASH
#include <ufs/ufs/dir.h>
#include <ufs/ufs/dirhash.h>
#endif
#ifdef UFS_GJOURNAL
#include <ufs/ufs/gjournal.h>
#endif

/*
 * Last reference to an inode.  If necessary, write or delete it.
 */
int
ufs_inactive(ap)
	struct vop_inactive_args /* {
		struct vnode *a_vp;
		struct thread *a_td;
	} */ *ap;
{
	struct vnode *vp = ap->a_vp;
	struct inode *ip = VTOI(vp);
	mode_t mode;
	int error = 0;
	off_t isize;
	struct mount *mp;

	mp = NULL;
	/*
	 * Ignore inodes related to stale file handles.
	 */
	if (ip->i_mode == 0)
		goto out;
#ifdef UFS_GJOURNAL
	ufs_gjournal_close(vp);
#endif
#ifdef QUOTA
	/*
	 * Before moving off the active list, we must be sure that
	 * any modified quotas have been pushed since these will no
	 * longer be checked once the vnode is on the inactive list.
	 */
	qsyncvp(vp);
#endif
	if ((ip->i_effnlink == 0 && DOINGSOFTDEP(vp)) ||
	    (ip->i_nlink <= 0 && !UFS_RDONLY(ip))) {
	loop:
		if (vn_start_secondary_write(vp, &mp, V_NOWAIT) != 0) {
			/* Cannot delete file while file system is suspended */
			if ((vp->v_iflag & VI_DOOMED) != 0) {
				/* Cannot return before file is deleted */
				(void) vn_start_secondary_write(vp, &mp,
								V_WAIT);
			} else {
				MNT_ILOCK(mp);
				if ((mp->mnt_kern_flag &
				     (MNTK_SUSPEND2 | MNTK_SUSPENDED)) == 0) {
					MNT_IUNLOCK(mp);
					goto loop;
				}
				/*
				 * Fail to inactivate vnode now and
				 * let ffs_snapshot() clean up after
				 * it has resumed the file system.
				 */
				VI_LOCK(vp);
				vp->v_iflag |= VI_OWEINACT;
				VI_UNLOCK(vp);
				MNT_IUNLOCK(mp);
				return (0);
			}
		}
	}
	isize = ip->i_size;
	if (ip->i_ump->um_fstype == UFS2)
		isize += ip->i_din2->di_extsize;
	if (ip->i_effnlink <= 0 && isize && !UFS_RDONLY(ip))
		error = UFS_TRUNCATE(vp, (off_t)0, IO_EXT | IO_NORMAL, NOCRED);
	if (ip->i_nlink <= 0 && ip->i_mode && !UFS_RDONLY(ip)) {
#ifdef QUOTA
		if (!getinoquota(ip))
			(void)chkiq(ip, -1, NOCRED, FORCE);
#endif
#ifdef UFS_EXTATTR
		ufs_extattr_vnode_inactive(vp, ap->a_td);
#endif
		/*
		 * Setting the mode to zero needs to wait for the inode
		 * to be written just as does a change to the link count.
		 * So, rather than creating a new entry point to do the
		 * same thing, we just use softdep_change_linkcnt().
		 */
		DIP_SET(ip, i_rdev, 0);
		mode = ip->i_mode;
		ip->i_mode = 0;
		DIP_SET(ip, i_mode, 0);
		ip->i_flag |= IN_CHANGE | IN_UPDATE;
		if (DOINGSOFTDEP(vp))
			softdep_change_linkcnt(ip);
		UFS_VFREE(vp, ip->i_number, mode);
	}
	if (ip->i_flag & (IN_ACCESS | IN_CHANGE | IN_MODIFIED | IN_UPDATE)) {
		if ((ip->i_flag & (IN_CHANGE | IN_UPDATE | IN_MODIFIED)) == 0 &&
		    mp == NULL &&
		    vn_start_secondary_write(vp, &mp, V_NOWAIT)) {
			mp = NULL;
			ip->i_flag &= ~IN_ACCESS;
		} else {
			if (mp == NULL)
				(void) vn_start_secondary_write(vp, &mp,
								V_WAIT);
			UFS_UPDATE(vp, 0);
		}
	}
out:
	/*
	 * If we are done with the inode, reclaim it
	 * so that it can be reused immediately.
	 */
	if (ip->i_mode == 0)
		vrecycle(vp);
	if (mp != NULL)
		vn_finished_secondary_write(mp);
	return (error);
}

void
ufs_prepare_reclaim(struct vnode *vp)
{
	struct inode *ip;
#ifdef QUOTA
	int i;
#endif

	ip = VTOI(vp);

	vnode_destroy_vobject(vp);
#ifdef QUOTA
	for (i = 0; i < MAXQUOTAS; i++) {
		if (ip->i_dquot[i] != NODQUOT) {
			dqrele(vp, ip->i_dquot[i]);
			ip->i_dquot[i] = NODQUOT;
		}
	}
#endif
#ifdef UFS_DIRHASH
	if (ip->i_dirhash != NULL)
		ufsdirhash_free(ip);
#endif
}

/*
 * Reclaim an inode so that it can be used for other purposes.
 */
int
ufs_reclaim(ap)
	struct vop_reclaim_args /* {
		struct vnode *a_vp;
		struct thread *a_td;
	} */ *ap;
{
	struct vnode *vp = ap->a_vp;
	struct inode *ip = VTOI(vp);
	struct ufsmount *ump = ip->i_ump;

	ufs_prepare_reclaim(vp);

	if (ip->i_flag & IN_LAZYMOD)
		ip->i_flag |= IN_MODIFIED;
	UFS_UPDATE(vp, 0);
	/*
	 * Remove the inode from its hash chain.
	 */
	vfs_hash_remove(vp);

	/*
	 * Lock the clearing of v_data so ffs_lock() can inspect it
	 * prior to obtaining the lock.
	 */
	VI_LOCK(vp);
	vp->v_data = 0;
	VI_UNLOCK(vp);
	UFS_IFREE(ump, ip);
	return (0);
}