
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

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/sysctl.h>

#include "fuse.h"

static void fuse_bringdown(eventhandler_tag eh_tag);
static int fuse_loader(struct module *m, int what, void *arg);

struct mtx fuse_mtx;

extern struct vfsops fuse_vfsops;
extern struct cdevsw fuse_cdevsw;
extern struct vop_vector fuse_vnops;
extern int fuse_pbuf_freecnt;

static struct vfsconf fuse_vfsconf = {
	.vfc_version = VFS_VERSION,
	.vfc_name = "fusefs",
	.vfc_vfsops = &fuse_vfsops,
	.vfc_typenum = -1,
	.vfc_flags = VFCF_SYNTHETIC
};

SYSCTL_INT(_vfs_fuse, OID_AUTO, kernelabi_major, CTLFLAG_RD,
    0, FUSE_KERNEL_VERSION, "FUSE kernel abi major version");
SYSCTL_INT(_vfs_fuse, OID_AUTO, kernelabi_minor, CTLFLAG_RD,
    0, FUSE_KERNEL_MINOR_VERSION, "FUSE kernel abi minor version");

/******************************
 *
 * >>> Module management stuff
 *
 ******************************/

static void
fuse_bringdown(eventhandler_tag eh_tag)
{

	fuse_ipc_destroy();
	fuse_device_destroy();
	mtx_destroy(&fuse_mtx);
}

static int
fuse_loader(struct module *m, int what, void *arg)
{
	static eventhandler_tag eh_tag = NULL;
	int err = 0;

	switch (what) {
	case MOD_LOAD:			/* kldload */
		fuse_pbuf_freecnt = nswbuf / 2 + 1;
		mtx_init(&fuse_mtx, "fuse_mtx", NULL, MTX_DEF);
		err = fuse_device_init();
		if (err) {
			mtx_destroy(&fuse_mtx);
			return (err);
		}
		fuse_ipc_init();

		/* vfs_modevent ignores its first arg */
		if ((err = vfs_modevent(NULL, what, &fuse_vfsconf)))
			fuse_bringdown(eh_tag);
		else
			printf("fuse-freebsd: version %s, FUSE ABI %d.%d\n",
			    FUSE_FREEBSD_VERSION,
			    FUSE_KERNEL_VERSION, FUSE_KERNEL_MINOR_VERSION);

		break;
	case MOD_UNLOAD:
		if ((err = vfs_modevent(NULL, what, &fuse_vfsconf)))
			return (err);
		fuse_bringdown(eh_tag);
		break;
	default:
		return (EINVAL);
	}

	return (err);
}

/* Registering the module */

static moduledata_t fuse_moddata = {
	"fuse",
	fuse_loader,
	&fuse_vfsconf
};

DECLARE_MODULE(fuse, fuse_moddata, SI_SUB_VFS, SI_ORDER_MIDDLE);
MODULE_VERSION(fuse, 1);