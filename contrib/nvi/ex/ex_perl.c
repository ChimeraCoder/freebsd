
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
static const char sccsid[] = "@(#)ex_perl.c	8.10 (Berkeley) 9/15/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "../common/common.h"

/* 
 * ex_perl -- :[line [,line]] perl [command]
 *	Run a command through the perl interpreter.
 *
 * ex_perldo -- :[line [,line]] perldo [command]
 *	Run a set of lines through the perl interpreter.
 *
 * PUBLIC: int ex_perl __P((SCR*, EXCMD *));
 */
int 
ex_perl(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
#ifdef HAVE_PERL_INTERP
	CHAR_T *p;
	size_t len;

	/* Skip leading white space. */
	if (cmdp->argc != 0)
		for (p = cmdp->argv[0]->bp,
		    len = cmdp->argv[0]->len; len > 0; --len, ++p)
			if (!isblank(*p))
				break;
	if (cmdp->argc == 0 || len == 0) {
		ex_emsg(sp, cmdp->cmd->usage, EXM_USAGE);
		return (1);
	}
	return (cmdp->cmd == &cmds[C_PERLCMD] ?
	    perl_ex_perl(sp, p, len, cmdp->addr1.lno, cmdp->addr2.lno) :
	    perl_ex_perldo(sp, p, len, cmdp->addr1.lno, cmdp->addr2.lno));
#else
	msgq(sp, M_ERR, "306|Vi was not loaded with a Perl interpreter");
	return (1);
#endif
}