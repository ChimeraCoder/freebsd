
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

#include "includes.h"

#if !defined(HAVE_GETPEEREID)

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#if defined(SO_PEERCRED)
int
getpeereid(int s, uid_t *euid, gid_t *gid)
{
	struct ucred cred;
	socklen_t len = sizeof(cred);

	if (getsockopt(s, SOL_SOCKET, SO_PEERCRED, &cred, &len) < 0)
		return (-1);
	*euid = cred.uid;
	*gid = cred.gid;

	return (0);
}
#elif defined(HAVE_GETPEERUCRED)

#ifdef HAVE_UCRED_H
# include <ucred.h>
#endif

int
getpeereid(int s, uid_t *euid, gid_t *gid)
{
	ucred_t *ucred = NULL;

	if (getpeerucred(s, &ucred) == -1)
		return (-1);
	if ((*euid = ucred_geteuid(ucred)) == -1)
		return (-1);
	if ((*gid = ucred_getrgid(ucred)) == -1)
		return (-1);

	ucred_free(ucred);

	return (0);
}
#else
int
getpeereid(int s, uid_t *euid, gid_t *gid)
{
	*euid = geteuid();
	*gid = getgid();

	return (0);
}
#endif /* defined(SO_PEERCRED) */

#endif /* !defined(HAVE_GETPEEREID) */