
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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1995                    *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/* p_top.c
 * Place a panel on top of the stack.
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_top.c,v 1.5 2005/02/19 16:42:23 tom Exp $")

NCURSES_EXPORT(int)
top_panel(PANEL * pan)
{
  T((T_CALLED("top_panel(%p)"), pan));
  returnCode(show_panel(pan));
}