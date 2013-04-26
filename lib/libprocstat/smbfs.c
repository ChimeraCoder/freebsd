
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
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/vnode.h>
#define _KERNEL
#include <sys/mount.h>
#undef _KERNEL

#include <netinet/in.h>

#include <assert.h>
#include <err.h>
#include <kvm.h>
#include <stdlib.h>

#include <fs/smbfs/smbfs.h>
#include <fs/smbfs/smbfs_node.h>

#include "libprocstat.h"
#include "common_kvm.h"

int
smbfs_filestat(kvm_t *kd, struct vnode *vp, struct vnstat *vn)
{
	struct smbnode node;
	struct mount mnt;
	int error;

	assert(kd);
	assert(vn);
	error = kvm_read_all(kd, (unsigned long)VTOSMB(vp), &node,
	    sizeof(node));
	if (error != 0) {
		warnx("can't read smbfs fnode at %p", (void *)VTOSMB(vp));
		return (1);
	}
        error = kvm_read_all(kd, (unsigned long)getvnodemount(vp), &mnt,
	    sizeof(mnt));
        if (error != 0) {
                warnx("can't read mount at %p for vnode %p",
                    (void *)getvnodemount(vp), vp);
                return (1);
        }
	vn->vn_fileid = node.n_ino;
	if (vn->vn_fileid == 0)
		vn->vn_fileid = 2;
	vn->vn_fsid = mnt.mnt_stat.f_fsid.val[0];
	return (0);
}