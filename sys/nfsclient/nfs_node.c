
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
#include <sys/fcntl.h>
#include <sys/fnv_hash.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/taskqueue.h>
#include <sys/vnode.h>

#include <vm/uma.h>

#include <nfs/nfsproto.h>
#include <nfs/nfs_lock.h>
#include <nfsclient/nfs.h>
#include <nfsclient/nfsnode.h>
#include <nfsclient/nfsmount.h>

static uma_zone_t nfsnode_zone;

static void	nfs_freesillyrename(void *arg, __unused int pending);

#define TRUE	1
#define	FALSE	0

void
nfs_nhinit(void)
{

	nfsnode_zone = uma_zcreate("NFSNODE", sizeof(struct nfsnode), NULL,
	    NULL, NULL, NULL, UMA_ALIGN_PTR, 0);
}

void
nfs_nhuninit(void)
{
	uma_zdestroy(nfsnode_zone);
}

struct nfs_vncmp {
	int	fhsize;
	void	*fh;
};

static int
nfs_vncmpf(struct vnode *vp, void *arg)
{
	struct nfs_vncmp *a;
	struct nfsnode *np;

	a = arg;
	np = VTONFS(vp);
	return (bcmp(a->fh, np->n_fhp, a->fhsize));
}

/*
 * Look up a vnode/nfsnode by file handle.
 * Callers must check for mount points!!
 * In all cases, a pointer to a
 * nfsnode structure is returned.
 */
int
nfs_nget(struct mount *mntp, nfsfh_t *fhp, int fhsize, struct nfsnode **npp, int flags)
{
	struct thread *td = curthread;	/* XXX */
	struct nfsnode *np;
	struct vnode *vp;
	struct vnode *nvp;
	int error;
	u_int hash;
	struct nfsmount *nmp;
	struct nfs_vncmp ncmp;

	nmp = VFSTONFS(mntp);
	*npp = NULL;

	hash = fnv_32_buf(fhp->fh_bytes, fhsize, FNV1_32_INIT);
	ncmp.fhsize = fhsize;
	ncmp.fh = fhp;

	error = vfs_hash_get(mntp, hash, flags,
	    td, &nvp, nfs_vncmpf, &ncmp);
	if (error)
		return (error);
	if (nvp != NULL) {
		*npp = VTONFS(nvp);
		return (0);
	}
	np = uma_zalloc(nfsnode_zone, M_WAITOK | M_ZERO);

	error = getnewvnode("nfs", mntp, &nfs_vnodeops, &nvp);
	if (error) {
		uma_zfree(nfsnode_zone, np);
		return (error);
	}
	vp = nvp;
	vp->v_bufobj.bo_ops = &buf_ops_nfs;
	vp->v_data = np;
	np->n_vnode = vp;
	/* 
	 * Initialize the mutex even if the vnode is going to be a loser.
	 * This simplifies the logic in reclaim, which can then unconditionally
	 * destroy the mutex (in the case of the loser, or if hash_insert happened
	 * to return an error no special casing is needed).
	 */
	mtx_init(&np->n_mtx, "NFSnode lock", NULL, MTX_DEF);
	/*
	 * NFS supports recursive and shared locking.
	 */
	lockmgr(vp->v_vnlock, LK_EXCLUSIVE | LK_NOWITNESS, NULL);
	VN_LOCK_AREC(vp);
	VN_LOCK_ASHARE(vp);
	if (fhsize > NFS_SMALLFH) {
		np->n_fhp = malloc(fhsize, M_NFSBIGFH, M_WAITOK);
	} else
		np->n_fhp = &np->n_fh;
	bcopy((caddr_t)fhp, (caddr_t)np->n_fhp, fhsize);
	np->n_fhsize = fhsize;
	error = insmntque(vp, mntp);
	if (error != 0) {
		*npp = NULL;
		if (np->n_fhsize > NFS_SMALLFH) {
			free((caddr_t)np->n_fhp, M_NFSBIGFH);
		}
		mtx_destroy(&np->n_mtx);
		uma_zfree(nfsnode_zone, np);
		return (error);
	}
	error = vfs_hash_insert(vp, hash, flags, 
	    td, &nvp, nfs_vncmpf, &ncmp);
	if (error)
		return (error);
	if (nvp != NULL) {
		*npp = VTONFS(nvp);
		/* vfs_hash_insert() vput()'s the losing vnode */
		return (0);
	}
	*npp = np;

	return (0);
}

/*
 * Do the vrele(sp->s_dvp) as a separate task in order to avoid a
 * deadlock because of a LOR when vrele() locks the directory vnode.
 */
static void
nfs_freesillyrename(void *arg, __unused int pending)
{
	struct sillyrename *sp;

	sp = arg;
	vrele(sp->s_dvp);
	free(sp, M_NFSREQ);
}

int
nfs_inactive(struct vop_inactive_args *ap)
{
	struct nfsnode *np;
	struct sillyrename *sp;
	struct thread *td = curthread;	/* XXX */

	np = VTONFS(ap->a_vp);
	mtx_lock(&np->n_mtx);
	if (ap->a_vp->v_type != VDIR) {
		sp = np->n_sillyrename;
		np->n_sillyrename = NULL;
	} else
		sp = NULL;
	if (sp) {
		mtx_unlock(&np->n_mtx);
		(void)nfs_vinvalbuf(ap->a_vp, 0, td, 1);
		/*
		 * Remove the silly file that was rename'd earlier
		 */
		(sp->s_removeit)(sp);
		crfree(sp->s_cred);
		TASK_INIT(&sp->s_task, 0, nfs_freesillyrename, sp);
		taskqueue_enqueue(taskqueue_thread, &sp->s_task);
		mtx_lock(&np->n_mtx);
	}
	np->n_flag &= NMODIFIED;
	mtx_unlock(&np->n_mtx);
	return (0);
}

/*
 * Reclaim an nfsnode so that it can be used for other purposes.
 */
int
nfs_reclaim(struct vop_reclaim_args *ap)
{
	struct vnode *vp = ap->a_vp;
	struct nfsnode *np = VTONFS(vp);
	struct nfsdmap *dp, *dp2;

	/*
	 * If the NLM is running, give it a chance to abort pending
	 * locks.
	 */
	if (nfs_reclaim_p)
		nfs_reclaim_p(ap);

	/*
	 * Destroy the vm object and flush associated pages.
	 */
	vnode_destroy_vobject(vp);

	vfs_hash_remove(vp);

	/*
	 * Free up any directory cookie structures and
	 * large file handle structures that might be associated with
	 * this nfs node.
	 */
	if (vp->v_type == VDIR) {
		dp = LIST_FIRST(&np->n_cookies);
		while (dp) {
			dp2 = dp;
			dp = LIST_NEXT(dp, ndm_list);
			free((caddr_t)dp2, M_NFSDIROFF);
		}
	}
	if (np->n_writecred != NULL)
		crfree(np->n_writecred);
	if (np->n_fhsize > NFS_SMALLFH) {
		free((caddr_t)np->n_fhp, M_NFSBIGFH);
	}
	mtx_destroy(&np->n_mtx);
	uma_zfree(nfsnode_zone, vp->v_data);
	vp->v_data = NULL;
	return (0);
}