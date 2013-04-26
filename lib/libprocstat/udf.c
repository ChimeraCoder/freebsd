
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
#include <sys/buf.h>
#define _KERNEL
#include <sys/mount.h>
#undef _KERNEL

#include <netinet/in.h>

#include <assert.h>
#include <err.h>
#include <kvm.h>
#include <stdlib.h>

#include <fs/udf/ecma167-udf.h>

#include "libprocstat.h"
#include "common_kvm.h"

/* XXX */
struct udf_mnt {
	int			im_flags;
	struct mount		*im_mountp;
	struct g_consumer	*im_cp;
	struct bufobj		*im_bo;
	struct cdev		*im_dev;
	struct vnode		*im_devvp;
	int			bsize;
	int			bshift;
	int			bmask;
	uint32_t		part_start;
	uint32_t		part_len;
	uint64_t		root_id;
	struct long_ad		root_icb;
	int			p_sectors;
	int			s_table_entries;
	void			*s_table;
	void			*im_d2l;
};
struct udf_node {
	struct vnode	*i_vnode;
	struct udf_mnt	*udfmp;
	ino_t		hash_id;
	long		diroff;
	struct file_entry *fentry;
};
#define VTON(vp)	((struct udf_node *)((vp)->v_data))

int
udf_filestat(kvm_t *kd, struct vnode *vp, struct vnstat *vn)
{
	struct udf_node node;
	struct udf_mnt mnt;
	int error;

	assert(kd);
	assert(vn);
	error = kvm_read_all(kd, (unsigned long)VTON(vp), &node, sizeof(node));
	if (error != 0) {
		warnx("can't read udf fnode at %p", (void *)VTON(vp));
		return (1);
	}
        error = kvm_read_all(kd, (unsigned long)node.udfmp, &mnt, sizeof(mnt));
        if (error != 0) {
                warnx("can't read udf_mnt at %p for vnode %p",
                    (void *)node.udfmp, vp);
                return (1);
        }
	vn->vn_fileid = node.hash_id;
	vn->vn_fsid = dev2udev(kd, mnt.im_dev);
	return (0);
}