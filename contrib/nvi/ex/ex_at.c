
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
static const char sccsid[] = "@(#)ex_at.c	10.12 (Berkeley) 9/15/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"

/*
 * ex_at -- :@[@ | buffer]
 *	    :*[* | buffer]
 *
 *	Execute the contents of the buffer.
 *
 * PUBLIC: int ex_at __P((SCR *, EXCMD *));
 */
int
ex_at(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	CB *cbp;
	CHAR_T name;
	EXCMD *ecp;
	RANGE *rp;
	TEXT *tp;
	size_t len;
	char *p;

	/*
	 * !!!
	 * Historically, [@*]<carriage-return> and [@*][@*] executed the most
	 * recently executed buffer in ex mode.
	 */
	name = FL_ISSET(cmdp->iflags, E_C_BUFFER) ? cmdp->buffer : '@';
	if (name == '@' || name == '*') {
		if (!F_ISSET(sp, SC_AT_SET)) {
			ex_emsg(sp, NULL, EXM_NOPREVBUF);
			return (1);
		}
		name = sp->at_lbuf;
	}
	sp->at_lbuf = name;
	F_SET(sp, SC_AT_SET);

	CBNAME(sp, cbp, name);
	if (cbp == NULL) {
		ex_emsg(sp, KEY_NAME(sp, name), EXM_EMPTYBUF);
		return (1);
	}

	/*
	 * !!!
	 * Historically the @ command took a range of lines, and the @ buffer
	 * was executed once per line.  The historic vi could be trashed by
	 * this because it didn't notice if the underlying file changed, or,
	 * for that matter, if there were no more lines on which to operate.
	 * For example, take a 10 line file, load "%delete" into a buffer,
	 * and enter :8,10@<buffer>.
	 *
	 * The solution is a bit tricky.  If the user specifies a range, take
	 * the same approach as for global commands, and discard the command
	 * if exit or switch to a new file/screen.  If the user doesn't specify
	 * the  range, continue to execute after a file/screen switch, which
	 * means @ buffers are still useful in a multi-screen environment.
	 */
	CALLOC_RET(sp, ecp, EXCMD *, 1, sizeof(EXCMD));
	CIRCLEQ_INIT(&ecp->rq);
	CALLOC_RET(sp, rp, RANGE *, 1, sizeof(RANGE));
	rp->start = cmdp->addr1.lno;
	if (F_ISSET(cmdp, E_ADDR_DEF)) {
		rp->stop = rp->start;
		FL_SET(ecp->agv_flags, AGV_AT_NORANGE);
	} else {
		rp->stop = cmdp->addr2.lno;
		FL_SET(ecp->agv_flags, AGV_AT);
	}
	CIRCLEQ_INSERT_HEAD(&ecp->rq, rp, q);

	/*
	 * Buffers executed in ex mode or from the colon command line in vi
	 * were ex commands.  We can't push it on the terminal queue, since
	 * it has to be executed immediately, and we may be in the middle of
	 * an ex command already.  Push the command on the ex command stack.
	 * Build two copies of the command.  We need two copies because the
	 * ex parser may step on the command string when it's parsing it.
	 */
	for (len = 0, tp = cbp->textq.cqh_last;
	    tp != (void *)&cbp->textq; tp = tp->q.cqe_prev)
		len += tp->len + 1;

	MALLOC_RET(sp, ecp->cp, char *, len * 2);
	ecp->o_cp = ecp->cp;
	ecp->o_clen = len;
	ecp->cp[len] = '\0';

	/* Copy the buffer into the command space. */
	for (p = ecp->cp + len, tp = cbp->textq.cqh_last;
	    tp != (void *)&cbp->textq; tp = tp->q.cqe_prev) {
		memcpy(p, tp->lb, tp->len);
		p += tp->len;
		*p++ = '\n';
	}

	LIST_INSERT_HEAD(&sp->gp->ecq, ecp, q);
	return (0);
}