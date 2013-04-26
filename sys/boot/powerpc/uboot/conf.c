
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

#include <stand.h>
#include "bootstrap.h"
#include "libuboot.h"

#if defined(LOADER_NET_SUPPORT)
#include "dev_net.h"
#endif

/*
 * We could use linker sets for some or all of these, but
 * then we would have to control what ended up linked into
 * the bootstrap.  So it's easier to conditionalise things
 * here.
 *
 * XXX rename these arrays to be consistent and less namespace-hostile
 */

/* Exported for libstand */
struct devsw *devsw[] = {
#if defined(LOADER_DISK_SUPPORT) || defined(LOADER_CD9660_SUPPORT)
    &uboot_storage,
#endif
#if defined(LOADER_NET_SUPPORT)
    &netdev,
#endif
    NULL
};

struct fs_ops *file_system[] = {
#if defined(LOADER_UFS_SUPPORT)
    &ufs_fsops,
#endif
#if defined(LOADER_CD9660_SUPPORT)
    &cd9660_fsops,
#endif
#if defined(LOADER_EXT2FS_SUPPORT)
    &ext2fs_fsops,
#endif
#if defined(LOADER_NFS_SUPPORT)
    &nfs_fsops,
#endif
#if defined(LOADER_TFTP_SUPPORT)
    &tftp_fsops,
#endif
#if defined(LOADER_GZIP_SUPPORT)
    &gzipfs_fsops,
#endif
#if defined(LOADER_BZIP2_SUPPORT)
    &bzipfs_fsops,
#endif
    NULL
};

struct netif_driver *netif_drivers[] = {
#if defined(LOADER_NET_SUPPORT)
	&uboot_net,
#endif
	NULL,
};

/* Exported for PowerPC only */
/* 
 * Sort formats so that those that can detect based on arguments
 * rather than reading the file go first.
 */

struct file_format *file_formats[] = {
	&uboot_elf,
	NULL
};

/* 
 * Consoles 
 */
extern struct console uboot_console;

struct console *consoles[] = {
	&uboot_console,
	NULL
};