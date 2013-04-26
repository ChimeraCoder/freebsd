
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
#include <sys/mount.h>
#include <sys/disklabel.h>
#include <sys/stat.h>

#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libufs.h>

int
getino(struct uufsd *disk, void **dino, ino_t inode, int *mode)
{
	ino_t min, max;
	caddr_t inoblock;
	struct ufs1_dinode *dp1;
	struct ufs2_dinode *dp2;
	struct fs *fs;

	ERROR(disk, NULL);

	fs = &disk->d_fs;
	inoblock = disk->d_inoblock;
	min = disk->d_inomin;
	max = disk->d_inomax;

	if (inoblock == NULL) {
		inoblock = malloc(fs->fs_bsize);
		if (inoblock == NULL) {
			ERROR(disk, "unable to allocate inode block");
			return (-1);
		}
		disk->d_inoblock = inoblock;
	}
	if (inode >= min && inode < max)
		goto gotit;
	bread(disk, fsbtodb(fs, ino_to_fsba(fs, inode)), inoblock,
	    fs->fs_bsize);
	disk->d_inomin = min = inode - (inode % INOPB(fs));
	disk->d_inomax = max = min + INOPB(fs);
gotit:	switch (disk->d_ufs) {
	case 1:
		dp1 = &((struct ufs1_dinode *)inoblock)[inode - min];
		*mode = dp1->di_mode & IFMT;
		*dino = dp1;
		return (0);
	case 2:
		dp2 = &((struct ufs2_dinode *)inoblock)[inode - min];
		*mode = dp2->di_mode & IFMT;
		*dino = dp2;
		return (0);
	default:
		break;
	}
	ERROR(disk, "unknown UFS filesystem type");
	return (-1);
}

int
putino(struct uufsd *disk)
{
	struct fs *fs;

	fs = &disk->d_fs;
	if (disk->d_inoblock == NULL) {
		ERROR(disk, "No inode block allocated");
		return (-1);
	}
	if (bwrite(disk, fsbtodb(fs, ino_to_fsba(&disk->d_fs, disk->d_inomin)),
	    disk->d_inoblock, disk->d_fs.fs_bsize) <= 0)
		return (-1);
	return (0);
}