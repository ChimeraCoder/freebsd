
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

#include <sys/zfs_ioctl.h>
#include <zfs_ioctl_compat.h>
#include "libzfs_core_compat.h"

extern int zfs_ioctl_version;

int
lzc_compat_pre(zfs_cmd_t *zc, zfs_ioc_t *ioc, nvlist_t **source)
{
	nvlist_t *nvl = NULL;
	nvpair_t *pair, *hpair;
	char *buf, *val;
	zfs_ioc_t vecnum;
	uint32_t type32;
	int32_t cleanup_fd;
	int error = 0;
	int pos;

	if (zfs_ioctl_version >= ZFS_IOCVER_LZC)
		return (0);

	vecnum = *ioc;

	switch (vecnum) {
	case ZFS_IOC_CREATE:
		type32 = fnvlist_lookup_int32(*source, "type");
		zc->zc_objset_type = (uint64_t)type32;
		nvlist_lookup_nvlist(*source, "props", &nvl);
		*source = nvl;
	break;
	case ZFS_IOC_CLONE:
		buf = fnvlist_lookup_string(*source, "origin");
		strlcpy(zc->zc_value, buf, MAXPATHLEN);
		nvlist_lookup_nvlist(*source, "props", &nvl);
		*ioc = ZFS_IOC_CREATE;
		*source = nvl;
	break;
	case ZFS_IOC_SNAPSHOT:
		nvl = fnvlist_lookup_nvlist(*source, "snaps");
		pair = nvlist_next_nvpair(nvl, NULL);
		if (pair != NULL) {
			buf = nvpair_name(pair);
			pos = strcspn(buf, "@");
			strlcpy(zc->zc_name, buf, pos + 1);
			strlcpy(zc->zc_value, buf + pos + 1, MAXPATHLEN);
		} else
			error = EINVAL;
		/* old kernel cannot create multiple snapshots */
		if (!error && nvlist_next_nvpair(nvl, pair) != NULL)
			error = EOPNOTSUPP;
		nvlist_free(nvl);
		nvl = NULL;
		nvlist_lookup_nvlist(*source, "props", &nvl);
		*source = nvl;
	break;
	case ZFS_IOC_SPACE_SNAPS:
		buf = fnvlist_lookup_string(*source, "firstsnap");
		strlcpy(zc->zc_value, buf, MAXPATHLEN);
	break;
	case ZFS_IOC_DESTROY_SNAPS:
		nvl = fnvlist_lookup_nvlist(*source, "snaps");
		pair = nvlist_next_nvpair(nvl, NULL);
		if (pair != NULL) {
			buf = nvpair_name(pair);
			pos = strcspn(buf, "@");
			strlcpy(zc->zc_name, buf, pos + 1);
		} else
			error = EINVAL;
		/* old kernel cannot atomically destroy multiple snaps */
		if (!error && nvlist_next_nvpair(nvl, pair) != NULL)
			error = EOPNOTSUPP;
		*source = nvl;
	break;
	case ZFS_IOC_HOLD:
		nvl = fnvlist_lookup_nvlist(*source, "holds");
		pair = nvlist_next_nvpair(nvl, NULL);
		if (pair != NULL) {
			buf = nvpair_name(pair);
			pos = strcspn(buf, "@");
			strlcpy(zc->zc_name, buf, pos + 1);
			strlcpy(zc->zc_value, buf + pos + 1, MAXPATHLEN);
			if (nvpair_value_string(pair, &val) == 0)
				strlcpy(zc->zc_string, val, MAXNAMELEN);
			else
				error = EINVAL;
		} else
			error = EINVAL;
		/* old kernel cannot atomically create multiple holds */
		if (!error && nvlist_next_nvpair(nvl, pair) != NULL)
			error = EOPNOTSUPP;
		nvlist_free(nvl);
		if (nvlist_lookup_int32(*source, "cleanup_fd",
		    &cleanup_fd) == 0)
			zc->zc_cleanup_fd = cleanup_fd;
		else
			zc->zc_cleanup_fd = -1;
	break;
	case ZFS_IOC_RELEASE:
		pair = nvlist_next_nvpair(*source, NULL);
		if (pair != NULL) {
			buf = nvpair_name(pair);
			pos = strcspn(buf, "@");
			strlcpy(zc->zc_name, buf, pos + 1);
			strlcpy(zc->zc_value, buf + pos + 1, MAXPATHLEN);
			if (nvpair_value_nvlist(pair, &nvl) == 0) {
				hpair = nvlist_next_nvpair(nvl, NULL);
				if (hpair != NULL)
					strlcpy(zc->zc_string,
					    nvpair_name(hpair), MAXNAMELEN);
				else
					error = EINVAL;
				if (!error && nvlist_next_nvpair(nvl,
				    hpair) != NULL)
					error = EOPNOTSUPP;
			} else
				error = EINVAL;
		} else
			error = EINVAL;
		/* old kernel cannot atomically release multiple holds */
		if (!error && nvlist_next_nvpair(nvl, pair) != NULL)
			error = EOPNOTSUPP;
	break;
	}

	return (error);
}

void
lzc_compat_post(zfs_cmd_t *zc, const zfs_ioc_t ioc)
{
	if (zfs_ioctl_version >= ZFS_IOCVER_LZC)
		return;

	switch (ioc) {
	case ZFS_IOC_CREATE:
	case ZFS_IOC_CLONE:
	case ZFS_IOC_SNAPSHOT:
	case ZFS_IOC_SPACE_SNAPS:
	case ZFS_IOC_DESTROY_SNAPS:
		zc->zc_nvlist_dst_filled = B_FALSE;
	break;
	}
}

int
lzc_compat_outnvl(zfs_cmd_t *zc, const zfs_ioc_t ioc, nvlist_t **outnvl)
{
	nvlist_t *nvl;

	if (zfs_ioctl_version >= ZFS_IOCVER_LZC)
		return (0);

	switch (ioc) {
	case ZFS_IOC_SPACE_SNAPS:
		nvl = fnvlist_alloc();
		fnvlist_add_uint64(nvl, "used", zc->zc_cookie);
		fnvlist_add_uint64(nvl, "compressed", zc->zc_objset_type);
		fnvlist_add_uint64(nvl, "uncompressed", zc->zc_perm_action);
		*outnvl = nvl;
	break;
	}

	return (0);
}