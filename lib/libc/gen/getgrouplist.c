
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
static char sccsid[] = "@(#)getgrouplist.c	8.2 (Berkeley) 12/8/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * get credential
 */
#include <sys/types.h>

#include <grp.h>
#include <string.h>
#include <unistd.h>

extern int __getgroupmembership(const char *, gid_t, gid_t *, int, int *);

int
getgrouplist(const char *uname, gid_t agroup, gid_t *groups, int *grpcnt)
{
	return __getgroupmembership(uname, agroup, groups, *grpcnt, grpcnt);
}