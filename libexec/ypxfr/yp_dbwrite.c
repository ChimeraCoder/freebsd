
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

#include <db.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <rpcsvc/yp.h>
#include "ypxfr_extern.h"

#define PERM_SECURE (S_IRUSR|S_IWUSR)

/*
 * Open a DB database read/write
 */
DB *
yp_open_db_rw(const char *domain, const char *map, const int flags)
{
	DB *dbp;
	char buf[1025];


	yp_errno = YP_TRUE;

	if (map[0] == '.' || strchr(map, '/')) {
		yp_errno = YP_BADARGS;
		return (NULL);
	}

#define FLAGS O_RDWR|O_EXLOCK|O_EXCL|O_CREAT

	snprintf(buf, sizeof(buf), "%s/%s/%s", yp_dir, domain, map);
	dbp = dbopen(buf,flags ? flags : FLAGS,PERM_SECURE,DB_HASH,&openinfo);

	if (dbp == NULL) {
		switch (errno) {
		case ENOENT:
			yp_errno = YP_NOMAP;
			break;
		case EFTYPE:
			yp_errno = YP_BADDB;
			break;
		default:
			yp_errno = YP_YPERR;
			break;
		}
	}

	return (dbp);
}

int
yp_put_record(DB *dbp, DBT *key, DBT *data, int allow_overwrite)
{
	int rval;

	if ((rval = (dbp->put)(dbp,key,data, allow_overwrite ? 0 :
							R_NOOVERWRITE))) {
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