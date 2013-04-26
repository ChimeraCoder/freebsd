
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
static const char sccsid[] = "@(#)ex_equal.c	10.10 (Berkeley) 3/6/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <limits.h>
#include <stdio.h>

#include "../common/common.h"

/*
 * ex_equal -- :address =
 *
 * PUBLIC: int ex_equal __P((SCR *, EXCMD *));
 */
int
ex_equal(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	recno_t lno;

	NEEDFILE(sp, cmdp);

	/*
	 * Print out the line number matching the specified address,
	 * or the number of the last line in the file if no address
	 * specified.
	 *
	 * !!!
	 * Historically, ":0=" displayed 0, and ":=" or ":1=" in an
	 * empty file displayed 1.  Until somebody complains loudly,
	 * we're going to do it right.  The tables in excmd.c permit
	 * lno to get away with any address from 0 to the end of the
	 * file, which, in an empty file, is 0.
	 */
	if (F_ISSET(cmdp, E_ADDR_DEF)) {
		if (db_last(sp, &lno))
			return (1);
	} else
		lno = cmdp->addr1.lno;

	(void)ex_printf(sp, "%ld\n", lno);
	return (0);
}