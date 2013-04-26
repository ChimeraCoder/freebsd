
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

/* OPENBSD ORIGINAL: lib/libc/gen/getgrouplist.c */

#include "includes.h"

#ifndef HAVE_GETGROUPLIST

/*
 * get credential
 */
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>

int
getgrouplist(const char *uname, gid_t agroup, gid_t *groups, int *grpcnt)
{
	struct group *grp;
	int i, ngroups;
	int ret, maxgroups;
	int bail;

	ret = 0;
	ngroups = 0;
	maxgroups = *grpcnt;

	/*
	 * install primary group
	 */
	if (ngroups >= maxgroups) {
		*grpcnt = ngroups;
		return (-1);
	}
	groups[ngroups++] = agroup;

	/*
	 * Scan the group file to find additional groups.
	 */
	setgrent();
	while ((grp = getgrent())) {
		if (grp->gr_gid == agroup)
			continue;
		for (bail = 0, i = 0; bail == 0 && i < ngroups; i++)
			if (groups[i] == grp->gr_gid)
				bail = 1;
		if (bail)
			continue;
		for (i = 0; grp->gr_mem[i]; i++) {
			if (!strcmp(grp->gr_mem[i], uname)) {
				if (ngroups >= maxgroups) {
					ret = -1;
					goto out;
				}
				groups[ngroups++] = grp->gr_gid;
				break;
			}
		}
	}
out:
	endgrent();
	*grpcnt = ngroups;
	return (ret);
}

#endif /* HAVE_GETGROUPLIST */