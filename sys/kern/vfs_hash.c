
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
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/vnode.h>

static MALLOC_DEFINE(M_VFS_HASH, "vfs_hash", "VFS hash table");

static LIST_HEAD(vfs_hash_head, vnode)	*vfs_hash_tbl;
static LIST_HEAD(,vnode)		vfs_hash_side;
static u_long				vfs_hash_mask;
static struct mtx			vfs_hash_mtx;

static void
vfs_hashinit(void *dummy __unused)
{

	vfs_hash_tbl = hashinit(desiredvnodes, M_VFS_HASH, &vfs_hash_mask);
	mtx_init(&vfs_hash_mtx, "vfs hash", NULL, MTX_DEF);
	LIST_INIT(&vfs_hash_side);
}

/* Must be SI_ORDER_SECOND so desiredvnodes is available */
SYSINIT(vfs_hash, SI_SUB_VFS, SI_ORDER_SECOND, vfs_hashinit, NULL);

u_int
vfs_hash_index(struct vnode *vp)
{

	return (vp->v_hash + vp->v_mount->mnt_hashseed);
}

static struct vfs_hash_head *
vfs_hash_bucket(const struct mount *mp, u_int hash)
{

	return (&vfs_hash_tbl[(hash + mp->mnt_hashseed) & vfs_hash_mask]);
}

int
vfs_hash_get(const struct mount *mp, u_int hash, int flags, struct thread *td, struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp;
	int error;

	while (1) {
		mtx_lock(&vfs_hash_mtx);
		LIST_FOREACH(vp, vfs_hash_bucket(mp, hash), v_hashlist) {
			if (vp->v_hash != hash)
				continue;
			if (vp->v_mount != mp)
				continue;
			if (fn != NULL && fn(vp, arg))
				continue;
			VI_LOCK(vp);
			mtx_unlock(&vfs_hash_mtx);
			error = vget(vp, flags | LK_INTERLOCK, td);
			if (error == ENOENT && (flags & LK_NOWAIT) == 0)
				break;
			if (error)
				return (error);
			*vpp = vp;
			return (0);
		}
		if (vp == NULL) {
			mtx_unlock(&vfs_hash_mtx);
			*vpp = NULL;
			return (0);
		}
	}
}

void
vfs_hash_remove(struct vnode *vp)
{

	mtx_lock(&vfs_hash_mtx);
	LIST_REMOVE(vp, v_hashlist);
	mtx_unlock(&vfs_hash_mtx);
}

int
vfs_hash_insert(struct vnode *vp, u_int hash, int flags, struct thread *td, struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp2;
	int error;

	*vpp = NULL;
	while (1) {
		mtx_lock(&vfs_hash_mtx);
		LIST_FOREACH(vp2,
		    vfs_hash_bucket(vp->v_mount, hash), v_hashlist) {
			if (vp2->v_hash != hash)
				continue;
			if (vp2->v_mount != vp->v_mount)
				continue;
			if (fn != NULL && fn(vp2, arg))
				continue;
			VI_LOCK(vp2);
			mtx_unlock(&vfs_hash_mtx);
			error = vget(vp2, flags | LK_INTERLOCK, td);
			if (error == ENOENT && (flags & LK_NOWAIT) == 0)
				break;
			mtx_lock(&vfs_hash_mtx);
			LIST_INSERT_HEAD(&vfs_hash_side, vp, v_hashlist);
			mtx_unlock(&vfs_hash_mtx);
			vput(vp);
			if (!error)
				*vpp = vp2;
			return (error);
		}
		if (vp2 == NULL)
			break;
			
	}
	vp->v_hash = hash;
	LIST_INSERT_HEAD(vfs_hash_bucket(vp->v_mount, hash), vp, v_hashlist);
	mtx_unlock(&vfs_hash_mtx);
	return (0);
}

void
vfs_hash_rehash(struct vnode *vp, u_int hash)
{

	mtx_lock(&vfs_hash_mtx);
	LIST_REMOVE(vp, v_hashlist);
	LIST_INSERT_HEAD(vfs_hash_bucket(vp->v_mount, hash), vp, v_hashlist);
	vp->v_hash = hash;
	mtx_unlock(&vfs_hash_mtx);
}