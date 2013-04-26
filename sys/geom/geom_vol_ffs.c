
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
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/sysctl.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/bio.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <geom/geom.h>
#include <geom/geom_slice.h>

FEATURE(geom_vol, "GEOM support for volume names from UFS superblock");

#define VOL_FFS_CLASS_NAME "VOL_FFS"

static int superblocks[] = SBLOCKSEARCH;

struct g_vol_ffs_softc {
	char *	vol;
};

static int
g_vol_ffs_start(struct bio *bp __unused)
{
	return(0);
}

static struct g_geom *
g_vol_ffs_taste(struct g_class *mp, struct g_provider *pp, int flags)
{
	struct g_geom *gp;
	struct g_consumer *cp;
	struct g_vol_ffs_softc *ms;
	int sb, superblock;
	struct fs *fs;

	g_trace(G_T_TOPOLOGY, "vol_taste(%s,%s)", mp->name, pp->name);
	g_topology_assert();

	/* 
	 * XXX This is a really weak way to make sure we don't recurse.
	 * Probably ought to use BIO_GETATTR to check for this.
	 */
	if (flags == G_TF_NORMAL &&
	    !strcmp(pp->geom->class->name, VOL_FFS_CLASS_NAME))
		return (NULL);

	gp = g_slice_new(mp, 1, pp, &cp, &ms, sizeof(*ms), g_vol_ffs_start);
	if (gp == NULL)
		return (NULL);
	g_topology_unlock();
	/*
	 * Walk through the standard places that superblocks hide and look
	 * for UFS magic. If we find magic, then check that the size in the
	 * superblock corresponds to the size of the underlying provider.
	 * Finally, look for a volume label and create an appropriate 
	 * provider based on that.
	 */
	for (sb=0; (superblock = superblocks[sb]) != -1; sb++) {
		/*
		 * Take care not to issue an invalid I/O request.  The
		 * offset and size of the superblock candidate must be
		 * multiples of the provider's sector size, otherwise an
		 * FFS can't exist on the provider anyway.
		 */
		if (superblock % cp->provider->sectorsize != 0 ||
		    SBLOCKSIZE % cp->provider->sectorsize != 0)
			continue;

		fs = (struct fs *) g_read_data(cp, superblock,
			SBLOCKSIZE, NULL);
		if (fs == NULL)
			continue;
		/* Check for magic and make sure things are the right size */
		if (fs->fs_magic == FS_UFS1_MAGIC) {
			if (fs->fs_old_size * fs->fs_fsize !=
			    (int32_t) pp->mediasize) {
				g_free(fs);
				continue;
			}
		} else if (fs->fs_magic == FS_UFS2_MAGIC) {
			if (fs->fs_size * fs->fs_fsize !=
			    (int64_t) pp->mediasize) {
				g_free(fs);
				continue;
			}
		} else {
			g_free(fs);
			continue;
		}
		/* Check for volume label */
		if (fs->fs_volname[0] == '\0') {
			g_free(fs);
			continue;
		}
		/* XXX We need to check for namespace conflicts. */
		/* XXX How do you handle a mirror set? */
		/* XXX We don't validate the volume name. */
		g_topology_lock();
		/* Alright, we have a label and a volume name, reconfig. */
		g_slice_config(gp, 0, G_SLICE_CONFIG_SET, (off_t) 0,
		    pp->mediasize, pp->sectorsize, "vol/%s",
		    fs->fs_volname);
		g_free(fs);
		g_topology_unlock();
		break;
	}
	g_topology_lock();
	g_access(cp, -1, 0, 0);
	if (LIST_EMPTY(&gp->provider)) {
		g_slice_spoiled(cp);
		return (NULL);
	}
	return (gp);
}

static struct g_class g_vol_ffs_class	= {
	.name = VOL_FFS_CLASS_NAME,
	.version = G_VERSION,
	.taste = g_vol_ffs_taste,
};

DECLARE_GEOM_CLASS(g_vol_ffs_class, g_vol_ffs);