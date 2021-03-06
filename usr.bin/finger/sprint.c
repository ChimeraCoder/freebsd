
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)sprint.c	8.3 (Berkeley) 4/28/95";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <db.h>
#include <err.h>
#include <langinfo.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <utmpx.h>
#include "finger.h"

static void	  stimeprint(WHERE *);

void
sflag_print(void)
{
	PERSON *pn;
	WHERE *w;
	int sflag, r, namelen;
	char p[80];
	PERSON *tmp;
	DBT data, key;
	struct tm *lc;

	if (d_first < 0)
		d_first = (*nl_langinfo(D_MD_ORDER) == 'd');
	/*
	 * short format --
	 *	login name
	 *	real name
	 *	terminal name (the XX of ttyXX)
	 *	if terminal writeable (add an '*' to the terminal name
	 *		if not)
	 *	if logged in show idle time and day logged in, else
	 *		show last login date and time.
	 *		If > 6 months, show year instead of time.
	 *	if (-o)
	 *		office location
	 *		office phone
	 *	else
	 *		remote host
	 */
#define	MAXREALNAME	16
#define MAXHOSTNAME     17      /* in reality, hosts are never longer than 16 */
	(void)printf("%-*s %-*s%s %s\n", MAXLOGNAME, "Login", MAXREALNAME,
	    "Name", " TTY      Idle  Login  Time  ", (gflag) ? "" :
	    oflag ? "Office  Phone" : "Where");

	for (sflag = R_FIRST;; sflag = R_NEXT) {
		r = (*db->seq)(db, &key, &data, sflag);
		if (r == -1)
			err(1, "db seq");
		if (r == 1)
			break;
		memmove(&tmp, data.data, sizeof tmp);
		pn = tmp;

		for (w = pn->whead; w != NULL; w = w->next) {
			namelen = MAXREALNAME;
			if (w->info == LOGGEDIN && !w->writable)
				--namelen;	/* leave space before `*' */
			(void)printf("%-*.*s %-*.*s", MAXLOGNAME, MAXLOGNAME,
				pn->name, MAXREALNAME, namelen,
				pn->realname ? pn->realname : "");
			if (!w->loginat) {
				(void)printf("  *     *   No logins   ");
				goto office;
			}
			(void)putchar(w->info == LOGGEDIN && !w->writable ?
			    '*' : ' ');
			if (*w->tty)
				(void)printf("%-7.7s ",
					     (strncmp(w->tty, "tty", 3)
					      && strncmp(w->tty, "cua", 3))
					     ? w->tty : w->tty + 3);
			else
				(void)printf("        ");
			if (w->info == LOGGEDIN) {
				stimeprint(w);
				(void)printf("  ");
			} else
				(void)printf("    *  ");
			lc = localtime(&w->loginat);
#define SECSPERDAY 86400
#define DAYSPERWEEK 7
#define DAYSPERNYEAR 365
			if (now - w->loginat < SECSPERDAY * (DAYSPERWEEK - 1)) {
				(void)strftime(p, sizeof(p), "%a", lc);
			} else {
				(void)strftime(p, sizeof(p),
					     d_first ? "%e %b" : "%b %e", lc);
			}
			(void)printf("%-6.6s", p);
			if (now - w->loginat >= SECSPERDAY * DAYSPERNYEAR / 2) {
				(void)strftime(p, sizeof(p), "%Y", lc);
			} else {
				(void)strftime(p, sizeof(p), "%R", lc);
			}
			(void)printf(" %-5.5s", p);
office:
			if (gflag)
				goto no_gecos;
			if (oflag) {
				if (pn->office)
					(void)printf(" %-7.7s", pn->office);
				else if (pn->officephone)
					(void)printf(" %-7.7s", " ");
				if (pn->officephone)
					(void)printf(" %-.9s",
					    prphone(pn->officephone));
			} else
				(void)printf(" %.*s", MAXHOSTNAME, w->host);
no_gecos:
			putchar('\n');
		}
	}
}

static void
stimeprint(WHERE *w)
{
	struct tm *delta;

	if (w->idletime == -1) {
		(void)printf("     ");
		return;
	}

	delta = gmtime(&w->idletime);
	if (!delta->tm_yday)
		if (!delta->tm_hour)
			if (!delta->tm_min)
				(void)printf("     ");
			else
				(void)printf("%5d", delta->tm_min);
		else
			(void)printf("%2d:%02d",
			    delta->tm_hour, delta->tm_min);
	else
		(void)printf("%4dd", delta->tm_yday);
}