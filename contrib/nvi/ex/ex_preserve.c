
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
static const char sccsid[] = "@(#)ex_preserve.c	10.12 (Berkeley) 4/27/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "../common/common.h"

/*
 * ex_preserve -- :pre[serve]
 *	Push the file to recovery.
 *
 * PUBLIC: int ex_preserve __P((SCR *, EXCMD *));
 */
int
ex_preserve(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	recno_t lno;

	NEEDFILE(sp, cmdp);

	if (!F_ISSET(sp->ep, F_RCV_ON)) {
		msgq(sp, M_ERR, "142|Preservation of this file not possible");
		return (1);
	}

	/* If recovery not initialized, do so. */
	if (F_ISSET(sp->ep, F_FIRSTMODIFY) && rcv_init(sp))
		return (1);

	/* Force the file to be read in, in case it hasn't yet. */
	if (db_last(sp, &lno))
		return (1);

	/* Sync to disk. */
	if (rcv_sync(sp, RCV_SNAPSHOT))
		return (1);

	msgq(sp, M_INFO, "143|File preserved");
	return (0);
}

/*
 * ex_recover -- :rec[over][!] file
 *	Recover the file.
 *
 * PUBLIC: int ex_recover __P((SCR *, EXCMD *));
 */
int
ex_recover(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	ARGS *ap;
	FREF *frp;

	ap = cmdp->argv[0];

	/* Set the alternate file name. */
	set_alt_name(sp, ap->bp);

	/*
	 * Check for modifications.  Autowrite did not historically
	 * affect :recover.
	 */
	if (file_m2(sp, FL_ISSET(cmdp->iflags, E_C_FORCE)))
		return (1);

	/* Get a file structure for the file. */
	if ((frp = file_add(sp, ap->bp)) == NULL)
		return (1);

	/* Set the recover bit. */
	F_SET(frp, FR_RECOVER);

	/* Switch files. */
	if (file_init(sp, frp, NULL, FS_SETALT |
	    (FL_ISSET(cmdp->iflags, E_C_FORCE) ? FS_FORCE : 0)))
		return (1);

	F_SET(sp, SC_FSWITCH);
	return (0);
}