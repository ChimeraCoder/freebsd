
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

#include <config/config.h>

#include <bsm/libbsm.h>

#include <string.h>
#ifdef HAVE_PTHREAD_MUTEX_LOCK
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifndef HAVE_STRLCPY
#include <compat/strlcpy.h>
#endif

/*
 * Parse the contents of the audit_user file into au_user_ent structures.
 */

static FILE		*fp = NULL;
static char		 linestr[AU_LINE_MAX];
static const char	*user_delim = ":";

#ifdef HAVE_PTHREAD_MUTEX_LOCK
static pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Parse one line from the audit_user file into the au_user_ent structure.
 */
static struct au_user_ent *
userfromstr(char *str, struct au_user_ent *u)
{
	char *username, *always, *never;
	char *last;

	username = strtok_r(str, user_delim, &last);
	always = strtok_r(NULL, user_delim, &last);
	never = strtok_r(NULL, user_delim, &last);

	if ((username == NULL) || (always == NULL) || (never == NULL))
		return (NULL);

	if (strlen(username) >= AU_USER_NAME_MAX)
		return (NULL);

	strlcpy(u->au_name, username, AU_USER_NAME_MAX);
	if (getauditflagsbin(always, &(u->au_always)) == -1)
		return (NULL);

	if (getauditflagsbin(never, &(u->au_never)) == -1)
		return (NULL);

	return (u);
}

/*
 * Rewind to beginning of the file
 */
static void
setauuser_locked(void)
{

	if (fp != NULL)
		fseek(fp, 0, SEEK_SET);
}

void
setauuser(void)
{

#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_lock(&mutex);
#endif
	setauuser_locked();
#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_unlock(&mutex);
#endif
}

/*
 * Close the file descriptor
 */
void
endauuser(void)
{

#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_lock(&mutex);
#endif
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_unlock(&mutex);
#endif
}

/*
 * Enumerate the au_user_ent structures from the file
 */
static struct au_user_ent *
getauuserent_r_locked(struct au_user_ent *u)
{
	char *nl;

	if ((fp == NULL) && ((fp = fopen(AUDIT_USER_FILE, "r")) == NULL))
		return (NULL);

	while (1) {
		if (fgets(linestr, AU_LINE_MAX, fp) == NULL)
			return (NULL);

		/* Remove new lines. */
		if ((nl = strrchr(linestr, '\n')) != NULL)
			*nl = '\0';

		/* Skip comments. */
		if (linestr[0] == '#')
			continue;

		/* Get the next structure. */
		if (userfromstr(linestr, u) == NULL)
			return (NULL);
		break;
	}

	return (u);
}

struct au_user_ent *
getauuserent_r(struct au_user_ent *u)
{
	struct au_user_ent *up;

#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_lock(&mutex);
#endif
	up = getauuserent_r_locked(u);
#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_unlock(&mutex);
#endif
	return (up);
}

struct au_user_ent *
getauuserent(void)
{
	static char user_ent_name[AU_USER_NAME_MAX];
	static struct au_user_ent u;

	bzero(&u, sizeof(u));
	bzero(user_ent_name, sizeof(user_ent_name));
	u.au_name = user_ent_name;

	return (getauuserent_r(&u));
}

/*
 * Find a au_user_ent structure matching the given user name.
 */
struct au_user_ent *
getauusernam_r(struct au_user_ent *u, const char *name)
{
	struct au_user_ent *up;

	if (name == NULL)
		return (NULL);

#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_lock(&mutex);
#endif

	setauuser_locked();
	while ((up = getauuserent_r_locked(u)) != NULL) {
		if (strcmp(name, u->au_name) == 0) {
#ifdef HAVE_PTHREAD_MUTEX_LOCK
			pthread_mutex_unlock(&mutex);
#endif
			return (up);
		}
	}

#ifdef HAVE_PTHREAD_MUTEX_LOCK
	pthread_mutex_unlock(&mutex);
#endif
	return (NULL);

}

struct au_user_ent *
getauusernam(const char *name)
{
	static char user_ent_name[AU_USER_NAME_MAX];
	static struct au_user_ent u;

	bzero(&u, sizeof(u));
	bzero(user_ent_name, sizeof(user_ent_name));
	u.au_name = user_ent_name;

	return (getauusernam_r(&u, name));
}

/*
 * Read the default system wide audit classes from audit_control, combine with
 * the per-user audit class and update the binary preselection mask.
 */
int
au_user_mask(char *username, au_mask_t *mask_p)
{
	char auditstring[MAX_AUDITSTRING_LEN + 1];
	char user_ent_name[AU_USER_NAME_MAX];
	struct au_user_ent u, *up;

	bzero(&u, sizeof(u));
	bzero(user_ent_name, sizeof(user_ent_name));
	u.au_name = user_ent_name;

	/* Get user mask. */
	if ((up = getauusernam_r(&u, username)) != NULL) {
		if (-1 == getfauditflags(&up->au_always, &up->au_never,
		    mask_p))
			return (-1);
		return (0);
	}

	/* Read the default system mask. */
	if (getacflg(auditstring, MAX_AUDITSTRING_LEN) == 0) {
		if (-1 == getauditflagsbin(auditstring, mask_p))
			return (-1);
		return (0);
	}

	/* No masks defined. */
	return (-1);
}

/*
 * Generate the process audit state by combining the audit masks passed as
 * parameters with the system audit masks.
 */
int
getfauditflags(au_mask_t *usremask, au_mask_t *usrdmask, au_mask_t *lastmask)
{
	char auditstring[MAX_AUDITSTRING_LEN + 1];

	if ((usremask == NULL) || (usrdmask == NULL) || (lastmask == NULL))
		return (-1);

	lastmask->am_success = 0;
	lastmask->am_failure = 0;

	/* Get the system mask. */
	if (getacflg(auditstring, MAX_AUDITSTRING_LEN) == 0) {
		if (getauditflagsbin(auditstring, lastmask) != 0)
			return (-1);
	}

	ADDMASK(lastmask, usremask);
	SUBMASK(lastmask, usrdmask);

	return (0);
}