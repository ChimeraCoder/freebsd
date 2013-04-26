
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include <mad.h>
#include <infiniband/common.h>

#undef DEBUG
#define DEBUG	if (ibdebug)	IBWARN

int
portid2portnum(ib_portid_t *portid)
{
	if (portid->lid > 0)
		return -1;

	if (portid->drpath.cnt == 0)
		return 0;

	return portid->drpath.p[(portid->drpath.cnt-1)];
}

char *
portid2str(ib_portid_t *portid)
{
	static char buf[1024] = "local";
	int n = 0;

	if (portid->lid > 0) {
		n += sprintf(buf + n, "Lid %d", portid->lid);
		if (portid->grh_present) {
			char gid[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"];
			if (inet_ntop(AF_INET6, portid->gid, gid, sizeof(gid)))
				n += sprintf(buf + n, " Gid %s", gid);
		}
		if (portid->drpath.cnt)
			n += sprintf(buf + n, " ");
		else
			return buf;
	}
	n += sprintf(buf + n, "DR path ");
	drpath2str(&(portid->drpath), buf + n, sizeof(buf) - n);

	return buf;
}

int
str2drpath(ib_dr_path_t *path, char *routepath, int drslid, int drdlid)
{
	char *s, *str = routepath;

	path->cnt = -1;

	DEBUG("DR str: %s", routepath);
	while (str && *str) {
		if ((s = strchr(str, ',')))
			*s = 0;
		path->p[++path->cnt] = atoi(str);
		if (!s)
			break;
		str = s+1;
	}

	path->drdlid = drdlid ? drdlid : 0xffff;
	path->drslid = drslid ? drslid : 0xffff;

	return path->cnt;
}

char *
drpath2str(ib_dr_path_t *path, char *dstr, size_t dstr_size)
{
	int i = 0;
	int rc = snprintf(dstr, dstr_size, "slid %d; dlid %d; %d",
		path->drslid, path->drdlid, path->p[0]);
	if (rc >= dstr_size)
		return dstr;
	for (i = 1; i <= path->cnt; i++) {
		rc += snprintf(dstr+rc, dstr_size-rc, ",%d", path->p[i]);
		if (rc >= dstr_size)
			break;
	}
	return (dstr);
}