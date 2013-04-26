
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
 ****************************************************************************/

/*
 *	nl.c
 *
 *	Routines:
 *		nl()
 *		nonl()
 *
 */

#include <curses.priv.h>

MODULE_ID("$Id: lib_nl.c,v 1.8 2000/12/10 02:43:27 tom Exp $")

#ifdef __EMX__
#include <io.h>
#endif

NCURSES_EXPORT(int)
nl(void)
{
    T((T_CALLED("nl()")));

    SP->_nl = TRUE;

#ifdef __EMX__
    _nc_flush();
    _fsetmode(NC_OUTPUT, "t");
#endif

    returnCode(OK);
}

NCURSES_EXPORT(int)
nonl(void)
{
    T((T_CALLED("nonl()")));

    SP->_nl = FALSE;

#ifdef __EMX__
    _nc_flush();
    _fsetmode(NC_OUTPUT, "b");
#endif

    returnCode(OK);
}