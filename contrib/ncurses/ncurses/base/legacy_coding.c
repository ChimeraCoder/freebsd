
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: legacy_coding.c,v 1.2 2005/12/17 23:38:17 tom Exp $")

NCURSES_EXPORT(int)
use_legacy_coding(int level)
{
    int result = ERR;

    T((T_CALLED("use_legacy_coding(%d)"), level));
    if (level >= 0 && level <= 2 && SP != 0) {
	result = SP->_legacy_coding;
	SP->_legacy_coding = level;
    }
    returnCode(result);
}