
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
#include <sys/vnode.h>
#include <sys/mount.h>

#include <ufs/ufs/extattr.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufs_extern.h>
#include <ufs/ufs/ufsmount.h>

#include <ufs/ffs/fs.h>
#include <ufs/ffs/ffs_extern.h>

#include <geom/geom.h>
#include <geom/journal/g_journal.h>

static const int superblocks[] = SBLOCKSEARCH;

static int
g_journal_ufs_clean(struct mount *mp)
{
	struct ufsmount *ump;
	struct fs *fs;
	int flags;

	ump = VFSTOUFS(mp);
	fs = ump->um_fs;

	flags = fs->fs_flags;
	fs->fs_flags &= ~(FS_UNCLEAN | FS_NEEDSFSCK);
	ffs_sbupdate(ump, MNT_WAIT, 1);
	fs->fs_flags = flags;

	return (0);
}

static void
g_journal_ufs_dirty(struct g_consumer *cp)
{
	struct fs *fs;
	int error, i, sb;

	if (SBLOCKSIZE % cp->provider->sectorsize != 0)
		return;
	for (i = 0; (sb = superblocks[i]) != -1; i++) {
		if (sb % cp->provider->sectorsize != 0)
			continue;
		fs = g_read_data(cp, sb, SBLOCKSIZE, NULL);
		if (fs == NULL)
			continue;
		if (fs->fs_magic != FS_UFS1_MAGIC &&
		    fs->fs_magic != FS_UFS2_MAGIC) {
			g_free(fs);
			continue;
		}
		GJ_DEBUG(0, "clean=%d flags=0x%x", fs->fs_clean, fs->fs_flags);
		fs->fs_clean = 0;
		fs->fs_flags |= FS_NEEDSFSCK | FS_UNCLEAN;
		error = g_write_data(cp, sb, fs, SBLOCKSIZE);
		g_free(fs);
		if (error != 0) {
			GJ_DEBUG(0, "Cannot mark file system %s as dirty "
			    "(error=%d).", cp->provider->name, error);
		} else {
			GJ_DEBUG(0, "File system %s marked as dirty.",
			    cp->provider->name);
		}
	}
}

const struct g_journal_desc g_journal_ufs = {
	.jd_fstype = "ufs",
	.jd_clean = g_journal_ufs_clean,
	.jd_dirty = g_journal_ufs_dirty
};

MODULE_DEPEND(g_journal, ufs, 1, 1, 1);