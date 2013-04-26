
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
static const char sccsid[] = "@(#)ex_undo.c	10.6 (Berkeley) 3/6/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/common.h"

/*
 * ex_undo -- u
 *	Undo the last change.
 *
 * PUBLIC: int ex_undo __P((SCR *, EXCMD *));
 */
int
ex_undo(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	EXF *ep;
	MARK m;

	/*
	 * !!!
	 * Historic undo always set the previous context mark.
	 */
	m.lno = sp->lno;
	m.cno = sp->cno;
	if (mark_set(sp, ABSMARK1, &m, 1))
		return (1);

	/*
	 * !!!
	 * Multiple undo isn't available in ex, as there's no '.' command.
	 * Whether 'u' is undo or redo is toggled each time, unless there
	 * was a change since the last undo, in which case it's an undo.
	 */
	ep = sp->ep;
	if (!F_ISSET(ep, F_UNDO)) {
		F_SET(ep, F_UNDO);
		ep->lundo = FORWARD;
	}
	switch (ep->lundo) {
	case BACKWARD:
		if (log_forward(sp, &m))
			return (1);
		ep->lundo = FORWARD;
		break;
	case FORWARD:
		if (log_backward(sp, &m))
			return (1);
		ep->lundo = BACKWARD;
		break;
	case NOTSET:
		abort();
	}
	sp->lno = m.lno;
	sp->cno = m.cno;
	return (0);
}