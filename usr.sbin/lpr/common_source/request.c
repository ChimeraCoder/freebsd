
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

static const char copyright[] =
	"Copyright (C) 1997, Massachusetts Institute of Technology\r\n";

#include "lp.cdefs.h"		/* A cross-platform version of <sys/cdefs.h> */
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>

#include <sys/param.h>		/* needed for lp.h but not used here */
#include <dirent.h>		/* ditto */
#include <stdio.h>		/* ditto */
#include "lp.h"
#include "lp.local.h"

void
init_request(struct request *rp)
{
	static struct request zero;

	*rp = zero;
	TAILQ_INIT(&rp->users);
	TAILQ_INIT(&rp->jobids);
}

void
free_request(struct request *rp)
{
	struct req_user *ru;
	struct req_jobid *rj;

	if (rp->logname)
		free(rp->logname);
	if (rp->authname)
		free(rp->authname);
	if (rp->prettyname)
		free(rp->prettyname);
	if (rp->authinfo)
		free(rp->authinfo);
	while ((ru = TAILQ_FIRST(&rp->users)) != 0) {
		TAILQ_REMOVE(&rp->users, ru, ru_link);
		free(ru);
	}
	while ((rj = TAILQ_FIRST(&rp->jobids)) != 0) {
		TAILQ_REMOVE(&rp->jobids, rj, rj_link);
		free(rj);
	}
	init_request(rp);
}