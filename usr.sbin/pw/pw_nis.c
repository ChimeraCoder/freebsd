
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <err.h>
#include <pwd.h>
#include <libutil.h>

#include "pw.h"

static int
pw_nisupdate(const char * path, struct passwd * pwd, char const * user)
{
	int pfd, tfd;
	struct passwd *pw = NULL;
	struct passwd *old_pw = NULL;

	if (pwd != NULL)
		pw = pw_dup(pwd);

	if (user != NULL)
		old_pw = GETPWNAM(user);

	if (pw_init(NULL, path))
		err(1,"pw_init()");
	if ((pfd = pw_lock()) == -1) {
		pw_fini();
		err(1, "pw_lock()");
	}
	if ((tfd = pw_tmp(-1)) == -1) {
		pw_fini();
		err(1, "pw_tmp()");
	}
	if (pw_copy(pfd, tfd, pw, old_pw) == -1) {
		pw_fini();
		err(1, "pw_copy()");
	}
	if (chmod(pw_tempname(), 0644) == -1)
		err(1, "chmod()");
	if (rename(pw_tempname(), path) == -1)
		err(1, "rename()");

	free(pw);
	pw_fini();

	return (0);
}

int
addnispwent(const char *path, struct passwd * pwd)
{
	return pw_nisupdate(path, pwd, NULL);
}

int
chgnispwent(const char *path, char const * login, struct passwd * pwd)
{
	return pw_nisupdate(path, pwd, login);
}

int
delnispwent(const char *path, const char *login)
{
	return pw_nisupdate(path, NULL, login);
}