
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
static const char sccsid[] = "@(#)v_right.c	10.7 (Berkeley) 3/6/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <limits.h>
#include <stdio.h>

#include "../common/common.h"
#include "vi.h"

/*
 * v_right -- [count]' ', [count]l
 *	Move right by columns.
 *
 * PUBLIC: int v_right __P((SCR *, VICMD *));
 */
int
v_right(sp, vp)
	SCR *sp;
	VICMD *vp;
{
	size_t len;
	int isempty;

	if (db_eget(sp, vp->m_start.lno, NULL, &len, &isempty)) {
		if (isempty)
			goto eol;
		return (1);
	}

	/* It's always illegal to move right on empty lines. */
	if (len == 0) {
eol:		v_eol(sp, NULL);
		return (1);
	}

	/*
	 * Non-motion commands move to the end of the range.  Delete and
	 * yank stay at the start.  Ignore others.  Adjust the end of the
	 * range for motion commands.
	 *
	 * !!!
	 * Historically, "[cdsy]l" worked at the end of a line.  Also,
	 * EOL is a count sink.
	 */
	vp->m_stop.cno = vp->m_start.cno +
	    (F_ISSET(vp, VC_C1SET) ? vp->count : 1);
	if (vp->m_start.cno == len - 1 && !ISMOTION(vp)) {
		v_eol(sp, NULL);
		return (1);
	}
	if (vp->m_stop.cno >= len) {
		vp->m_stop.cno = len - 1;
		vp->m_final = ISMOTION(vp) ? vp->m_start : vp->m_stop;
	} else if (ISMOTION(vp)) {
		--vp->m_stop.cno;
		vp->m_final = vp->m_start;
	} else
		vp->m_final = vp->m_stop;
	return (0);
}

/*
 * v_dollar -- [count]$
 *	Move to the last column.
 *
 * PUBLIC: int v_dollar __P((SCR *, VICMD *));
 */
int
v_dollar(sp, vp)
	SCR *sp;
	VICMD *vp;
{
	size_t len;
	int isempty;

	/*
	 * !!!
	 * A count moves down count - 1 rows, so, "3$" is the same as "2j$".
	 */
	if ((F_ISSET(vp, VC_C1SET) ? vp->count : 1) != 1) {
		/*
		 * !!!
		 * Historically, if the $ is a motion, and deleting from
		 * at or before the first non-blank of the line, it's a
		 * line motion, and the line motion flag is set.
		 */
		vp->m_stop.cno = 0;
		if (nonblank(sp, vp->m_start.lno, &vp->m_stop.cno))
			return (1);
		if (ISMOTION(vp) && vp->m_start.cno <= vp->m_stop.cno)
			F_SET(vp, VM_LMODE);

		--vp->count;
		if (v_down(sp, vp))
			return (1);
	}

	/*
	 * !!!
	 * Historically, it was illegal to use $ as a motion command on
	 * an empty line.  Unfortunately, even though C was historically
	 * aliased to c$, it (and not c$) was special cased to work on
	 * empty lines.  Since we alias C to c$ too, we have a problem.
	 * To fix it, we let c$ go through, on the assumption that it's
	 * not a problem for it to work.
	 */
	if (db_eget(sp, vp->m_stop.lno, NULL, &len, &isempty)) {
		if (!isempty)
			return (1);
		len = 0;
	}

	if (len == 0) {
		if (ISMOTION(vp) && !ISCMD(vp->rkp, 'c')) {
			v_eol(sp, NULL);
			return (1);
		}
		return (0);
	}

	/*
	 * Non-motion commands move to the end of the range.  Delete
	 * and yank stay at the start.  Ignore others.
	 */
	vp->m_stop.cno = len ? len - 1 : 0;
	vp->m_final = ISMOTION(vp) ? vp->m_start : vp->m_stop;
	return (0);
}