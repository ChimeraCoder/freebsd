
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#ifndef lint
static const char sccsid[] = "@(#)term.c	8.1 (Berkeley) 6/9/93";
#endif

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termcap.h>
#include <unistd.h>
#include <ttyent.h>
#include "extern.h"

static char tbuf[1024];		/* Termcap entry. */

const char *askuser(const char *);
char	*ttys(char *);

/*
 * Figure out what kind of terminal we're dealing with, and then read in
 * its termcap entry.
 */
const char *
get_termcap_entry(char *userarg, char **tcapbufp)
{
	struct ttyent *t;
	int rval;
	char *p, *ttypath;
	const char *ttype;

	if (userarg) {
		ttype = userarg;
		goto found;
	}

	/* Try the environment. */
	if ((ttype = getenv("TERM")))
		goto map;

	/* Try ttyname(3); check for dialup or other mapping. */
	if ((ttypath = ttyname(STDERR_FILENO))) {
		if ((p = strrchr(ttypath, '/')))
			++p;
		else
			p = ttypath;
		if ((t = getttynam(p))) {
			ttype = t->ty_type;
			goto map;
		}
	}

	/* If still undefined, use "unknown". */
	ttype = "unknown";

map:	ttype = mapped(ttype);

	/*
	 * If not a path, remove TERMCAP from the environment so we get a
	 * real entry from /etc/termcap.  This prevents us from being fooled
	 * by out of date stuff in the environment.
	 */
found:	if ((p = getenv("TERMCAP")) != NULL && *p != '/')
		unsetenv("TERMCAP");

	/*
	 * ttype now contains a pointer to the type of the terminal.
	 * If the first character is '?', ask the user.
	 */
	if (ttype[0] == '?') {
		if (ttype[1] != '\0')
			ttype = askuser(ttype + 1);
		else
			ttype = askuser(NULL);
	}

	/* Find the termcap entry.  If it doesn't exist, ask the user. */
	while ((rval = tgetent(tbuf, ttype)) == 0) {
		warnx("terminal type %s is unknown", ttype);
		ttype = askuser(NULL);
	}
	if (rval == -1)
		errx(1, "termcap: %s", strerror(errno ? errno : ENOENT));
	*tcapbufp = tbuf;
	return (ttype);
}

/* Prompt the user for a terminal type. */
const char *
askuser(const char *dflt)
{
	static char answer[256];
	char *p;

	/* We can get recalled; if so, don't continue uselessly. */
	if (feof(stdin) || ferror(stdin)) {
		(void)fprintf(stderr, "\n");
		exit(1);
	}
	for (;;) {
		if (dflt)
			(void)fprintf(stderr, "Terminal type? [%s] ", dflt);
		else
			(void)fprintf(stderr, "Terminal type? ");
		(void)fflush(stderr);

		if (fgets(answer, sizeof(answer), stdin) == NULL) {
			if (dflt == NULL) {
				(void)fprintf(stderr, "\n");
				exit(1);
			}
			return (dflt);
		}

		if ((p = strchr(answer, '\n')))
			*p = '\0';
		if (answer[0])
			return (answer);
		if (dflt != NULL)
			return (dflt);
	}
}