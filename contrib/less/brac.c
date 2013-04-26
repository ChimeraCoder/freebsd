
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


/*
 * Routines to perform bracket matching functions.
 */

#include "less.h"
#include "position.h"

/*
 * Try to match the n-th open bracket 
 *  which appears in the top displayed line (forwdir),
 * or the n-th close bracket 
 *  which appears in the bottom displayed line (!forwdir).
 * The characters which serve as "open bracket" and 
 * "close bracket" are given.
 */
	public void
match_brac(obrac, cbrac, forwdir, n)
	register int obrac;
	register int cbrac;
	int forwdir;
	int n;
{
	register int c;
	register int nest;
	POSITION pos;
	int (*chget)();

	extern int ch_forw_get(), ch_back_get();

	/*
	 * Seek to the line containing the open bracket.
	 * This is either the top or bottom line on the screen,
	 * depending on the type of bracket.
	 */
	pos = position((forwdir) ? TOP : BOTTOM);
	if (pos == NULL_POSITION || ch_seek(pos))
	{
		if (forwdir)
			error("Nothing in top line", NULL_PARG);
		else
			error("Nothing in bottom line", NULL_PARG);
		return;
	}

	/*
	 * Look thru the line to find the open bracket to match.
	 */
	do
	{
		if ((c = ch_forw_get()) == '\n' || c == EOI)
		{
			if (forwdir)
				error("No bracket in top line", NULL_PARG);
			else
				error("No bracket in bottom line", NULL_PARG);
			return;
		}
	} while (c != obrac || --n > 0);

	/*
	 * Position the file just "after" the open bracket
	 * (in the direction in which we will be searching).
	 * If searching forward, we are already after the bracket.
	 * If searching backward, skip back over the open bracket.
	 */
	if (!forwdir)
		(void) ch_back_get();

	/*
	 * Search the file for the matching bracket.
	 */
	chget = (forwdir) ? ch_forw_get : ch_back_get;
	nest = 0;
	while ((c = (*chget)()) != EOI)
	{
		if (c == obrac)
			nest++;
		else if (c == cbrac && --nest < 0)
		{
			/*
			 * Found the matching bracket.
			 * If searching backward, put it on the top line.
			 * If searching forward, put it on the bottom line.
			 */
			jump_line_loc(ch_tell(), forwdir ? -1 : 1);
			return;
		}
	}
	error("No matching bracket", NULL_PARG);
}