
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
static char sccsid[] = "@(#)lisp.c	8.3 (Berkeley) 4/2/94";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "ctags.h"

/*
 * lisp tag functions
 * just look for (def or (DEF
 */
void
l_entries(void)
{
	int	special;
	char	*cp;
	char	savedc;
	char	tok[MAXTOKEN];

	for (;;) {
		lineftell = ftell(inf);
		if (!fgets(lbuf, sizeof(lbuf), inf))
			return;
		++lineno;
		lbp = lbuf;
		if (!cicmp("(def"))
			continue;
		special = NO;
		switch(*lbp | ' ') {
		case 'm':
			if (cicmp("method"))
				special = YES;
			break;
		case 'w':
			if (cicmp("wrapper") || cicmp("whopper"))
				special = YES;
		}
		for (; !isspace(*lbp); ++lbp)
			continue;
		for (; isspace(*lbp); ++lbp)
			continue;
		for (cp = lbp; *cp && *cp != '\n'; ++cp)
			continue;
		*cp = EOS;
		if (special) {
			if (!(cp = strchr(lbp, ')')))
				continue;
			for (; cp >= lbp && *cp != ':'; --cp)
				continue;
			if (cp < lbp)
				continue;
			lbp = cp;
			for (; *cp && *cp != ')' && *cp != ' '; ++cp)
				continue;
		}
		else
			for (cp = lbp + 1;
			    *cp && *cp != '(' && *cp != ' '; ++cp)
				continue;
		savedc = *cp;
		*cp = EOS;
		(void)strlcpy(tok, lbp, sizeof(tok));	/* possible trunc */
		*cp = savedc;
		getline();
		pfnote(tok, lineno);
	}
	/*NOTREACHED*/
}