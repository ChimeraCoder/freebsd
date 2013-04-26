
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

/* OPENBSD ORIGINAL: lib/libc/gen/pwcache.c */

#include "includes.h"

#include <sys/types.h>

#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	NCACHE	64			/* power of 2 */
#define	MASK	(NCACHE - 1)		/* bits to store with */

#ifndef HAVE_USER_FROM_UID
char *
user_from_uid(uid_t uid, int nouser)
{
	static struct ncache {
		uid_t	uid;
		char	*name;
	} c_uid[NCACHE];
	static int pwopen;
	static char nbuf[15];		/* 32 bits == 10 digits */
	struct passwd *pw;
	struct ncache *cp;

	cp = c_uid + (uid & MASK);
	if (cp->uid != uid || cp->name == NULL) {
		if (pwopen == 0) {
#ifdef HAVE_SETPASSENT
			setpassent(1);
#endif
			pwopen = 1;
		}
		if ((pw = getpwuid(uid)) == NULL) {
			if (nouser)
				return (NULL);
			(void)snprintf(nbuf, sizeof(nbuf), "%u", uid);
		}
		cp->uid = uid;
		if (cp->name != NULL)
			free(cp->name);
		cp->name = strdup(pw ? pw->pw_name : nbuf);
	}
	return (cp->name);
}
#endif

#ifndef HAVE_GROUP_FROM_GID
char *
group_from_gid(gid_t gid, int nogroup)
{
	static struct ncache {
		gid_t	gid;
		char	*name;
	} c_gid[NCACHE];
	static int gropen;
	static char nbuf[15];		/* 32 bits == 10 digits */
	struct group *gr;
	struct ncache *cp;

	cp = c_gid + (gid & MASK);
	if (cp->gid != gid || cp->name == NULL) {
		if (gropen == 0) {
#ifdef HAVE_SETGROUPENT
			setgroupent(1);
#endif
			gropen = 1;
		}
		if ((gr = getgrgid(gid)) == NULL) {
			if (nogroup)
				return (NULL);
			(void)snprintf(nbuf, sizeof(nbuf), "%u", gid);
		}
		cp->gid = gid;
		if (cp->name != NULL)
			free(cp->name);
		cp->name = strdup(gr ? gr->gr_name : nbuf);
	}
	return (cp->name);
}
#endif