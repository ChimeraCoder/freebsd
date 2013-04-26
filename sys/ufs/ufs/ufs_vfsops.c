
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
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/socket.h>
#include <sys/vnode.h>

#include <ufs/ufs/extattr.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/ufs_extern.h>
#ifdef UFS_DIRHASH
#include <ufs/ufs/dir.h>
#include <ufs/ufs/dirhash.h>
#endif

MALLOC_DEFINE(M_UFSMNT, "ufs_mount", "UFS mount structure");

/*
 * Return the root of a filesystem.
 */
int
ufs_root(mp, flags, vpp)
	struct mount *mp;
	int flags;
	struct vnode **vpp;
{
	struct vnode *nvp;
	int error;

	error = VFS_VGET(mp, (ino_t)ROOTINO, flags, &nvp);
	if (error)
		return (error);
	*vpp = nvp;
	return (0);
}

/*
 * Do operations associated with quotas
 */
int
ufs_quotactl(mp, cmds, id, arg)
	struct mount *mp;
	int cmds;
	uid_t id;
	void *arg;
{
#ifndef QUOTA
	return (EOPNOTSUPP);
#else
	struct thread *td;
	int cmd, type, error;

	td = curthread;
	cmd = cmds >> SUBCMDSHIFT;
	type = cmds & SUBCMDMASK;
	if (id == -1) {
		switch (type) {

		case USRQUOTA:
			id = td->td_ucred->cr_ruid;
			break;

		case GRPQUOTA:
			id = td->td_ucred->cr_rgid;
			break;

		default:
			return (EINVAL);
		}
	}
	if ((u_int)type >= MAXQUOTAS)
		return (EINVAL);

	switch (cmd) {
	case Q_QUOTAON:
		error = quotaon(td, mp, type, arg);
		break;

	case Q_QUOTAOFF:
		error = quotaoff(td, mp, type);
		break;

	case Q_SETQUOTA32:
		error = setquota32(td, mp, id, type, arg);
		break;

	case Q_SETUSE32:
		error = setuse32(td, mp, id, type, arg);
		break;

	case Q_GETQUOTA32:
		error = getquota32(td, mp, id, type, arg);
		break;

	case Q_SETQUOTA:
		error = setquota(td, mp, id, type, arg);
		break;

	case Q_SETUSE:
		error = setuse(td, mp, id, type, arg);
		break;

	case Q_GETQUOTA:
		error = getquota(td, mp, id, type, arg);
		break;

	case Q_GETQUOTASIZE:
		error = getquotasize(td, mp, id, type, arg);
		break;

	case Q_SYNC:
		error = qsync(mp);
		break;

	default:
		error = EINVAL;
		break;
	}
	return (error);
#endif
}

/*
 * Initial UFS filesystems, done only once.
 */
int
ufs_init(vfsp)
	struct vfsconf *vfsp;
{

#ifdef QUOTA
	dqinit();
#endif
#ifdef UFS_DIRHASH
	ufsdirhash_init();
#endif
	return (0);
}

/*
 * Uninitialise UFS filesystems, done before module unload.
 */
int
ufs_uninit(vfsp)
	struct vfsconf *vfsp;
{

#ifdef QUOTA
	dquninit();
#endif
#ifdef UFS_DIRHASH
	ufsdirhash_uninit();
#endif
	return (0);
}

/*
 * This is the generic part of fhtovp called after the underlying
 * filesystem has validated the file handle.
 *
 * Call the VFS_CHECKEXP beforehand to verify access.
 */
int
ufs_fhtovp(mp, ufhp, flags, vpp)
	struct mount *mp;
	struct ufid *ufhp;
	int flags;
	struct vnode **vpp;
{
	struct inode *ip;
	struct vnode *nvp;
	int error;

	error = VFS_VGET(mp, ufhp->ufid_ino, flags, &nvp);
	if (error) {
		*vpp = NULLVP;
		return (error);
	}
	ip = VTOI(nvp);
	if (ip->i_mode == 0 || ip->i_gen != ufhp->ufid_gen ||
	    ip->i_effnlink <= 0) {
		vput(nvp);
		*vpp = NULLVP;
		return (ESTALE);
	}
	*vpp = nvp;
	vnode_create_vobject(*vpp, DIP(ip, i_size), curthread);
	return (0);
}