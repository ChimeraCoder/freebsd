
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

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)ex_abbrev.c	10.7 (Berkeley) 3/6/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"
#include "../vi/vi.h"

/*
 * ex_abbr -- :abbreviate [key replacement]
 *	Create an abbreviation or display abbreviations.
 *
 * PUBLIC: int ex_abbr __P((SCR *, EXCMD *));
 */
int
ex_abbr(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	CHAR_T *p;
	size_t len;

	switch (cmdp->argc) {
	case 0:
		if (seq_dump(sp, SEQ_ABBREV, 0) == 0)
			msgq(sp, M_INFO, "105|No abbreviations to display");
		return (0);
	case 2:
		break;
	default:
		abort();
	}

	/*
	 * Check for illegal characters.
	 *
	 * !!!
	 * Another fun one, historically.  See vi/v_ntext.c:txt_abbrev() for
	 * details.  The bottom line is that all abbreviations have to end
	 * with a "word" character, because it's the transition from word to
	 * non-word characters that triggers the test for an abbreviation.  In
	 * addition, because of the way the test is done, there can't be any
	 * transitions from word to non-word character (or vice-versa) other
	 * than between the next-to-last and last characters of the string,
	 * and there can't be any <blank> characters.  Warn the user.
	 */
	if (!inword(cmdp->argv[0]->bp[cmdp->argv[0]->len - 1])) {
		msgq(sp, M_ERR,
		    "106|Abbreviations must end with a \"word\" character");
			return (1);
	}
	for (p = cmdp->argv[0]->bp; *p != '\0'; ++p)
		if (isblank(p[0])) {
			msgq(sp, M_ERR,
			    "107|Abbreviations may not contain tabs or spaces");
			return (1);
		}
	if (cmdp->argv[0]->len > 2)
		for (p = cmdp->argv[0]->bp,
		    len = cmdp->argv[0]->len - 2; len; --len, ++p)
			if (inword(p[0]) != inword(p[1])) {
				msgq(sp, M_ERR,
"108|Abbreviations may not mix word/non-word characters, except at the end");
				return (1);
			}

	if (seq_set(sp, NULL, 0, cmdp->argv[0]->bp, cmdp->argv[0]->len,
	    cmdp->argv[1]->bp, cmdp->argv[1]->len, SEQ_ABBREV, SEQ_USERDEF))
		return (1);

	F_SET(sp->gp, G_ABBREV);
	return (0);
}

/*
 * ex_unabbr -- :unabbreviate key
 *      Delete an abbreviation.
 *
 * PUBLIC: int ex_unabbr __P((SCR *, EXCMD *));
 */
int
ex_unabbr(sp, cmdp)
	SCR *sp;
        EXCMD *cmdp;
{
	ARGS *ap;

	ap = cmdp->argv[0];
	if (!F_ISSET(sp->gp, G_ABBREV) ||
	    seq_delete(sp, ap->bp, ap->len, SEQ_ABBREV)) {
		msgq_str(sp, M_ERR, ap->bp,
		    "109|\"%s\" is not an abbreviation");
		return (1);
	}
	return (0);
}