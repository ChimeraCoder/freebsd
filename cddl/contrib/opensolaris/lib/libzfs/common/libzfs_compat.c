
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
 * Copyright (c) 2013 Martin Matuska <mm@FreeBSD.org>. All rights reserved.
 */

#include "libzfs_compat.h"

int zfs_ioctl_version = ZFS_IOCVER_UNDEF;
static int zfs_spa_version = -1;

/*
 * Get zfs_ioctl_version
 */
int
get_zfs_ioctl_version(void)
{
	size_t ver_size;
	int ver = ZFS_IOCVER_NONE;

	ver_size = sizeof(ver);
	sysctlbyname("vfs.zfs.version.ioctl", &ver, &ver_size, NULL, 0);

	return (ver);
}

/*
 * Get the SPA version
 */
static int
get_zfs_spa_version(void)
{
	size_t ver_size;
	int ver = 0;

	ver_size = sizeof(ver);
	sysctlbyname("vfs.zfs.version.spa", &ver, &ver_size, NULL, 0);

	return (ver);
}

/*
 * This is FreeBSD version of ioctl, because Solaris' ioctl() updates
 * zc_nvlist_dst_size even if an error is returned, on FreeBSD if an
 * error is returned zc_nvlist_dst_size won't be updated.
 */
int
zcmd_ioctl(int fd, int request, zfs_cmd_t *zc)
{
	size_t oldsize;
	int ret, cflag = ZFS_CMD_COMPAT_NONE;

	if (zfs_ioctl_version == ZFS_IOCVER_UNDEF)
		zfs_ioctl_version = get_zfs_ioctl_version();

	if (zfs_ioctl_version == ZFS_IOCVER_LZC)
		cflag = ZFS_CMD_COMPAT_LZC;
	else if (zfs_ioctl_version == ZFS_IOCVER_DEADMAN)
		cflag = ZFS_CMD_COMPAT_DEADMAN;

	/*
	 * If vfs.zfs.version.ioctl is not defined, assume we have v28
	 * compatible binaries and use vfs.zfs.version.spa to test for v15
	 */
	if (zfs_ioctl_version < ZFS_IOCVER_DEADMAN) {
		cflag = ZFS_CMD_COMPAT_V28;

		if (zfs_spa_version < 0)
			zfs_spa_version = get_zfs_spa_version();

		if (zfs_spa_version == SPA_VERSION_15 ||
		    zfs_spa_version == SPA_VERSION_14 ||
		    zfs_spa_version == SPA_VERSION_13)
			cflag = ZFS_CMD_COMPAT_V15;
	}

	oldsize = zc->zc_nvlist_dst_size;
	ret = zcmd_ioctl_compat(fd, request, zc, cflag);

	if (ret == 0 && oldsize < zc->zc_nvlist_dst_size) {
		ret = -1;
		errno = ENOMEM;
	}

	return (ret);
}