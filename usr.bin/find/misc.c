
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
static char sccsid[] = "@(#)misc.c	8.2 (Berkeley) 4/1/94";
#else
#endif
#endif /* not lint */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "find.h"

/*
 * brace_subst --
 *	Replace occurrences of {} in s1 with s2 and return the result string.
 */
void
brace_subst(char *orig, char **store, char *path, int len)
{
	int plen;
	char ch, *p;

	plen = strlen(path);
	for (p = *store; (ch = *orig) != '\0'; ++orig)
		if (ch == '{' && orig[1] == '}') {
			while ((p - *store) + plen > len)
				if (!(*store = realloc(*store, len *= 2)))
					err(1, NULL);
			memmove(p, path, plen);
			p += plen;
			++orig;
		} else
			*p++ = ch;
	*p = '\0';
}

/*
 * queryuser --
 *	print a message to standard error and then read input from standard
 *	input. If the input is an affirmative response (according to the
 *	current locale) then 1 is returned.
 */
int
queryuser(char *argv[])
{
	char *p, resp[256];

	(void)fprintf(stderr, "\"%s", *argv);
	while (*++argv)
		(void)fprintf(stderr, " %s", *argv);
	(void)fprintf(stderr, "\"? ");
	(void)fflush(stderr);

	if (fgets(resp, sizeof(resp), stdin) == NULL)
		*resp = '\0';
	if ((p = strchr(resp, '\n')) != NULL)
		*p = '\0';
	else {
		(void)fprintf(stderr, "\n");
		(void)fflush(stderr);
	}
        return (rpmatch(resp) == 1);
}