
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
static char sccsid[] = "@(#)initgroups.c	8.1 (Berkeley) 6/4/93";
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include "namespace.h"
#include <err.h>
#include "un-namespace.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
initgroups(uname, agroup)
	const char *uname;
	gid_t agroup;
{
	int ngroups, ret;
	long ngroups_max;
	gid_t *groups;

	/*
	 * Provide space for one group more than possible to allow
	 * setgroups to fail and set errno.
	 */
	ngroups_max = sysconf(_SC_NGROUPS_MAX) + 2;
	if ((groups = malloc(sizeof(*groups) * ngroups_max)) == NULL)
		return (ENOMEM);

	ngroups = (int)ngroups_max;
	getgrouplist(uname, agroup, groups, &ngroups);
	ret = setgroups(ngroups, groups);
	free(groups);
	return (ret);
}