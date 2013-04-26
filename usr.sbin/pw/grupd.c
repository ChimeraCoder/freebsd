
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <grp.h>
#include <libutil.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "pwupd.h"

static char * grpath = _PATH_PWD;

int
setgrdir(const char * dir)
{
	if (dir == NULL)
		return -1;
	else
		grpath = strdup(dir);
	if (grpath == NULL)
		return -1;

	return 0;
}

char *
getgrpath(const char * file)
{
	static char pathbuf[MAXPATHLEN];

	snprintf(pathbuf, sizeof pathbuf, "%s/%s", grpath, file);
	return pathbuf;
}

static int
gr_update(struct group * grp, char const * group)
{
	int pfd, tfd;
	struct group *gr = NULL;
	struct group *old_gr = NULL;

	if (grp != NULL)
		gr = gr_dup(grp);

	if (group != NULL)
		old_gr = GETGRNAM(group);

	if (gr_init(grpath, NULL))
		err(1, "gr_init()");

	if ((pfd = gr_lock()) == -1) {
		gr_fini();
		err(1, "gr_lock()");
	}
	if ((tfd = gr_tmp(-1)) == -1) {
		gr_fini();
		err(1, "gr_tmp()");
	}
	if (gr_copy(pfd, tfd, gr, old_gr) == -1) {
		gr_fini();
		err(1, "gr_copy()");
	}
	if (gr_mkdb() == -1) {
		gr_fini();
		err(1, "gr_mkdb()");
	}
	free(gr);
	gr_fini();
	return 0;
}


int
addgrent(struct group * grp)
{
	return gr_update(grp, NULL);
}

int
chggrent(char const * login, struct group * grp)
{
	return gr_update(grp, login);
}

int
delgrent(struct group * grp)
{
	char group[MAXLOGNAME];

	strlcpy(group, grp->gr_name, MAXLOGNAME);

	return gr_update(NULL, group);
}