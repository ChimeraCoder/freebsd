
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

/* p_move.c
 * Move a panel to a new location
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_move.c,v 1.9 2005/02/19 16:46:49 tom Exp $")

NCURSES_EXPORT(int)
move_panel(PANEL * pan, int starty, int startx)
{
  T((T_CALLED("move_panel(%p,%d,%d)"), pan, starty, startx));

  if (!pan)
    returnCode(ERR);

  if (IS_LINKED(pan))
    {
      Touchpan(pan);
      PANEL_UPDATE(pan, (PANEL *) 0);
    }

  returnCode(mvwin(pan->win, starty, startx));
}