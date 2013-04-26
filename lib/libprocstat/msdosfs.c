
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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/vnode.h>

#include <netinet/in.h>

#define	_KERNEL
#include <sys/mount.h>
#include <fs/msdosfs/bpb.h>
#include <fs/msdosfs/msdosfsmount.h>
#undef _KERNEL

#include <fs/msdosfs/denode.h>
#include <fs/msdosfs/direntry.h>
#include <fs/msdosfs/fat.h>

#include <err.h>
#include <kvm.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * XXX -
 * VTODE is defined in denode.h only if _KERNEL is defined, but that leads to
 * header explosion
 */
#define VTODE(vp) ((struct denode *)getvnodedata(vp))

#include "libprocstat.h"
#include "common_kvm.h"

struct dosmount {
	struct dosmount *next;
	struct msdosfsmount *kptr;	/* Pointer in kernel space */
	struct msdosfsmount data;	/* User space copy of structure */
};

int
msdosfs_filestat(kvm_t *kd, struct vnode *vp, struct vnstat *vn)
{
	struct denode denode;
	static struct dosmount *mounts;
	struct dosmount *mnt;
	u_long dirsperblk;
	int fileid;

	if (!kvm_read_all(kd, (unsigned long)VTODE(vp), &denode,
	    sizeof(denode))) {
		warnx("can't read denode at %p", (void *)VTODE(vp));
		return (1);
	}

	/*
	 * Find msdosfsmount structure for the vnode's filesystem. Needed
	 * for some filesystem parameters
	 */
	for (mnt = mounts; mnt; mnt = mnt->next)
		if (mnt->kptr == denode.de_pmp)
			break;

	if (!mnt) {
		if ((mnt = malloc(sizeof(struct dosmount))) == NULL) {
			warn("malloc()");
			return (1);
		}
		if (!kvm_read_all(kd, (unsigned long)denode.de_pmp,
		    &mnt->data, sizeof(mnt->data))) {
			free(mnt);
			    warnx("can't read mount info at %p",
			    (void *)denode.de_pmp);
			return (1);
		}
		mnt->next = mounts;
		mounts = mnt;
		mnt->kptr = denode.de_pmp;
	}

	vn->vn_fsid = dev2udev(kd, mnt->data.pm_dev);
	vn->vn_mode = 0555;
	vn->vn_mode |= denode.de_Attributes & ATTR_READONLY ? 0 : 0222;
	vn->vn_mode &= mnt->data.pm_mask;

	/* Distinguish directories and files. No "special" files in FAT. */
	vn->vn_mode |= denode.de_Attributes & ATTR_DIRECTORY ? S_IFDIR : S_IFREG;
	vn->vn_size = denode.de_FileSize;

	/*
	 * XXX -
	 * Culled from msdosfs_vnops.c. There appears to be a problem
	 * here, in that a directory has the same inode number as the first
	 * file in the directory. stat(2) suffers from this problem also, so
	 * I won't try to fix it here.
	 * 
	 * The following computation of the fileid must be the same as that
	 * used in msdosfs_readdir() to compute d_fileno. If not, pwd
	 * doesn't work.
	 */
	dirsperblk = mnt->data.pm_BytesPerSec / sizeof(struct direntry);
	if (denode.de_Attributes & ATTR_DIRECTORY) {
		fileid = cntobn(&mnt->data, denode.de_StartCluster)
		    * dirsperblk;
		if (denode.de_StartCluster == MSDOSFSROOT)
			fileid = 1;
	} else {
		fileid = cntobn(&mnt->data, denode.de_dirclust) * dirsperblk;
		if (denode.de_dirclust == MSDOSFSROOT)
			fileid = roottobn(&mnt->data, 0) * dirsperblk;
		fileid += denode.de_diroffset / sizeof(struct direntry);
	}

	vn->vn_fileid = fileid;
	return (0);
}