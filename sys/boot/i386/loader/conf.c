
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
#include <bootstrap.h>
#include "libi386/libi386.h"
#if defined(LOADER_ZFS_SUPPORT)
#include "../zfs/libzfs.h"
#endif

/*
 * We could use linker sets for some or all of these, but
 * then we would have to control what ended up linked into
 * the bootstrap.  So it's easier to conditionalise things
 * here.
 *
 * XXX rename these arrays to be consistent and less namespace-hostile
 *
 * XXX as libi386 and biosboot merge, some of these can become linker sets.
 */

#if defined(LOADER_NFS_SUPPORT) && defined(LOADER_TFTP_SUPPORT)
#error "Cannot have both tftp and nfs support yet."
#endif

#if defined(LOADER_FIREWIRE_SUPPORT)
extern struct devsw fwohci;
#endif

/* Exported for libstand */
struct devsw *devsw[] = {
    &bioscd,
    &biosdisk,
#if defined(LOADER_NFS_SUPPORT) || defined(LOADER_TFTP_SUPPORT)
    &pxedisk,
#endif
#if defined(LOADER_FIREWIRE_SUPPORT)
    &fwohci,
#endif
#if defined(LOADER_ZFS_SUPPORT)
    &zfs_dev,
#endif
    NULL
};

struct fs_ops *file_system[] = {
#if defined(LOADER_ZFS_SUPPORT)
    &zfs_fsops,
#endif
    &ufs_fsops,
    &ext2fs_fsops,
    &dosfs_fsops,
    &cd9660_fsops,
#if defined(LOADER_NANDFS_SUPPORT)
    &nandfs_fsops,
#endif
#ifdef LOADER_SPLIT_SUPPORT
    &splitfs_fsops,
#endif
#ifdef LOADER_GZIP_SUPPORT
    &gzipfs_fsops,
#endif
#ifdef LOADER_BZIP2_SUPPORT
    &bzipfs_fsops,
#endif
#ifdef LOADER_NFS_SUPPORT 
    &nfs_fsops,
#endif
#ifdef LOADER_TFTP_SUPPORT
    &tftp_fsops,
#endif
    NULL
};

/* Exported for i386 only */
/* 
 * Sort formats so that those that can detect based on arguments
 * rather than reading the file go first.
 */
extern struct file_format	i386_elf;
extern struct file_format	i386_elf_obj;
extern struct file_format	amd64_elf;
extern struct file_format	amd64_elf_obj;

struct file_format *file_formats[] = {
#ifdef LOADER_PREFER_AMD64
    &amd64_elf,
    &amd64_elf_obj,
#endif
    &i386_elf,
    &i386_elf_obj,
#ifndef LOADER_PREFER_AMD64
    &amd64_elf,
    &amd64_elf_obj,
#endif
    NULL
};

/* 
 * Consoles 
 *
 * We don't prototype these in libi386.h because they require
 * data structures from bootstrap.h as well.
 */
extern struct console vidconsole;
extern struct console comconsole;
#if defined(LOADER_FIREWIRE_SUPPORT)
extern struct console dconsole;
#endif
extern struct console nullconsole;
extern struct console spinconsole;

struct console *consoles[] = {
    &vidconsole,
    &comconsole,
#if defined(LOADER_FIREWIRE_SUPPORT)
    &dconsole,
#endif
    &nullconsole,
    &spinconsole,
    NULL
};

extern struct pnphandler isapnphandler;
extern struct pnphandler biospnphandler;
extern struct pnphandler biospcihandler;

struct pnphandler *pnphandlers[] = {
    &biospnphandler,		/* should go first, as it may set isapnp_readport */
    &isapnphandler,
    &biospcihandler,
    NULL
};