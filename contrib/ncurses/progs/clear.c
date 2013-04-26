
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

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 * clear.c --  clears the terminal's screen
 */

#define USE_LIBTINFO
#include <progs.priv.h>

MODULE_ID("$Id: clear.c,v 1.11 2007/10/13 22:16:02 tom Exp $")

static int
putch(int c)
{
    return putchar(c);
}

int
main(
	int argc GCC_UNUSED,
	char *argv[]GCC_UNUSED)
{
    setupterm((char *) 0, STDOUT_FILENO, (int *) 0);
    ExitProgram((tputs(clear_screen, lines > 0 ? lines : 1, putch) == ERR)
		? EXIT_FAILURE
		: EXIT_SUCCESS);
}