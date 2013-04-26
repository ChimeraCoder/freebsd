
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
#include <fstab.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libufs.h>

/* Internally, track the 'name' value, it's ours. */
#define	MINE_NAME	0x01
/* Track if its fd points to a writable device. */
#define	MINE_WRITE	0x02

int
ufs_disk_close(struct uufsd *disk)
{
	ERROR(disk, NULL);
	close(disk->d_fd);
	if (disk->d_inoblock != NULL) {
		free(disk->d_inoblock);
		disk->d_inoblock = NULL;
	}
	if (disk->d_mine & MINE_NAME) {
		free((char *)(uintptr_t)disk->d_name);
		disk->d_name = NULL;
	}
	if (disk->d_sbcsum != NULL) {
		free(disk->d_sbcsum);
		disk->d_sbcsum = NULL;
	}
	return (0);
}

int
ufs_disk_fillout(struct uufsd *disk, const char *name)
{
	if (ufs_disk_fillout_blank(disk, name) == -1) {
		return (-1);
	}
	if (sbread(disk) == -1) {
		ERROR(disk, "could not read superblock to fill out disk");
		return (-1);
	}
	return (0);
}

int
ufs_disk_fillout_blank(struct uufsd *disk, const char *name)
{
	struct stat st;
	struct fstab *fs;
	struct statfs sfs;
	const char *oname;
	char dev[MAXPATHLEN];
	int fd, ret;

	ERROR(disk, NULL);

	oname = name;
again:	if ((ret = stat(name, &st)) < 0) {
		if (*name != '/') {
			snprintf(dev, sizeof(dev), "%s%s", _PATH_DEV, name);
			name = dev;
			goto again;
		}
		/*
		 * The given object doesn't exist, but don't panic just yet -
		 * it may be still mount point listed in /etc/fstab, but without
		 * existing corresponding directory.
		 */
		name = oname;
	}
	if (ret >= 0 && S_ISREG(st.st_mode)) {
		/* Possibly a disk image, give it a try.  */
		;
	} else if (ret >= 0 && S_ISCHR(st.st_mode)) {
		/* This is what we need, do nothing. */
		;
	} else if ((fs = getfsfile(name)) != NULL) {
		/*
		 * The given mount point is listed in /etc/fstab.
		 * It is possible that someone unmounted file system by hand
		 * and different file system is mounted on this mount point,
		 * but we still prefer /etc/fstab entry, because on the other
		 * hand, there could be /etc/fstab entry for this mount
		 * point, but file system is not mounted yet (eg. noauto) and
		 * statfs(2) will point us at different file system.
		 */
		name = fs->fs_spec;
	} else if (ret >= 0 && S_ISDIR(st.st_mode)) {
		/*
		 * The mount point is not listed in /etc/fstab, so it may be
		 * file system mounted by hand.
		 */
		if (statfs(name, &sfs) < 0) {
			ERROR(disk, "could not find special device");
			return (-1);
		}
		strlcpy(dev, sfs.f_mntfromname, sizeof(dev));
		name = dev;
	} else {
		ERROR(disk, "could not find special device");
		return (-1);
	}
	fd = open(name, O_RDONLY);
	if (fd == -1) {
		ERROR(disk, "could not open special device");
		return (-1);
	}

	disk->d_bsize = 1;
	disk->d_ccg = 0;
	disk->d_fd = fd;
	disk->d_inoblock = NULL;
	disk->d_inomin = 0;
	disk->d_inomax = 0;
	disk->d_lcg = 0;
	disk->d_mine = 0;
	disk->d_ufs = 0;
	disk->d_error = NULL;
	disk->d_sbcsum = NULL;

	if (oname != name) {
		name = strdup(name);
		if (name == NULL) {
			ERROR(disk, "could not allocate memory for disk name");
			return (-1);
		}
		disk->d_mine |= MINE_NAME;
	}
	disk->d_name = name;

	return (0);
}

int
ufs_disk_write(struct uufsd *disk)
{
	ERROR(disk, NULL);

	if (disk->d_mine & MINE_WRITE)
		return (0);

	close(disk->d_fd);

	disk->d_fd = open(disk->d_name, O_RDWR);
	if (disk->d_fd < 0) {
		ERROR(disk, "failed to open disk for writing");
		return (-1);
	}

	disk->d_mine |= MINE_WRITE;

	return (0);
}