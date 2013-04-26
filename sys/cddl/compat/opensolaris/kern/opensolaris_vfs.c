
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
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/cred.h>
#include <sys/vfs.h>
#include <sys/priv.h>
#include <sys/libkern.h>

MALLOC_DECLARE(M_MOUNT);

void
vfs_setmntopt(vfs_t *vfsp, const char *name, const char *arg,
    int flags __unused)
{
	struct vfsopt *opt;
	size_t namesize;
	int locked;

	if (!(locked = mtx_owned(MNT_MTX(vfsp))))
		MNT_ILOCK(vfsp);

	if (vfsp->mnt_opt == NULL) {
		void *opts;

		MNT_IUNLOCK(vfsp);
		opts = malloc(sizeof(*vfsp->mnt_opt), M_MOUNT, M_WAITOK);
		MNT_ILOCK(vfsp);
		if (vfsp->mnt_opt == NULL) {
			vfsp->mnt_opt = opts;
			TAILQ_INIT(vfsp->mnt_opt);
		} else {
			free(opts, M_MOUNT);
		}
	}

	MNT_IUNLOCK(vfsp);

	opt = malloc(sizeof(*opt), M_MOUNT, M_WAITOK);
	namesize = strlen(name) + 1;
	opt->name = malloc(namesize, M_MOUNT, M_WAITOK);
	strlcpy(opt->name, name, namesize);
	opt->pos = -1;
	opt->seen = 1;
	if (arg == NULL) {
		opt->value = NULL;
		opt->len = 0;
	} else {
		opt->len = strlen(arg) + 1;
		opt->value = malloc(opt->len, M_MOUNT, M_WAITOK);
		bcopy(arg, opt->value, opt->len);
	}

	MNT_ILOCK(vfsp);
	TAILQ_INSERT_TAIL(vfsp->mnt_opt, opt, link);
	if (!locked)
		MNT_IUNLOCK(vfsp);
}

void
vfs_clearmntopt(vfs_t *vfsp, const char *name)
{
	int locked;

	if (!(locked = mtx_owned(MNT_MTX(vfsp))))
		MNT_ILOCK(vfsp);
	vfs_deleteopt(vfsp->mnt_opt, name);
	if (!locked)
		MNT_IUNLOCK(vfsp);
}

int
vfs_optionisset(const vfs_t *vfsp, const char *opt, char **argp)
{
	struct vfsoptlist *opts = vfsp->mnt_optnew;
	int error;

	if (opts == NULL)
		return (0);
	error = vfs_getopt(opts, opt, (void **)argp, NULL);
	return (error != 0 ? 0 : 1);
}

int
mount_snapshot(kthread_t *td, vnode_t **vpp, const char *fstype, char *fspath,
    char *fspec, int fsflags)
{
	struct vfsconf *vfsp;
	struct mount *mp;
	vnode_t *vp, *mvp;
	struct ucred *cr;
	int error;

	/*
	 * Be ultra-paranoid about making sure the type and fspath
	 * variables will fit in our mp buffers, including the
	 * terminating NUL.
	 */
	if (strlen(fstype) >= MFSNAMELEN || strlen(fspath) >= MNAMELEN)
		return (ENAMETOOLONG);

	vfsp = vfs_byname_kld(fstype, td, &error);
	if (vfsp == NULL)
		return (ENODEV);

	vp = *vpp;
	if (vp->v_type != VDIR)
		return (ENOTDIR);
	/*
	 * We need vnode lock to protect v_mountedhere and vnode interlock
	 * to protect v_iflag.
	 */
	vn_lock(vp, LK_SHARED | LK_RETRY);
	VI_LOCK(vp);
	if ((vp->v_iflag & VI_MOUNT) != 0 || vp->v_mountedhere != NULL) {
		VI_UNLOCK(vp);
		VOP_UNLOCK(vp, 0);
		return (EBUSY);
	}
	vp->v_iflag |= VI_MOUNT;
	VI_UNLOCK(vp);
	VOP_UNLOCK(vp, 0);

	/*
	 * Allocate and initialize the filesystem.
	 * We don't want regular user that triggered snapshot mount to be able
	 * to unmount it, so pass credentials of the parent mount.
	 */
	mp = vfs_mount_alloc(vp, vfsp, fspath, vp->v_mount->mnt_cred);

	mp->mnt_optnew = NULL;
	vfs_setmntopt(mp, "from", fspec, 0);
	mp->mnt_optnew = mp->mnt_opt;
	mp->mnt_opt = NULL;

	/*
	 * Set the mount level flags.
	 */
	mp->mnt_flag = fsflags & MNT_UPDATEMASK;
	/*
	 * Snapshots are always read-only.
	 */
	mp->mnt_flag |= MNT_RDONLY;
	/*
	 * We don't want snapshots to allow access to vulnerable setuid
	 * programs, so we turn off setuid when mounting snapshots.
	 */
	mp->mnt_flag |= MNT_NOSUID;
	/*
	 * We don't want snapshots to be visible in regular
	 * mount(8) and df(1) output.
	 */
	mp->mnt_flag |= MNT_IGNORE;
	/*
	 * XXX: This is evil, but we can't mount a snapshot as a regular user.
	 * XXX: Is is safe when snapshot is mounted from within a jail?
	 */
	cr = td->td_ucred;
	td->td_ucred = kcred;
	error = VFS_MOUNT(mp);
	td->td_ucred = cr;

	if (error != 0) {
		VI_LOCK(vp);
		vp->v_iflag &= ~VI_MOUNT;
		VI_UNLOCK(vp);
		vrele(vp);
		vfs_unbusy(mp);
		vfs_mount_destroy(mp);
		*vpp = NULL;
		return (error);
	}

	if (mp->mnt_opt != NULL)
		vfs_freeopts(mp->mnt_opt);
	mp->mnt_opt = mp->mnt_optnew;
	(void)VFS_STATFS(mp, &mp->mnt_stat);

	/*
	 * Prevent external consumers of mount options from reading
	 * mnt_optnew.
	*/
	mp->mnt_optnew = NULL;

	vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
#ifdef FREEBSD_NAMECACHE
	cache_purge(vp);
#endif
	VI_LOCK(vp);
	vp->v_iflag &= ~VI_MOUNT;
	VI_UNLOCK(vp);

	vp->v_mountedhere = mp;
	/* Put the new filesystem on the mount list. */
	mtx_lock(&mountlist_mtx);
	TAILQ_INSERT_TAIL(&mountlist, mp, mnt_list);
	mtx_unlock(&mountlist_mtx);
	vfs_event_signal(NULL, VQ_MOUNT, 0);
	if (VFS_ROOT(mp, LK_EXCLUSIVE, &mvp))
		panic("mount: lost mount");
	vput(vp);
	vfs_unbusy(mp);
	*vpp = mvp;
	return (0);
}