
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

/* p_hide.c
 * Remove a panel from the stack
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_hide.c,v 1.9 2005/02/19 16:39:41 tom Exp $")

NCURSES_EXPORT(int)
hide_panel(register PANEL * pan)
{
  int err = OK;

  T((T_CALLED("hide_panel(%p)"), pan));
  if (!pan)
    returnCode(ERR);

  dBug(("--> hide_panel %s", USER_PTR(pan->user)));
  dStack("<u%d>", 1, pan);

  HIDE_PANEL(pan, err, ERR);

  dStack("<u%d>", 9, pan);

  returnCode(err);
}