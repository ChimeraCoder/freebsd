
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

#if !defined(lint) && !defined(LINT)
static const char rcsid[] =
  "$FreeBSD$";
#endif

/* vix 26jan87 [log is in RCS file]
 */


#include "cron.h"

static char *User_name;

void
free_user(u)
	user	*u;
{
	entry	*e, *ne;

	free(u->name);
	for (e = u->crontab;  e != NULL;  e = ne) {
		ne = e->next;
		free_entry(e);
	}
	free(u);
}

static void
log_error(msg)
	char	*msg;
{
	log_it(User_name, getpid(), "PARSE", msg);
}

user *
load_user(crontab_fd, pw, name)
	int		crontab_fd;
	struct passwd	*pw;		/* NULL implies syscrontab */
	char		*name;
{
	char	envstr[MAX_ENVSTR];
	FILE	*file;
	user	*u;
	entry	*e;
	int	status;
	char	**envp, **tenvp;

	if (!(file = fdopen(crontab_fd, "r"))) {
		warn("fdopen on crontab_fd in load_user");
		return NULL;
	}

	Debug(DPARS, ("load_user()\n"))

	/* file is open.  build user entry, then read the crontab file.
	 */
	if ((u = (user *) malloc(sizeof(user))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	if ((u->name = strdup(name)) == NULL) {
		free(u);
		errno = ENOMEM;
		return NULL;
	}
	u->crontab = NULL;

	/* 
	 * init environment.  this will be copied/augmented for each entry.
	 */
	if ((envp = env_init()) == NULL) {
		free(u->name);
		free(u);
		return NULL;
	}

	/*
	 * load the crontab
	 */
	while ((status = load_env(envstr, file)) >= OK) {
		switch (status) {
		case ERR:
			free_user(u);
			u = NULL;
			goto done;
		case FALSE:
			User_name = u->name;    /* for log_error */
			e = load_entry(file, log_error, pw, envp);
			if (e) {
				e->next = u->crontab;
				u->crontab = e;
			}
			break;
		case TRUE:
			if ((tenvp = env_set(envp, envstr))) {
				envp = tenvp;
			} else {
				free_user(u);
				u = NULL;
				goto done;
			}
			break;
		}
	}

 done:
	env_free(envp);
	fclose(file);
	Debug(DPARS, ("...load_user() done\n"))
	return u;
}