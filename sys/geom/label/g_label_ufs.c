
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

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <geom/geom.h>
#include <geom/label/g_label.h>

#define G_LABEL_UFS_VOLUME_DIR	"ufs"
#define G_LABEL_UFS_ID_DIR	"ufsid"

#define	G_LABEL_UFS_VOLUME	0
#define	G_LABEL_UFS_ID		1

static const int superblocks[] = SBLOCKSEARCH;

static void
g_label_ufs_taste_common(struct g_consumer *cp, char *label, size_t size, int what)
{
	struct g_provider *pp;
	int sb, superblock;
	struct fs *fs;

	g_topology_assert_not();
	pp = cp->provider;
	label[0] = '\0';

	if (SBLOCKSIZE % cp->provider->sectorsize != 0)
		return;

	/*
	 * Walk through the standard places that superblocks hide and look
	 * for UFS magic. If we find magic, then check that the size in the
	 * superblock corresponds to the size of the underlying provider.
	 * Finally, look for a volume label and create an appropriate
	 * provider based on that.
	 */
	for (sb = 0; (superblock = superblocks[sb]) != -1; sb++) {
		/*
		 * Take care not to issue an invalid I/O request. The offset of
		 * the superblock candidate must be multiples of the provider's
		 * sector size, otherwise an FFS can't exist on the provider
		 * anyway.
		 */
		if (superblock % cp->provider->sectorsize != 0)
			continue;

		fs = (struct fs *)g_read_data(cp, superblock, SBLOCKSIZE, NULL);
		if (fs == NULL)
			continue;
		/* Check for magic. We also need to check if file system size is equal
		 * to providers size, because sysinstall(8) used to bogusly put first
		 * partition at offset 0 instead of 16, and glabel/ufs would find file
		 * system on slice instead of partition.
		 */
		if (fs->fs_magic == FS_UFS1_MAGIC && fs->fs_fsize > 0 &&
		    pp->mediasize / fs->fs_fsize == fs->fs_old_size) {
		    	/* Valid UFS1. */
		} else if (fs->fs_magic == FS_UFS2_MAGIC && fs->fs_fsize > 0 &&
		    ((pp->mediasize / fs->fs_fsize == fs->fs_size) ||
		    (pp->mediasize / fs->fs_fsize == fs->fs_providersize))) {
		    	/* Valid UFS2. */
		} else {
			g_free(fs);
			continue;
		}
		if (fs->fs_sblockloc != superblock || fs->fs_ncg < 1 ||
		    fs->fs_bsize < MINBSIZE ||
		    fs->fs_bsize < sizeof(struct fs)) {
			g_free(fs);
			continue;
		}
		G_LABEL_DEBUG(1, "%s file system detected on %s.",
		    fs->fs_magic == FS_UFS1_MAGIC ? "UFS1" : "UFS2", pp->name);
		switch (what) {
		case G_LABEL_UFS_VOLUME:
			/* Check for volume label */
			if (fs->fs_volname[0] == '\0') {
				g_free(fs);
				continue;
			}
			strlcpy(label, fs->fs_volname, size);
			break;
		case G_LABEL_UFS_ID:
			if (fs->fs_id[0] == 0 && fs->fs_id[1] == 0) {
				g_free(fs);
				continue;
			}
			snprintf(label, size, "%08x%08x", fs->fs_id[0],
			    fs->fs_id[1]);
			break;
		}
		g_free(fs);
		break;
	}
}

static void
g_label_ufs_volume_taste(struct g_consumer *cp, char *label, size_t size)
{

	g_label_ufs_taste_common(cp, label, size, G_LABEL_UFS_VOLUME);
}

static void
g_label_ufs_id_taste(struct g_consumer *cp, char *label, size_t size)
{

	g_label_ufs_taste_common(cp, label, size, G_LABEL_UFS_ID);
}

struct g_label_desc g_label_ufs_volume = {
	.ld_taste = g_label_ufs_volume_taste,
	.ld_dir = G_LABEL_UFS_VOLUME_DIR,
	.ld_enabled = 1
};

struct g_label_desc g_label_ufs_id = {
	.ld_taste = g_label_ufs_id_taste,
	.ld_dir = G_LABEL_UFS_ID_DIR,
	.ld_enabled = 1
};

G_LABEL_INIT(ufsid, g_label_ufs_id, "Create device nodes for UFS file system IDs");
G_LABEL_INIT(ufs, g_label_ufs_volume, "Create device nodes for UFS volume names");