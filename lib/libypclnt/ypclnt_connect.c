
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

#include <sys/param.h>

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rpcsvc/ypclnt.h>

#include "ypclnt.h"

int
ypclnt_connect(ypclnt_t *ypclnt)
{
	int r;

	/* get default domain name unless specified */
	if (ypclnt->domain == NULL) {
		if ((ypclnt->domain = malloc(MAXHOSTNAMELEN)) == NULL) {
			ypclnt_error(ypclnt, __func__,
			    "%s", strerror(errno));
			return (-1);
		}
		if (getdomainname(ypclnt->domain, MAXHOSTNAMELEN) != 0) {
			ypclnt_error(ypclnt, __func__,
			    "can't get NIS domain name");
			return (-1);
		}
	}

	/* map must be specified */
	if (ypclnt->map == NULL) {
		ypclnt_error(ypclnt, __func__,
		    "caller must specify map name");
		return (-1);
	}

	/* get master server for requested map unless specified */
	if (ypclnt->server == NULL) {
		r = yp_master(ypclnt->domain, ypclnt->map, &ypclnt->server);
		if (r != 0) {
			ypclnt_error(ypclnt, __func__,
			    "can't get NIS server name: %s", yperr_string(r));
			return (-1);
		}
	}

	ypclnt_error(ypclnt, NULL, NULL);
	return (0);
}