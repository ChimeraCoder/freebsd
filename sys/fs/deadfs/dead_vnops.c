
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/poll.h>
#include <sys/vnode.h>

/*
 * Prototypes for dead operations on vnodes.
 */
static vop_lookup_t	dead_lookup;
static vop_open_t	dead_open;
static vop_poll_t	dead_poll;
static vop_read_t	dead_read;
static vop_write_t	dead_write;
static vop_getwritemount_t dead_getwritemount;
static vop_rename_t	dead_rename;

struct vop_vector dead_vnodeops = {
	.vop_default =		&default_vnodeops,

	.vop_access =		VOP_EBADF,
	.vop_advlock =		VOP_EBADF,
	.vop_bmap =		VOP_EBADF,
	.vop_create =		VOP_PANIC,
	.vop_getattr =		VOP_EBADF,
	.vop_getwritemount =	dead_getwritemount,
	.vop_inactive =		VOP_NULL,
	.vop_ioctl =		VOP_EBADF,
	.vop_link =		VOP_PANIC,
	.vop_lookup =		dead_lookup,
	.vop_mkdir =		VOP_PANIC,
	.vop_mknod =		VOP_PANIC,
	.vop_open =		dead_open,
	.vop_pathconf =		VOP_EBADF,	/* per pathconf(2) */
	.vop_poll =		dead_poll,
	.vop_read =		dead_read,
	.vop_readdir =		VOP_EBADF,
	.vop_readlink =		VOP_EBADF,
	.vop_reclaim =		VOP_NULL,
	.vop_remove =		VOP_PANIC,
	.vop_rename =		dead_rename,
	.vop_rmdir =		VOP_PANIC,
	.vop_setattr =		VOP_EBADF,
	.vop_symlink =		VOP_PANIC,
	.vop_vptocnp =		VOP_EBADF,
	.vop_write =		dead_write,
};

/* ARGSUSED */
static int
dead_getwritemount(ap)
	struct vop_getwritemount_args /* {
		struct vnode *a_vp;
		struct mount **a_mpp;
	} */ *ap;
{
	*(ap->a_mpp) = NULL;
	return (0);
}

/*
 * Trivial lookup routine that always fails.
 */
/* ARGSUSED */
static int
dead_lookup(ap)
	struct vop_lookup_args /* {
		struct vnode * a_dvp;
		struct vnode ** a_vpp;
		struct componentname * a_cnp;
	} */ *ap;
{

	*ap->a_vpp = NULL;
	return (ENOTDIR);
}

/*
 * Open always fails as if device did not exist.
 */
/* ARGSUSED */
static int
dead_open(ap)
	struct vop_open_args /* {
		struct vnode *a_vp;
		int  a_mode;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap;
{

	return (ENXIO);
}

/*
 * Vnode op for read
 */
/* ARGSUSED */
static int
dead_read(ap)
	struct vop_read_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int  a_ioflag;
		struct ucred *a_cred;
	} */ *ap;
{
	/*
	 * Return EOF for tty devices, EIO for others
	 */
	if ((ap->a_vp->v_vflag & VV_ISTTY) == 0)
		return (EIO);
	return (0);
}

/*
 * Vnode op for write
 */
/* ARGSUSED */
static int
dead_write(ap)
	struct vop_write_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int  a_ioflag;
		struct ucred *a_cred;
	} */ *ap;
{
	return (EIO);
}

/*
 * Trivial poll routine that always returns POLLHUP.
 * This is necessary so that a process which is polling a file
 * gets notified when that file is revoke()d.
 */
static int
dead_poll(ap)
	struct vop_poll_args *ap;
{
	return (POLLHUP);
}

static int
dead_rename(ap)
	struct vop_rename_args  /* {
		struct vnode *a_fdvp;
		struct vnode *a_fvp;
		struct componentname *a_fcnp;
		struct vnode *a_tdvp;
		struct vnode *a_tvp;
		struct componentname *a_tcnp;
	} */ *ap;
{

	vop_rename_fail(ap);
	return (EXDEV);
}