
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
#include <sys/pathname.h>
#include <sys/vfs.h>
#include <sys/vnode.h>

int
lookupname(char *dirname, enum uio_seg seg, enum symfollow follow,
    vnode_t **dirvpp, vnode_t **compvpp)
{

	return (lookupnameat(dirname, seg, follow, dirvpp, compvpp, NULL));
}

int
lookupnameat(char *dirname, enum uio_seg seg, enum symfollow follow,
    vnode_t **dirvpp, vnode_t **compvpp, vnode_t *startvp)
{
	struct nameidata nd;
	int error, ltype;

	ASSERT(dirvpp == NULL);

	vref(startvp);
	ltype = VOP_ISLOCKED(startvp);
	VOP_UNLOCK(startvp, 0);
	NDINIT_ATVP(&nd, LOOKUP, LOCKLEAF | follow, seg, dirname,
	    startvp, curthread);
	error = namei(&nd);
	*compvpp = nd.ni_vp;
	NDFREE(&nd, NDF_ONLY_PNBUF);
	vn_lock(startvp, ltype | LK_RETRY);
	return (error);
}

int
traverse(vnode_t **cvpp, int lktype)
{
	vnode_t *cvp;
	vnode_t *tvp;
	vfs_t *vfsp;
	int error;

	cvp = *cvpp;
	tvp = NULL;

	/*
	 * If this vnode is mounted on, then we transparently indirect
	 * to the vnode which is the root of the mounted file system.
	 * Before we do this we must check that an unmount is not in
	 * progress on this vnode.
	 */

	for (;;) {
		/*
		 * Reached the end of the mount chain?
		 */
		vfsp = vn_mountedvfs(cvp);
		if (vfsp == NULL)
			break;
		error = vfs_busy(vfsp, 0);
		/*
		 * tvp is NULL for *cvpp vnode, which we can't unlock.
		 */
		if (tvp != NULL)
			vput(cvp);
		else
			vrele(cvp);
		if (error)
			return (error);

		/*
		 * The read lock must be held across the call to VFS_ROOT() to
		 * prevent a concurrent unmount from destroying the vfs.
		 */
		error = VFS_ROOT(vfsp, lktype, &tvp);
		vfs_unbusy(vfsp);
		if (error != 0)
			return (error);
		cvp = tvp;
	}

	*cvpp = cvp;
	return (0);
}