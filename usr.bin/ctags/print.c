
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
static char sccsid[] = "@(#)print.c	8.3 (Berkeley) 4/2/94";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "ctags.h"

/*
 * getline --
 *	get the line the token of interest occurred on,
 *	prepare it for printing.
 */
void
getline(void)
{
	long	saveftell;
	int	c;
	int	cnt;
	char	*cp;

	saveftell = ftell(inf);
	(void)fseek(inf, lineftell, L_SET);
	if (xflag)
		for (cp = lbuf; GETC(!=, EOF) && c != '\n'; *cp++ = c)
			continue;
	/*
	 * do all processing here, so we don't step through the
	 * line more than once; means you don't call this routine
	 * unless you're sure you've got a keeper.
	 */
	else for (cnt = 0, cp = lbuf; GETC(!=, EOF) && cnt < ENDLINE; ++cnt) {
		if (c == '\\') {		/* backslashes */
			if (cnt > ENDLINE - 2)
				break;
			*cp++ = '\\'; *cp++ = '\\';
			++cnt;
		}
		else if (c == (int)searchar) {	/* search character */
			if (cnt > ENDLINE - 2)
				break;
			*cp++ = '\\'; *cp++ = c;
			++cnt;
		}
		else if (c == '\n') {	/* end of keep */
			*cp++ = '$';		/* can find whole line */
			break;
		}
		else
			*cp++ = c;
	}
	*cp = EOS;
	(void)fseek(inf, saveftell, L_SET);
}

/*
 * put_entries --
 *	write out the tags
 */
void
put_entries(NODE *node)
{

	if (node->left)
		put_entries(node->left);
	if (vflag)
		printf("%s %s %d\n",
		    node->entry, node->file, (node->lno + 63) / 64);
	else if (xflag)
		printf("%-16s %4d %-16s %s\n",
		    node->entry, node->lno, node->file, node->pat);
	else
		fprintf(outf, "%s\t%s\t%c^%s%c\n",
		    node->entry, node->file, searchar, node->pat, searchar);
	if (node->right)
		put_entries(node->right);
}