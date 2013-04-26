
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

/*
 * XXX -
 * This had to be separated from fstat.c because cd9660s has namespace
 * conflicts with UFS.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/mount.h>

#include <netinet/in.h>

#include <err.h>

#include <isofs/cd9660/cd9660_node.h>
#define _KERNEL
#include <isofs/cd9660/iso.h>
#undef _KERNEL

#include <kvm.h>
#include <stdio.h>

#include "libprocstat.h"
#include "common_kvm.h"

int
isofs_filestat(kvm_t *kd, struct vnode *vp, struct vnstat *vn)
{
	struct iso_node isonode;
	struct iso_mnt mnt;

	if (!kvm_read_all(kd, (unsigned long)VTOI(vp), &isonode,
	    sizeof(isonode))) {
		warnx("can't read iso_node at %p",
		    (void *)VTOI(vp));
		return (1);
	}
	if (!kvm_read_all(kd, (unsigned long)isonode.i_mnt, &mnt,
	    sizeof(mnt))) {
		warnx("can't read iso_mnt at %p",
		    (void *)VTOI(vp));
		return (1);
	}
	vn->vn_fsid = dev2udev(kd, mnt.im_dev);
	vn->vn_mode = (mode_t)isonode.inode.iso_mode;
	vn->vn_fileid = isonode.i_number;
	vn->vn_size = isonode.i_size;
	return (0);
}