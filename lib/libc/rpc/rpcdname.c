
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
static char sccsid[] = "@(#)rpcdname.c 1.7 91/03/11 Copyr 1989 Sun Micro";
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * rpcdname.c
 * Gets the default domain name
 */

#include "namespace.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "un-namespace.h"

static char *default_domain = 0;

static char *
get_default_domain()
{
	char temp[256];

	if (default_domain)
		return (default_domain);
	if (getdomainname(temp, sizeof(temp)) < 0)
		return (0);
	if ((int) strlen(temp) > 0) {
		default_domain = (char *)malloc((strlen(temp)+(unsigned)1));
		if (default_domain == 0)
			return (0);
		(void) strcpy(default_domain, temp);
		return (default_domain);
	}
	return (0);
}

/*
 * This is a wrapper for the system call getdomainname which returns a
 * ypclnt.h error code in the failure case.  It also checks to see that
 * the domain name is non-null, knowing that the null string is going to
 * get rejected elsewhere in the NIS client package.
 */
int
__rpc_get_default_domain(domain)
	char **domain;
{
	if ((*domain = get_default_domain()) != 0)
		return (0);
	return (-1);
}