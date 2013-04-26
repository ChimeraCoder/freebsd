
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

#include <pwd.h>
#include <grp.h>
#include <libutil.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

#include "pwupd.h"

static FILE * pwd_fp = NULL;

void
vendpwent(void)
{
	if (pwd_fp != NULL) {
		fclose(pwd_fp);
		pwd_fp = NULL;
	}
}

void
vsetpwent(void)
{
	vendpwent();
}

static struct passwd *
vnextpwent(char const *nam, uid_t uid, int doclose)
{
	struct passwd *pw;
	char *line;
	size_t linecap;
	ssize_t linelen;

	pw = NULL;
	line = NULL;
	linecap = 0;
	linelen = 0;

	if (pwd_fp != NULL || (pwd_fp = fopen(getpwpath(_MASTERPASSWD), "r")) != NULL) {
		while ((linelen = getline(&line, &linecap, pwd_fp)) > 0) {
			/* Skip comments and empty lines */
			if (*line == '\n' || *line == '#')
				continue;
			/* trim latest \n */
			if (line[linelen - 1 ] == '\n')
				line[linelen - 1] = '\0';
			pw = pw_scan(line, PWSCAN_MASTER);
			if (uid != (uid_t)-1) {
				if (uid == pw->pw_uid)
					break;
			} else if (nam != NULL) {
				if (strcmp(nam, pw->pw_name) == 0)
					break;
			} else
				break;
			free(pw);
			pw = NULL;
		}
		if (doclose)
			vendpwent();
	}
	free(line);

	return (pw);
}

struct passwd *
vgetpwent(void)
{
  return vnextpwent(NULL, -1, 0);
}

struct passwd *
vgetpwuid(uid_t uid)
{
  return vnextpwent(NULL, uid, 1);
}

struct passwd *
vgetpwnam(const char * nam)
{
  return vnextpwent(nam, -1, 1);
}


static FILE * grp_fp = NULL;

void
vendgrent(void)
{
	if (grp_fp != NULL) {
		fclose(grp_fp);
		grp_fp = NULL;
	}
}

RET_SETGRENT
vsetgrent(void)
{
	vendgrent();
#if defined(__FreeBSD__)
	return 0;
#endif
}

static struct group *
vnextgrent(char const *nam, gid_t gid, int doclose)
{
	struct group *gr;
	char *line;
	size_t linecap;
	ssize_t linelen;

	gr = NULL;
	line = NULL;
	linecap = 0;
	linelen = 0;

	if (grp_fp != NULL || (grp_fp = fopen(getgrpath(_GROUP), "r")) != NULL) {
		while ((linelen = getline(&line, &linecap, grp_fp)) > 0) {
			/* Skip comments and empty lines */
			if (*line == '\n' || *line == '#')
				continue;
			/* trim latest \n */
			if (line[linelen - 1 ] == '\n')
				line[linelen - 1] = '\0';
			gr = gr_scan(line);
			if (gid != (gid_t)-1) {
				if (gid == gr->gr_gid)
					break;
			} else if (nam != NULL) {
				if (strcmp(nam, gr->gr_name) == 0)
					break;
			} else
				break;
			free(gr);
			gr = NULL;
		}
		if (doclose)
			vendgrent();
	}
	free(line);

	return (gr);
}

struct group *
vgetgrent(void)
{
  return vnextgrent(NULL, -1, 0);
}


struct group *
vgetgrgid(gid_t gid)
{
  return vnextgrent(NULL, gid, 1);
}

struct group *
vgetgrnam(const char * nam)
{
  return vnextgrent(nam, -1, 1);
}