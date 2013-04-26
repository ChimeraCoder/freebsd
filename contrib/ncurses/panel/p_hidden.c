
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

/* p_hidden.c
 * Test whether or not panel is hidden
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_hidden.c,v 1.7 2005/02/19 16:39:17 tom Exp $")

NCURSES_EXPORT(int)
panel_hidden(const PANEL * pan)
{
  T((T_CALLED("panel_hidden(%p)"), pan));
  if (!pan)
    returnCode(ERR);
  returnCode(IS_LINKED(pan) ? FALSE : TRUE);
}