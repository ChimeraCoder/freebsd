
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
#include <sys/disklabel.h>
#include <sys/mount.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <libufs.h>
#include <libgeom.h>
#include <core/geom.h>
#include <misc/subr.h>

#include "geom_journal.h"

static struct fs *
read_superblock(const char *prov)
{
	static struct uufsd disk;
	struct fs *fs;

	if (ufs_disk_fillout(&disk, prov) == -1)
		return (NULL);
	fs = &disk.d_fs;
	ufs_disk_close(&disk);
	return (fs);
}

int
g_journal_ufs_exists(const char *prov)
{

	return (read_superblock(prov) != NULL);
}

int
g_journal_ufs_using_last_sector(const char *prov)
{
	struct fs *fs;
	off_t psize, fssize;

	fs = read_superblock(prov);
	if (fs == NULL)
		return (0);
	/* Provider size in 512 bytes blocks. */
	psize = g_get_mediasize(prov) / DEV_BSIZE;
	/* File system size in 512 bytes blocks. */
	fssize = fsbtodb(fs, fs->fs_size);
	return (psize <= fssize);
}