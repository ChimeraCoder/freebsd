
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
static char sccsid[] = "@(#)netname.c 1.8 91/03/11 Copyr 1986 Sun Micro";
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * netname utility routines
 * convert from unix names to network names and vice-versa
 * This module is operating system dependent!
 * What we define here will work with any unix system that has adopted
 * the sun NIS domain architecture.
 */

#include "namespace.h"
#include <sys/param.h>
#include <rpc/rpc.h>
#include <rpc/rpc_com.h>
#ifdef YP
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#endif
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

#define TYPE_BIT(type)  (sizeof (type) * CHAR_BIT)

#define TYPE_SIGNED(type) (((type) -1) < 0)

/*
** 302 / 1000 is log10(2.0) rounded up.
** Subtract one for the sign bit if the type is signed;
** add one for integer division truncation;
** add one more for a minus sign if the type is signed.
*/
#define INT_STRLEN_MAXIMUM(type) \
    ((TYPE_BIT(type) - TYPE_SIGNED(type)) * 302 / 1000 + 1 + TYPE_SIGNED(type))

static char *OPSYS = "unix";

/*
 * Figure out my fully qualified network name
 */
int
getnetname(name)
	char name[MAXNETNAMELEN+1];
{
	uid_t uid;

	uid = geteuid();
	if (uid == 0) {
		return (host2netname(name, (char *) NULL, (char *) NULL));
	} else {
		return (user2netname(name, uid, (char *) NULL));
	}
}


/*
 * Convert unix cred to network-name
 */
int
user2netname(netname, uid, domain)
	char netname[MAXNETNAMELEN + 1];
	const uid_t uid;
	const char *domain;
{
	char *dfltdom;

	if (domain == NULL) {
		if (__rpc_get_default_domain(&dfltdom) != 0) {
			return (0);
		}
		domain = dfltdom;
	}
	if (strlen(domain) + 1 + INT_STRLEN_MAXIMUM(u_long) + 1 + strlen(OPSYS) > MAXNETNAMELEN) {
		return (0);
	}
	(void) sprintf(netname, "%s.%ld@%s", OPSYS, (u_long)uid, domain);	
	return (1);
}


/*
 * Convert host to network-name
 */
int
host2netname(netname, host, domain)
	char netname[MAXNETNAMELEN + 1];
	const char *host;
	const char *domain;
{
	char *dfltdom;
	char hostname[MAXHOSTNAMELEN+1];

	if (domain == NULL) {
		if (__rpc_get_default_domain(&dfltdom) != 0) {
			return (0);
		}
		domain = dfltdom;
	}
	if (host == NULL) {
		(void) gethostname(hostname, sizeof(hostname));
		host = hostname;
	}
	if (strlen(domain) + 1 + strlen(host) + 1 + strlen(OPSYS) > MAXNETNAMELEN) {
		return (0);
	} 
	(void) sprintf(netname, "%s.%s@%s", OPSYS, host, domain);
	return (1);
}