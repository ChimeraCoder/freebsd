
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
#include "namespace.h"
#include <sys/acl.h>
#include "un-namespace.h"
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "acl_support.h"

/*
 * Given a uid/gid, return a username/groupname for the text form of an ACL.
 * Note that we truncate user and group names, rather than error out, as
 * this is consistent with other tools manipulating user and group names.
 * XXX NOT THREAD SAFE, RELIES ON GETPWUID, GETGRGID
 * XXX USES *PW* AND *GR* WHICH ARE STATEFUL AND THEREFORE THIS ROUTINE
 * MAY HAVE SIDE-EFFECTS
 */
int
_posix1e_acl_id_to_name(acl_tag_t tag, uid_t id, ssize_t buf_len, char *buf,
    int flags)
{
	struct group	*g;
	struct passwd	*p;
	int	i;

	switch(tag) {
	case ACL_USER:
		if (flags & ACL_TEXT_NUMERIC_IDS)
			p = NULL;
		else
			p = getpwuid(id);
		if (!p)
			i = snprintf(buf, buf_len, "%d", id);
		else
			i = snprintf(buf, buf_len, "%s", p->pw_name);

		if (i < 0) {
			errno = ENOMEM;
			return (-1);
		}
		return (0);

	case ACL_GROUP:
		if (flags & ACL_TEXT_NUMERIC_IDS)
			g = NULL;
		else
			g = getgrgid(id);
		if (g == NULL)
			i = snprintf(buf, buf_len, "%d", id);
		else
			i = snprintf(buf, buf_len, "%s", g->gr_name);

		if (i < 0) {
			errno = ENOMEM;
			return (-1);
		}
		return (0);

	default:
		return (EINVAL);
	}
}