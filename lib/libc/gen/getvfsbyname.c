
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)kvm_getvfsbyname.c	8.1 (Berkeley) 4/3/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
 * Given a filesystem name, determine if it is resident in the kernel,
 * and if it is resident, return its xvfsconf structure.
 */
int
getvfsbyname(fsname, vfcp)
	const char *fsname;
	struct xvfsconf *vfcp;
{
	struct xvfsconf *xvfsp;
	size_t buflen;
	int cnt, i;

	if (sysctlbyname("vfs.conflist", NULL, &buflen, NULL, 0) < 0)
		return (-1);
	xvfsp = malloc(buflen);
	if (xvfsp == NULL)
		return (-1);
	if (sysctlbyname("vfs.conflist", xvfsp, &buflen, NULL, 0) < 0) {
		free(xvfsp);
		return (-1);
	}
	cnt = buflen / sizeof(struct xvfsconf);
	for (i = 0; i < cnt; i++) {
		if (strcmp(fsname, xvfsp[i].vfc_name) == 0) {
			memcpy(vfcp, xvfsp + i, sizeof(struct xvfsconf));
			free(xvfsp);
			return (0);
		}
	}
	free(xvfsp);
	errno = ENOENT;
	return (-1);
}