
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
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <sys/conf.h>
#define	_KERNEL
#include <sys/pipe.h>
#include <sys/mount.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <fs/devfs/devfs.h>
#include <fs/devfs/devfs_int.h>
#undef _KERNEL
#include <nfs/nfsproto.h>
#include <nfsclient/nfs.h>
#include <nfsclient/nfsnode.h>

#include <assert.h>
#include <err.h>
#include <kvm.h>
#include <stddef.h>
#include <string.h>

#include <libprocstat.h>
#include "common_kvm.h"

int
kvm_read_all(kvm_t *kd, unsigned long addr, void *buf, size_t nbytes)
{
	ssize_t error;

	if (nbytes >= SSIZE_MAX)
		return (0);
	error = kvm_read(kd, addr, buf, nbytes);
	return (error == (ssize_t)(nbytes));
}

int
kdevtoname(kvm_t *kd, struct cdev *dev, char *buf)
{
	struct cdev si;

	assert(buf);
	if (!kvm_read_all(kd, (unsigned long)dev, &si, sizeof(si)))
		return (1);
	strlcpy(buf, si.si_name, SPECNAMELEN + 1);
	return (0);
}

int
ufs_filestat(kvm_t *kd, struct vnode *vp, struct vnstat *vn)
{
	struct inode inode;

	if (!kvm_read_all(kd, (unsigned long)VTOI(vp), &inode, sizeof(inode))) {
		warnx("can't read inode at %p", (void *)VTOI(vp));
		return (1);
	}
	/*
	 * The st_dev from stat(2) is a dev_t. These kernel structures
	 * contain cdev pointers. We need to convert to dev_t to make
	 * comparisons
	 */
	vn->vn_fsid = dev2udev(kd, inode.i_dev);
	vn->vn_fileid = inode.i_number;
	vn->vn_mode = (mode_t)inode.i_mode;
	vn->vn_size = inode.i_size;
	return (0);
}

int
devfs_filestat(kvm_t *kd, struct vnode *vp, struct vnstat *vn)
{
	struct devfs_dirent devfs_dirent;
	struct mount mount;

	if (!kvm_read_all(kd, (unsigned long)getvnodedata(vp), &devfs_dirent,
	    sizeof(devfs_dirent))) {
		warnx("can't read devfs_dirent at %p",
		    (void *)vp->v_data);
		return (1);
	}
	if (!kvm_read_all(kd, (unsigned long)getvnodemount(vp), &mount,
	    sizeof(mount))) {
		warnx("can't read mount at %p",
		    (void *)getvnodemount(vp));
		return (1);
	}
	vn->vn_fsid = mount.mnt_stat.f_fsid.val[0];
	vn->vn_fileid = devfs_dirent.de_inode;
	vn->vn_mode = (devfs_dirent.de_mode & ~S_IFMT) | S_IFCHR;
	vn->vn_size = 0;
	return (0);
}

int
nfs_filestat(kvm_t *kd, struct vnode *vp, struct vnstat *vn)
{
	struct nfsnode nfsnode;
	mode_t mode;

	if (!kvm_read_all(kd, (unsigned long)VTONFS(vp), &nfsnode,
	    sizeof(nfsnode))) {
		warnx("can't read nfsnode at %p",
		    (void *)VTONFS(vp));
		return (1);
	}
	vn->vn_fsid = nfsnode.n_vattr.va_fsid;
	vn->vn_fileid = nfsnode.n_vattr.va_fileid;
	vn->vn_size = nfsnode.n_size;
	mode = (mode_t)nfsnode.n_vattr.va_mode;
	switch (vp->v_type) {
	case VREG:
		mode |= S_IFREG;
		break;
	case VDIR:
		mode |= S_IFDIR;
		break;
	case VBLK:
		mode |= S_IFBLK;
		break;
	case VCHR:
		mode |= S_IFCHR;
		break;
	case VLNK:
		mode |= S_IFLNK;
		break;
	case VSOCK:
		mode |= S_IFSOCK;
		break;
	case VFIFO:
		mode |= S_IFIFO;
		break;
	default:
		break;
	};
	vn->vn_mode = mode;
	return (0);
}

/*
 * Read the cdev structure in the kernel in order to work out the
 * associated dev_t
 */
dev_t
dev2udev(kvm_t *kd, struct cdev *dev)
{
	struct cdev_priv priv;

	assert(kd);
	if (kvm_read_all(kd, (unsigned long)cdev2priv(dev), &priv,
	    sizeof(priv))) {
		return ((dev_t)priv.cdp_inode);
	} else {
		warnx("can't convert cdev *%p to a dev_t\n", dev);
		return (-1);
	}
}

void *
getvnodedata(struct vnode *vp)
{
	return (vp->v_data);
}

struct mount *
getvnodemount(struct vnode *vp)
{
	return (vp->v_mount);
}