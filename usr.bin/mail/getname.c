
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
#if 0
static char sccsid[] = "@(#)getname.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "rcv.h"
#include <pwd.h>
#include "extern.h"

/* Getname / getuserid for those with hashed passwd data base). */

/*
 * Search the passwd file for a uid. Return name on success, NULL on failure.
 */
char *
getname(int uid)
{
	struct passwd *pw;

	if ((pw = getpwuid(uid)) == NULL)
		return (NULL);
	return (pw->pw_name);
}

/*
 * Convert the passed name to a user id and return it.  Return -1
 * on error.
 */
int
getuserid(char name[])
{
	struct passwd *pw;

	if ((pw = getpwnam(name)) == NULL)
		return (-1);
	return (pw->pw_uid);
}