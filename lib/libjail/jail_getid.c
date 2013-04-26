
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/jail.h>
#include <sys/uio.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jail.h"


/*
 * Return the JID corresponding to a jail name.
 */
int
jail_getid(const char *name)
{
	char *ep;
	int jid;
	struct iovec jiov[4];

	jid = strtoul(name, &ep, 10);
	if (*name && !*ep)
		return jid;
	*(const void **)&jiov[0].iov_base = "name";
	jiov[0].iov_len = sizeof("name");
	jiov[1].iov_len = strlen(name) + 1;
	jiov[1].iov_base = alloca(jiov[1].iov_len);
	strcpy(jiov[1].iov_base, name);
	*(const void **)&jiov[2].iov_base = "errmsg";
	jiov[2].iov_len = sizeof("errmsg");
	jiov[3].iov_base = jail_errmsg;
	jiov[3].iov_len = JAIL_ERRMSGLEN;
	jail_errmsg[0] = 0;
	jid = jail_get(jiov, 4, 0);
	if (jid < 0 && !jail_errmsg[0])
		snprintf(jail_errmsg, JAIL_ERRMSGLEN, "jail_get: %s",
		    strerror(errno));
	return jid;
}

/*
 * Return the name corresponding to a JID.
 */
char *
jail_getname(int jid)
{
	struct iovec jiov[6];
	char *name;
	char namebuf[MAXHOSTNAMELEN];

	*(const void **)&jiov[0].iov_base = "jid";
	jiov[0].iov_len = sizeof("jid");
	jiov[1].iov_base = &jid;
	jiov[1].iov_len = sizeof(jid);
	*(const void **)&jiov[2].iov_base = "name";
	jiov[2].iov_len = sizeof("name");
	jiov[3].iov_base = namebuf;
	jiov[3].iov_len = sizeof(namebuf);
	*(const void **)&jiov[4].iov_base = "errmsg";
	jiov[4].iov_len = sizeof("errmsg");
	jiov[5].iov_base = jail_errmsg;
	jiov[5].iov_len = JAIL_ERRMSGLEN;
	jail_errmsg[0] = 0;
	jid = jail_get(jiov, 6, 0);
	if (jid < 0) {
		if (!jail_errmsg[0])
			snprintf(jail_errmsg, JAIL_ERRMSGLEN, "jail_get: %s",
			    strerror(errno));
		return NULL;
	} else {
		name = strdup(namebuf);
		if (name == NULL)
			strerror_r(errno, jail_errmsg, JAIL_ERRMSGLEN);
	}
	return name;
}