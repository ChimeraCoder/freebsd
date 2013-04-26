
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
static const char sccsid[] = "@(#)v_xchar.c	10.9 (Berkeley) 10/23/96";
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
 * v_xchar -- [buffer] [count]x
 *	Deletes the character(s) on which the cursor sits.
 *
 * PUBLIC: int v_xchar __P((SCR *, VICMD *));
 */
int
v_xchar(sp, vp)
	SCR *sp;
	VICMD *vp;
{
	size_t len;
	int isempty;

	if (db_eget(sp, vp->m_start.lno, NULL, &len, &isempty)) {
		if (isempty)
			goto nodel;
		return (1);
	}
	if (len == 0) {
nodel:		msgq(sp, M_BERR, "206|No characters to delete");
		return (1);
	}

	/*
	 * Delete from the cursor toward the end of line, w/o moving the
	 * cursor.
	 *
	 * !!!
	 * Note, "2x" at EOL isn't the same as "xx" because the left movement
	 * of the cursor as part of the 'x' command isn't taken into account.
	 * Historically correct.
	 */
	if (F_ISSET(vp, VC_C1SET))
		vp->m_stop.cno += vp->count - 1;
	if (vp->m_stop.cno >= len - 1) {
		vp->m_stop.cno = len - 1;
		vp->m_final.cno = vp->m_start.cno ? vp->m_start.cno - 1 : 0;
	} else
		vp->m_final.cno = vp->m_start.cno;

	if (cut(sp,
	    F_ISSET(vp, VC_BUFFER) ? &vp->buffer : NULL,
	    &vp->m_start, &vp->m_stop, 0))
		return (1);
	return (del(sp, &vp->m_start, &vp->m_stop, 0));
}

/*
 * v_Xchar -- [buffer] [count]X
 *	Deletes the character(s) immediately before the current cursor
 *	position.
 *
 * PUBLIC: int v_Xchar __P((SCR *, VICMD *));
 */
int
v_Xchar(sp, vp)
	SCR *sp;
	VICMD *vp;
{
	u_long cnt;

	if (vp->m_start.cno == 0) {
		v_sol(sp);
		return (1);
	}

	cnt = F_ISSET(vp, VC_C1SET) ? vp->count : 1;
	if (cnt >= vp->m_start.cno)
		vp->m_start.cno = 0;
	else
		vp->m_start.cno -= cnt;
	--vp->m_stop.cno;
	vp->m_final.cno = vp->m_start.cno;

	if (cut(sp,
	    F_ISSET(vp, VC_BUFFER) ? &vp->buffer : NULL,
	    &vp->m_start, &vp->m_stop, 0))
		return (1);
	return (del(sp, &vp->m_start, &vp->m_stop, 0));
}