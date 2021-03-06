
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
#include <sys/bio.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/vnode.h>
#include <sys/mount.h>

#include <geom/geom.h>
#include <geom/geom_vfs.h>

/*
 * subroutines for use by filesystems.
 *
 * XXX: should maybe live somewhere else ?
 */
#include <sys/buf.h>

struct g_vfs_softc {
	struct mtx	 sc_mtx;
	struct bufobj	*sc_bo;
	int		 sc_active;
	int		 sc_orphaned;
};

static struct buf_ops __g_vfs_bufops = {
	.bop_name =	"GEOM_VFS",
	.bop_write =	bufwrite,
	.bop_strategy =	g_vfs_strategy,	
	.bop_sync =	bufsync,	
	.bop_bdflush =	bufbdflush
};

struct buf_ops *g_vfs_bufops = &__g_vfs_bufops;

static g_orphan_t g_vfs_orphan;

static struct g_class g_vfs_class = {
	.name =		"VFS",
	.version =	G_VERSION,
	.orphan =	g_vfs_orphan,
};

DECLARE_GEOM_CLASS(g_vfs_class, g_vfs);

static void
g_vfs_destroy(void *arg, int flags __unused)
{
	struct g_consumer *cp;

	g_topology_assert();
	cp = arg;
	if (cp->acr > 0 || cp->acw > 0 || cp->ace > 0)
		g_access(cp, -cp->acr, -cp->acw, -cp->ace);
	g_detach(cp);
	if (cp->geom->softc == NULL)
		g_wither_geom(cp->geom, ENXIO);
}

static void
g_vfs_done(struct bio *bip)
{
	struct g_consumer *cp;
	struct g_vfs_softc *sc;
	struct buf *bp;
	int destroy;
	struct mount *mp;
	struct vnode *vp;
	struct cdev *cdevp;

	/*
	 * Collect statistics on synchronous and asynchronous read
	 * and write counts for disks that have associated filesystems.
	 * Since this run by the g_up thread it is single threaded and
	 * we do not need to use atomic increments on the counters.
	 */
	bp = bip->bio_caller2;
	vp = bp->b_vp;
	if (vp == NULL) {
		mp = NULL;
	} else {
		/*
		 * If not a disk vnode, use its associated mount point
		 * otherwise use the mountpoint associated with the disk.
		 */
		VI_LOCK(vp);
		if (vp->v_type != VCHR ||
		    (cdevp = vp->v_rdev) == NULL ||
		    cdevp->si_devsw == NULL ||
		    (cdevp->si_devsw->d_flags & D_DISK) == 0)
			mp = vp->v_mount;
		else
			mp = cdevp->si_mountpt;
		VI_UNLOCK(vp);
	}
	if (mp != NULL) {
		if (bp->b_iocmd == BIO_WRITE) {
			if (LK_HOLDER(bp->b_lock.lk_lock) == LK_KERNPROC)
				mp->mnt_stat.f_asyncwrites++;
			else
				mp->mnt_stat.f_syncwrites++;
		} else {
			if (LK_HOLDER(bp->b_lock.lk_lock) == LK_KERNPROC)
				mp->mnt_stat.f_asyncreads++;
			else
				mp->mnt_stat.f_syncreads++;
		}
	}

	cp = bip->bio_from;
	sc = cp->geom->softc;
	if (bip->bio_error) {
		printf("g_vfs_done():");
		g_print_bio(bip);
		printf("error = %d\n", bip->bio_error);
	}
	bp->b_error = bip->bio_error;
	bp->b_ioflags = bip->bio_flags;
	if (bip->bio_error)
		bp->b_ioflags |= BIO_ERROR;
	bp->b_resid = bp->b_bcount - bip->bio_completed;
	g_destroy_bio(bip);

	mtx_lock(&sc->sc_mtx);
	destroy = ((--sc->sc_active) == 0 && sc->sc_orphaned);
	mtx_unlock(&sc->sc_mtx);
	if (destroy)
		g_post_event(g_vfs_destroy, cp, M_WAITOK, NULL);

	bufdone(bp);
}

void
g_vfs_strategy(struct bufobj *bo, struct buf *bp)
{
	struct g_vfs_softc *sc;
	struct g_consumer *cp;
	struct bio *bip;

	cp = bo->bo_private;
	sc = cp->geom->softc;

	/*
	 * If the provider has orphaned us, just return EXIO.
	 */
	mtx_lock(&sc->sc_mtx);
	if (sc->sc_orphaned) {
		mtx_unlock(&sc->sc_mtx);
		bp->b_error = ENXIO;
		bp->b_ioflags |= BIO_ERROR;
		bufdone(bp);
		return;
	}
	sc->sc_active++;
	mtx_unlock(&sc->sc_mtx);

	bip = g_alloc_bio();
	bip->bio_cmd = bp->b_iocmd;
	bip->bio_offset = bp->b_iooffset;
	bip->bio_length = bp->b_bcount;
	bdata2bio(bp, bip);
	if ((bp->b_flags & B_BARRIER) != 0) {
		bip->bio_flags |= BIO_ORDERED;
		bp->b_flags &= ~B_BARRIER;
	}
	bip->bio_done = g_vfs_done;
	bip->bio_caller2 = bp;
	g_io_request(bip, cp);
}

static void
g_vfs_orphan(struct g_consumer *cp)
{
	struct g_geom *gp;
	struct g_vfs_softc *sc;
	int destroy;

	g_topology_assert();

	gp = cp->geom;
	g_trace(G_T_TOPOLOGY, "g_vfs_orphan(%p(%s))", cp, gp->name);
	sc = gp->softc;
	if (sc == NULL)
		return;
	mtx_lock(&sc->sc_mtx);
	sc->sc_orphaned = 1;
	destroy = (sc->sc_active == 0);
	mtx_unlock(&sc->sc_mtx);
	if (destroy)
		g_vfs_destroy(cp, 0);

	/*
	 * Do not destroy the geom.  Filesystem will do that during unmount.
	 */
}

int
g_vfs_open(struct vnode *vp, struct g_consumer **cpp, const char *fsname, int wr)
{
	struct g_geom *gp;
	struct g_provider *pp;
	struct g_consumer *cp;
	struct g_vfs_softc *sc;
	struct bufobj *bo;
	int error;

	g_topology_assert();

	*cpp = NULL;
	bo = &vp->v_bufobj;
	if (bo->bo_private != vp)
		return (EBUSY);

	pp = g_dev_getprovider(vp->v_rdev);
	if (pp == NULL)
		return (ENOENT);
	gp = g_new_geomf(&g_vfs_class, "%s.%s", fsname, pp->name);
	sc = g_malloc(sizeof(*sc), M_WAITOK | M_ZERO);
	mtx_init(&sc->sc_mtx, "g_vfs", NULL, MTX_DEF);
	sc->sc_bo = bo;
	gp->softc = sc;
	cp = g_new_consumer(gp);
	g_attach(cp, pp);
	error = g_access(cp, 1, wr, wr);
	if (error) {
		g_wither_geom(gp, ENXIO);
		return (error);
	}
	vnode_create_vobject(vp, pp->mediasize, curthread);
	*cpp = cp;
	cp->private = vp;
	bo->bo_ops = g_vfs_bufops;
	bo->bo_private = cp;
	bo->bo_bsize = pp->sectorsize;

	return (error);
}

void
g_vfs_close(struct g_consumer *cp)
{
	struct g_geom *gp;
	struct g_vfs_softc *sc;

	g_topology_assert();

	gp = cp->geom;
	sc = gp->softc;
	bufobj_invalbuf(sc->sc_bo, V_SAVE, 0, 0);
	sc->sc_bo->bo_private = cp->private;
	gp->softc = NULL;
	mtx_destroy(&sc->sc_mtx);
	if (!sc->sc_orphaned || cp->provider == NULL)
		g_wither_geom_close(gp, ENXIO);
	g_free(sc);
}