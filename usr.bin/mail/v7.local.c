
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
static char sccsid[] = "@(#)v7.local.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Mail -- a mail program
 *
 * Version 7
 *
 * Local routines that are installation dependent.
 */

#include "rcv.h"
#include <fcntl.h>
#include "extern.h"

/*
 * Locate the user's mailbox file (ie, the place where new, unread
 * mail is queued).
 */
void
findmail(char *user, char *buf, int buflen)
{
	char *tmp = getenv("MAIL");

	if (tmp == NULL)
		(void)snprintf(buf, buflen, "%s/%s", _PATH_MAILDIR, user);
	else
		(void)strlcpy(buf, tmp, buflen);
}

/*
 * Get rid of the queued mail.
 */
void
demail(void)
{

	if (value("keep") != NULL || rm(mailname) < 0)
		(void)close(open(mailname, O_CREAT | O_TRUNC | O_WRONLY, 0600));
}

/*
 * Discover user login name.
 */
char *
username(void)
{
	char *np;
	uid_t uid;

	if ((np = getenv("USER")) != NULL)
		return (np);
	if ((np = getenv("LOGNAME")) != NULL)
		return (np);
	if ((np = getname(uid = getuid())) != NULL)
		return (np);
	printf("Cannot associate a name with uid %u\n", (unsigned)uid);
	return (NULL);
}