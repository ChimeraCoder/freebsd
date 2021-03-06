
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
#include <unistd.h>
#include <stdarg.h>
#include <pwd.h>
#include <libutil.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>

#include "pwupd.h"

#define HAVE_PWDB_C	1
#define	HAVE_PWDB_U	1

static char pathpwd[] = _PATH_PWD;
static char * pwpath = pathpwd;
 
int
setpwdir(const char * dir)
{
	if (dir == NULL)
		return -1;
	else
		pwpath = strdup(dir);
	if (pwpath == NULL)
		return -1;
	return 0;
}

char *
getpwpath(char const * file)
{
	static char pathbuf[MAXPATHLEN];

	snprintf(pathbuf, sizeof pathbuf, "%s/%s", pwpath, file);
	return pathbuf;
}

static int
pwdb(char *arg,...)
{
	int             i = 0;
	pid_t           pid;
	va_list         ap;
	char           *args[10];

	args[i++] = _PATH_PWD_MKDB;
	va_start(ap, arg);
	while (i < 6 && arg != NULL) {
		args[i++] = arg;
		arg = va_arg(ap, char *);
	}
	if (pwpath != pathpwd) {
		args[i++] = "-d";
		args[i++] = pwpath;
	}
	args[i++] = getpwpath(_MASTERPASSWD);
	args[i] = NULL;

	if ((pid = fork()) == -1)	/* Error (errno set) */
		i = errno;
	else if (pid == 0) {	/* Child */
		execv(args[0], args);
		_exit(1);
	} else {		/* Parent */
		waitpid(pid, &i, 0);
		if (WEXITSTATUS(i))
			i = EIO;
	}
	va_end(ap);
	return i;
}

static int
pw_update(struct passwd * pwd, char const * user)
{
	int             rc = 0;

	/*
	 * First, let's check the see if the database is alright
	 * Note: -C is only available in FreeBSD 2.2 and above
	 */
#ifdef HAVE_PWDB_C
	rc = pwdb("-C", (char *)NULL);	/* Check only */
	if (rc == 0) {
#else
	{				/* No -C */
#endif
		int pfd, tfd;
		struct passwd *pw = NULL;
		struct passwd *old_pw = NULL;

	       	if (pwd != NULL)
		       pw = pw_dup(pwd);

		if (user != NULL)
			old_pw = GETPWNAM(user);

		if (pw_init(pwpath, NULL))
			err(1, "pw_init()");
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
		/*
		 * in case of deletion of a user, the whole database
		 * needs to be regenerated
		 */
		if (pw_mkdb(pw != NULL ? user : NULL) == -1) {
			pw_fini();
			err(1, "pw_mkdb()");
		}
		free(pw);
		pw_fini();
	}
	return 0;
}

int
addpwent(struct passwd * pwd)
{
	return pw_update(pwd, NULL);
}

int
chgpwent(char const * login, struct passwd * pwd)
{
	return pw_update(pwd, login);
}

int
delpwent(struct passwd * pwd)
{
	char login[MAXLOGNAME];
	
	strlcpy(login, pwd->pw_name, MAXLOGNAME);
	return pw_update(NULL, login);
}