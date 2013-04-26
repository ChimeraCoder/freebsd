
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
static const char sccsid[] = "@(#)ex_mark.c	10.8 (Berkeley) 3/6/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <limits.h>
#include <stdio.h>

#include "../common/common.h"

/*
 * ex_mark -- :mark char
 *	      :k char
 *	Mark lines.
 *
 *
 * PUBLIC: int ex_mark __P((SCR *, EXCMD *));
 */
int
ex_mark(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	NEEDFILE(sp, cmdp);

	if (cmdp->argv[0]->len != 1) {
		msgq(sp, M_ERR, "136|Mark names must be a single character");
		return (1);
	}
	return (mark_set(sp, cmdp->argv[0]->bp[0], &cmdp->addr1, 1));
}