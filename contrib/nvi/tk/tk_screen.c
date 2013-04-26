
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
static const char sccsid[] = "@(#)tk_screen.c	8.9 (Berkeley) 5/24/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "../common/common.h"
#include "tki.h"

/*
 * tk_screen --
 *	Initialize/shutdown the Tcl/Tk screen.
 *
 * PUBLIC: int tk_screen __P((SCR *, u_int32_t));
 */
int
tk_screen(sp, flags)
	SCR *sp;
	u_int32_t flags;
{
	TK_PRIVATE *tkp;

	tkp = TKP(sp);

	/* See if we're already in the right mode. */
	if (LF_ISSET(SC_VI) && F_ISSET(sp, SC_SCR_VI))
		return (0);

	/* Ex isn't possible. */
	if (LF_ISSET(SC_EX))
		return (1);

	/* Initialize terminal based information. */
	if (tk_term_init(sp)) 
		return (1);

	/* Put up the first file name. */
	if (tk_rename(sp))
		return (1);

	F_SET(tkp, TK_SCR_VI_INIT);
	return (0);
}

/*
 * tk_quit --
 *	Shutdown the screens.
 *
 * PUBLIC: int tk_quit __P((GS *));
 */
int
tk_quit(gp)
	GS *gp;
{
	TK_PRIVATE *tkp;
	int rval;

	/* Clean up the terminal mappings. */
	rval = tk_term_end(gp);

	tkp = GTKP(gp);
	F_CLR(tkp, TK_SCR_VI_INIT);

	return (rval);
}