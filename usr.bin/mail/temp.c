
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
static char sccsid[] = "@(#)temp.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "rcv.h"
#include "extern.h"

/*
 * Mail -- a mail program
 *
 * Give names to all the temporary files that we will need.
 */

char	*tmpdir;

void
tinit(void)
{
	char *cp;

	if ((tmpdir = getenv("TMPDIR")) == NULL || *tmpdir == '\0')
		tmpdir = _PATH_TMP;
	if ((tmpdir = strdup(tmpdir)) == NULL)
		errx(1, "Out of memory");
	/* Strip trailing '/' if necessary */
	cp = tmpdir + strlen(tmpdir) - 1;
	while (cp > tmpdir && *cp == '/') {
		*cp = '\0';
		cp--;
	}

	/*
	 * It's okay to call savestr in here because main will
	 * do a spreserve() after us.
	 */
	if (myname != NULL) {
		if (getuserid(myname) < 0)
			errx(1, "\"%s\" is not a user of this system", myname);
	} else {
		if ((cp = username()) == NULL) {
			myname = "ubluit";
			if (rcvmode)
				errx(1, "Who are you!?");
		} else
			myname = savestr(cp);
	}
	if ((cp = getenv("HOME")) == NULL || *cp == '\0' ||
	    strlen(cp) >= PATHSIZE)
		homedir = NULL;
	else
		homedir = savestr(cp);
	if (debug)
		printf("user = %s, homedir = %s\n", myname,
		    homedir ? homedir : "NONE");
}