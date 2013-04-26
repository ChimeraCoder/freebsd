
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
static const char sccsid[] = "@(#)wrterm.c	8.1 (Berkeley) 6/9/93";
#endif

#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

#include "extern.h"

/*
 * Output termcap entry to stdout, quoting characters that would give the
 * shell problems and omitting empty fields.
 */
void
wrtermcap(char *bp)
{
	register int ch;
	register char *p;
	char *t, *sep;

	/* Find the end of the terminal names. */
	if ((t = strchr(bp, ':')) == NULL)
		errx(1, "termcap names not colon terminated");
	*t++ = '\0';

	/* Output terminal names that don't have whitespace. */
	sep = strdup("");
	while ((p = strsep(&bp, "|")) != NULL)
		if (*p != '\0' && strpbrk(p, " \t") == NULL) {
			(void)printf("%s%s", sep, p);
			sep = strdup("|");
		}
	(void)putchar(':');

	/*
	 * Output fields, transforming any dangerous characters.  Skip
	 * empty fields or fields containing only whitespace.
	 */
	while ((p = strsep(&t, ":")) != NULL) {
		while ((ch = *p) != '\0' && isspace(ch))
			++p;
		if (ch == '\0')
			continue;
		while ((ch = *p++) != '\0')
			switch(ch) {
			case '\033':
				(void)printf("\\E");
			case  ' ':		/* No spaces. */
				(void)printf("\\040");
				break;
			case '!':		/* No csh history chars. */
				(void)printf("\\041");
				break;
			case ',':		/* No csh history chars. */
				(void)printf("\\054");
				break;
			case '"':		/* No quotes. */
				(void)printf("\\042");
				break;
			case '\'':		/* No quotes. */
				(void)printf("\\047");
				break;
			case '`':		/* No quotes. */
				(void)printf("\\140");
				break;
			case '\\':		/* Anything following is OK. */
			case '^':
				(void)putchar(ch);
				if ((ch = *p++) == '\0')
					break;
				/* FALLTHROUGH */
			default:
				(void)putchar(ch);
		}
		(void)putchar(':');
	}
}