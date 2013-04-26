
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
static const char sccsid[] = "@(#)ex_stop.c	10.10 (Berkeley) 3/6/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../common/common.h"

/*
 * ex_stop -- :stop[!]
 *	      :suspend[!]
 *	Suspend execution.
 *
 * PUBLIC: int ex_stop __P((SCR *, EXCMD *));
 */
int
ex_stop(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	int allowed;

	/* For some strange reason, the force flag turns off autowrite. */
	if (!FL_ISSET(cmdp->iflags, E_C_FORCE) && file_aw(sp, FS_ALL))
		return (1);

	if (sp->gp->scr_suspend(sp, &allowed))
		return (1);
	if (!allowed)
		ex_emsg(sp, NULL, EXM_NOSUSPEND);
	return (0);
}