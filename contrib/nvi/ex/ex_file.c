
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
static const char sccsid[] = "@(#)ex_file.c	10.12 (Berkeley) 7/12/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"

/*
 * ex_file -- :f[ile] [name]
 *	Change the file's name and display the status line.
 *
 * PUBLIC: int ex_file __P((SCR *, EXCMD *));
 */
int
ex_file(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	CHAR_T *p;
	FREF *frp;

	NEEDFILE(sp, cmdp);

	switch (cmdp->argc) {
	case 0:
		break;
	case 1:
		frp = sp->frp;

		/* Make sure can allocate enough space. */
		if ((p = v_strdup(sp,
		    cmdp->argv[0]->bp, cmdp->argv[0]->len)) == NULL)
			return (1);

		/* If already have a file name, it becomes the alternate. */
		if (!F_ISSET(frp, FR_TMPFILE))
			set_alt_name(sp, frp->name);

		/* Free the previous name. */
		free(frp->name);
		frp->name = p;

		/*
		 * The file has a real name, it's no longer a temporary,
		 * clear the temporary file flags.
		 */
		F_CLR(frp, FR_TMPEXIT | FR_TMPFILE);

		/* Have to force a write if the file exists, next time. */
		F_SET(frp, FR_NAMECHANGE);

		/* Notify the screen. */
		(void)sp->gp->scr_rename(sp, sp->frp->name, 1);
		break;
	default:
		abort();
	}
	msgq_status(sp, sp->lno, MSTAT_SHOWLAST);
	return (0);
}