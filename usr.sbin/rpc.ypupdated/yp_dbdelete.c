
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <db.h>
#include <sys/stat.h>
#include <errno.h>
#include <paths.h>
#include <rpcsvc/yp.h>
#include "ypxfr_extern.h"

int
yp_del_record(DB *dbp, DBT *key)
{
	int rval;

	if ((rval = (dbp->del)(dbp,key,0))) {
		switch (rval) {
		case 1:
			return(YP_FALSE);
			break;
		case -1:
		default:
			(void)(dbp->close)(dbp);
			return(YP_BADDB);
			break;
		}
	}

	return(YP_TRUE);
}